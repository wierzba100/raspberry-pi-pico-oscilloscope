#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import sys
import time

tablica = []

ser = serial.Serial("COM2", 115200, timeout=0)

ser.write(str.encode("ON\n"))

czas_startu = time.time()
print("start")

while True:
    line = ser.readline()
    if((line != b'') and (line != b' ') and (line != b'\r\n')):
        #print(line)
        cleaned_message = line.replace(b'\r\n', b'')
        decoded_bytes = np.frombuffer(line, dtype=np.uint8)
        #print(decoded_bytes)
        break

czas_konca = time.time()
print(f"CZAS: {czas_konca-czas_startu}")

np.save('samples.npy', decoded_bytes)

np.set_printoptions(suppress=True)
np.set_printoptions(precision=2)
np.set_printoptions(threshold=sys.maxsize)

data = np.load('samples.npy')
print("Samples:")
#print(data)
print("Done")
