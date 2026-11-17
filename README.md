# BLE Scanner

A command line utility for Bluetooth LE scanning on Linux / BlueZ

## Compiling

1. Install prerequisites

`sudo apt-get install libbluetooth-dev`

2. Compile the program

`make`

## Running

Run the program like this:

`./scanner`

Each time it detects a Bluetooth LE advertisement it prints out the MAC address of the advertisting device, the RSSI and the bytes of the advertisement like this:

```
BDADDR: 50:F1:0B:83:9B:03  RSSI: -70  ADV PLOAD:  02 01 1A 02 0A 08 0A FF 4C 00 10 05 3A 1C E6 BA 41
BDADDR: 52:6F:B6:A3:E2:3E  RSSI: -89  ADV PLOAD:  02 01 1A 02 0A 0C 0B FF 4C 00 10 06 0B 19 65 1C 7A 18
BDADDR: 20:6D:31:61:81:D1  RSSI: -91  ADV PLOAD:  02 01 06 0C 09 46 69 72 65 77 61 6C 6C 61 41 50

```

The program will auto exit after one of two things happens:  (a) 10 seconds pass with no detections.  (b) 1000 advertisements are detected. 

### Parameters

-i <interval>                       The scan interval as an integer in units of 0.625 ms
-w <window>                         The scan window as an integer in units of 0.625 ms
-m <target 48-bit MAC address>      When included, the program will return the time elapsed before detecting an advertisement from the target MAC address
-a                                  When included, will perform active scanning and return scan responses


### Examples

1. Determining the time it takes to receive an advertisement from a target MAC address with a 20% duty cycle (50ms interval, 10ms window).
```
./exe/scanner -m 50:F1:0B:83:9B:03 -i 80 -w 16
target addr: 50:F1:0B:83:9B:03
interval=50 ms
window=10 ms
Using hci0
target bdaddr found after 214 ms
```

2. Performing a 50% duty cycle active scan with 100ms interval and 50ms window.
```
$ sudo ./exe/scanner -i 160 -w 80
interval=100 ms
window=50 ms
Using hci0
BDADDR: 7B:96:2A:92:C1:54  RSSI: -83  ADV PL:  02 01 1A 1B FF 4C 00 0C 0E 00 84 70 7F C6 C9 82 8C B1 A2 E9 09 08 04 10 06 0B 19 1C E2 28 78
BDADDR: 20:6D:31:61:81:D1  RSSI: -87  ADV PL:  02 01 06 0C 09 46 69 72 65 77 61 6C 6C 61 41 50
BDADDR: 58:A6:AA:A2:C4:9E  RSSI: -82  ADV PL:  02 01 1A 0D FF 4C 00 16 08 00 14 81 96 21 D3 38 2E
```

## Need Help?  Want to change this?

Feel free to open an Issue or submit a Pull Request on Github here: https://github.com/davidgyoung/ble-scanner

## License

[BSD 3](./LICENSE)

## Credits

This program is based off of code by Damian Kołakowski here https://github.com/damian-kolakowski/intel-edison-playground/blob/master/scan.c

