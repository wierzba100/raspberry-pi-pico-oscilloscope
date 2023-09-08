#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import sys
import time

tablica = []

ser = serial.Serial("COM2", 115200, timeout=None)

ser.write(str.encode("ON\n"))

czas_startu = time.time()
print("start")

while True:
    data_raw = ser.read(1)
    if data_raw == b'\x01':
        data_raw = ser.read(200002)
        #print(data_raw)
        cleaned_message = data_raw.replace(b'\r\n', b'')
        decoded_bytes = np.frombuffer(cleaned_message, dtype=np.uint8)
        break;


ser.close()
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
