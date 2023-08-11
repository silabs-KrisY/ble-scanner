// Copyright (c) 2021 David G. Young
// Copyright (c) 2015 Damian Kołakowski. All rights reserved.

// cc scanner.c -lbluetooth -o scanner

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/time.h>
#include <getopt.h>
#include <stdbool.h>

#define SCAN_INTERVAL_DEFAULT 0x0010 //10ms (units of 0.625ms)
#define SCAN_WINDOW_DEFAULT 0x0010 //10ms (units of 0.625ms)

int device;

struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam)
{
	struct hci_request rq;
	memset(&rq, 0, sizeof(rq));
	rq.ogf = OGF_LE_CTL;
	rq.ocf = ocf;
	rq.cparam = cparam;
	rq.clen = clen;
	rq.rparam = status;
	rq.rlen = 1;
	return rq;
}

// cleanup and exit the program with exit code 0
void exit_clean()
{
	int ret, status;

	// Disable scanning.

	le_set_scan_enable_cp scan_cp;
	memset(&scan_cp, 0, sizeof(scan_cp));
	scan_cp.enable = 0x00;	// Disable flag.

	struct hci_request disable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);
	ret = hci_send_req(device, &disable_adv_rq, 1000);
	if ( ret < 0 )
		perror(" ERROR: Failed to disable scan.");

	hci_close_dev(device);
	exit( 0 );
}

// handles timeout
void signal_handler( int s )
{
	printf( "Exiting..\n" );
	fflush(stdout);
	exit_clean();
}

int main(int argc, char **argv)
{
	int ret, status;
	bdaddr_t target_bdaddr;
        bool target_bdaddr_flag=false;
	int interval=SCAN_INTERVAL_DEFAULT;
	int window=SCAN_WINDOW_DEFAULT;
	int active_scan=0; //default to passive scan
	int c;
	struct timeval stop, start;

	// parse args for interval, window, and addr
	while ((c = getopt(argc,argv,"i:w:m:a")) != -1)
	{
            switch (c)
	    {
	        case 'i':
		    interval = atoi(optarg);
                    printf("interval=%d ms\r\n", (int)(interval*0.625));
                    break;
                case 'w':
		    window = atoi(optarg);
                    printf("window=%d ms\r\n", (int)(window*0.625));
                    break;
                 case 'm':
		    status=str2ba(optarg,&target_bdaddr);
                    if (status < 0) {
                        perror(" ERROR: error in bdaddr conversion!");
                        return 0;
                    } else {
                        char addr[18];
                        ba2str(&target_bdaddr, addr);
                        printf("target addr: %s\r\n", addr);
                        target_bdaddr_flag=true;
                    }
                    break;
                  case 'a':
                    active_scan=0x01; //select active scan
                    printf("performing active scan\r\n");
                    break;
              }
          }


	// Get HCI device.

	device = hci_open_dev(1);
	if ( device < 0 ) {
		device = hci_open_dev(0);
		if (device >= 0) {
   		printf("Using hci0\n");
		}
	}
	else {
   		printf("Using hci1\n");
	}


	if ( device < 0 ) {
		perror(" ERROR: Failed to open HCI device.");
		return 0;
	}

        // Reset device

        struct hci_request reset_rq;
      	memset(&reset_rq,0,sizeof(reset_rq));
	reset_rq.ogf = OGF_HOST_CTL;
	reset_rq.ocf = OCF_RESET;
	reset_rq.rparam = &status;
	reset_rq.rlen=1;
        ret = hci_send_req(device,&reset_rq,1000);
        if (ret < 0 ) {
           perror(" ERROR: Failed to reset.");
           return 0;
        }

	// Set BLE scan parameters.

	le_set_scan_parameters_cp scan_params_cp;
	memset(&scan_params_cp, 0, sizeof(scan_params_cp));
	scan_params_cp.type 			= active_scan;
        scan_params_cp.interval                 = htobs(interval);
        scan_params_cp.window                   = htobs(window);

	scan_params_cp.own_bdaddr_type 	= 0x00; // Public Device Address (default).
	scan_params_cp.filter 			= 0x00; // Accept all.

	struct hci_request scan_params_rq = ble_hci_request(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &status, &scan_params_cp);

	ret = hci_send_req(device, &scan_params_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror(" ERROR: Failed to set scan parameters data.");
		return 0;
	}

	// Set BLE events report mask.

	le_set_event_mask_cp event_mask_cp;
	memset(&event_mask_cp, 0, sizeof(le_set_event_mask_cp));
	int i = 0;
	for ( i = 0 ; i < 8 ; i++ ) event_mask_cp.mask[i] = 0xFF;

	struct hci_request set_mask_rq = ble_hci_request(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &status, &event_mask_cp);
	ret = hci_send_req(device, &set_mask_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror(" ERROR: Failed to set event mask.");
		return 0;
	}

	// Enable scanning.

	le_set_scan_enable_cp scan_cp;
	memset(&scan_cp, 0, sizeof(scan_cp));
	scan_cp.enable 		= 0x01;	// Enable flag.
	scan_cp.filter_dup 	= 0x00; // Filtering disabled.

	struct hci_request enable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);

	ret = hci_send_req(device, &enable_adv_rq, 1000);
	if ( ret < 0 ) {
		hci_close_dev(device);
		perror(" ERROR: Failed to enable scan.");
		return 0;
	}
	gettimeofday(&start, NULL); //record timestamp

	// Get Results.

	struct hci_filter nf;
	hci_filter_clear(&nf);
	hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
	hci_filter_set_event(EVT_LE_META_EVENT, &nf);
	if ( setsockopt(device, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0 ) {
		hci_close_dev(device);
		perror(" ERROR: Could not set socket options\n");
		return 0;
	}


	uint8_t buf[HCI_MAX_EVENT_SIZE];
	evt_le_meta_event * meta_event;
	le_advertising_info * info;
	int len;
	int count = 0;
        bool exit_while=false;

	//const int timeout = 10;
	const int timeout = 0; // no timeout
	const int reset_timeout = 1; // wether to reset the timer on a received scan event (continuous scanning)
	//const int max_count = 1000;
	const int max_count = -1; // no max count

	sigset_t sigalrm_set; // apparently the signal must be unblocked in some cases
	sigemptyset ( &sigalrm_set );
        sigaddset ( &sigalrm_set, SIGALRM );

	// if timeout is <= 0, don't worry about the SIGALRM/timeout
	if ( timeout > 0 )
	{	// Install a signal handler so that we can set the exit code and clean up
        	if ( signal( SIGALRM, signal_handler ) == SIG_ERR )
        	{
                	hci_close_dev(device);
                	perror( "Could not install signal handler\n" );
                	return 0;
        	}
		alarm( timeout ); // set the alarm timer, when time is up the program will be terminated

		if ( sigprocmask( SIG_UNBLOCK, &sigalrm_set, NULL ) != 0 )
		{
			hci_close_dev(device);
			perror( "Could not unblock alarm signal" );
			return 0;
		}
	}

	// Keep scanning until the timeout is triggered or we have seen lots of advertisements.  Then exit.
	// We exit in this case because the scan may have failed or stopped. Higher level code can restart
	while (( count < max_count || max_count <= 0 ) && (exit_while == false))
	{
		len = read(device, buf, sizeof(buf));
		if ( len >= HCI_EVENT_HDR_SIZE )
		{
			meta_event = (evt_le_meta_event*)(buf+HCI_EVENT_HDR_SIZE+1);
			if ( meta_event->subevent == EVT_LE_ADVERTISING_REPORT )
			{
				count++;
				if ( reset_timeout != 0 && timeout > 0 ) // reset/restart the alarm timer
					alarm( timeout );

				// print results
				uint8_t reports_count = meta_event->data[0];
				void * offset = meta_event->data + 1;
				while ( reports_count-- )
				{
                                        info = (le_advertising_info *)offset;
                                        if (target_bdaddr_flag==false) {
					    char addr[18];
					    ba2str( &(info->bdaddr), addr);
					    printf("%s %d", addr, (int8_t)info->data[info->length]);
					    for (int i = 0; i < info->length; i++)
						printf(" %02X", (unsigned char)info->data[i]);
					    printf("\n");
					    offset = info->data + info->length + 2;
				        } else {
                                            // look for target bdaddr and exit with timestamp when found
                                            if (bacmp(&(info->bdaddr),&target_bdaddr) == 0) 
                                            {
                                               gettimeofday(&stop, NULL);
                                               printf("target bdaddr found after %lu ms\r\n", (stop.tv_sec - start.tv_sec) * 1000 + (int)((stop.tv_usec - start.tv_usec) / 1000));
                                               exit_while = true;
                                               break;
                                             } 
                                         }
                                }
			}
		}
	}

	// Prevent SIGALARM from firing during the clean up procedure
	if ( sigprocmask( SIG_BLOCK, &sigalrm_set, NULL ) != 0 )
	{
		hci_close_dev(device);
		perror( "Could not block alarm signal" );
		return 0;
	}

	exit_clean();
	return 0;
}
