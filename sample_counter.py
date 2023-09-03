#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import sys
import time

tablica = []

ser = serial.Serial("COM2", 115200, timeout=0)

while True:
    ser.write(str.encode("ON\n"))
    line = ser.readline()
    if(line == b'Connected\r\n'):
        break

czas_startu = time.time()
print("start")

while True:
    line = ser.readline()
    if((line != b'') and (line != b' ') and (line != b'\r\n')):
        if(line == b'End\r\n'):
            czas_konca = time.time()
            break
        else:
            cleaned_message = line.replace(b'\r\n', b'')
            #print(cleaned_message)
            tablica.append(cleaned_message)

print(f"CZAS: {czas_konca-czas_startu}")

np.save('samples.npy', tablica)

np.set_printoptions(suppress=True)
np.set_printoptions(precision=2)
np.set_printoptions(threshold=sys.maxsize)

data = np.load('samples.npy')
print("Samples:")
print(data)
print("Done")
