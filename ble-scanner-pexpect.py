#!/user/bin/env python3

'''This script runs the ble-scanner executable in a loop, with a random
delay between executions. It targets a specific MAC address and collects
duration measurements (how long it took to find the specific beacon) and
writes them to output.txt for post analysis '''

# Copyright 2023 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

import pexpect
import time
import random
ITERATIONS=1000

debug = 0 # set to 1 for debug prints
iteration=ITERATIONS
print("Running for: " + str(iteration) + " iterations")
with open('output.txt', 'w') as f:

	while (iteration>=0):
		if debug==1:
			print("Starting iteration #" + str(ITERATIONS-iteration+1))
		wait = random.random()
		if debug == 1:
			print("Waiting random " + str(wait) + " seconds")
		time.sleep(wait)
		child = pexpect.spawn("sudo ./exe/scanner -i 178 -w 8 -m 1C:34:F1:DE:25:74")

		i = child.expect([pexpect.TIMEOUT,"Using"], timeout=5)
		if i == 0:
    			print("ble-scanner failed to initialize.")
    			child.kill(0)
    			exit()

		if debug == 1:
			print("ble-scanner init successful")

		i = child.expect([pexpect.TIMEOUT,pexpect.EOF], timeout=100)

		if i == 0:
    			# ble scanner timeout
    			print("ble scanner timeout!")
    			child.kill(0)
    			exit()

		str_split = child.before.split(b" ")
		if debug==1:
			print("Split string:")
			print(str_split)
		if b"ERROR:" in str_split:
			print("ble-scanner error: " + child.before.decode('utf-8'))
			exit()
		if len(str_split) != 7:
			print("Error in split")
			print(str_split)
			continue
		int_str = str_split[len(str_split)-2].decode('utf-8')
		found_time = int(int_str)
		print("Iteration #" + str(ITERATIONS-iteration+1) + ": Found time: " + str(found_time))
		print(found_time,file=f) # write to file
		iteration=iteration-1

print("Exiting!")
