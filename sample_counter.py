#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

# Usage:
# eg. python3 plotter /dev/ttyACM0 10

import serial
import sys
import time

counter = 0

if len(sys.argv) < 2:
    raise Exception("No port specified!")

ser = serial.Serial(sys.argv[1], 115200, timeout=0)

czas_trwania = float(sys.argv[2])
czas_startu = time.time()


while True:
    line = ser.readline()
    try:
        line = float(line)
    except:
        print("Something went wrong")
    else:
        counter = counter + 1
    
    czas_teraz = time.time()
    if czas_teraz - czas_startu >= czas_trwania:
        break

print(counter)
