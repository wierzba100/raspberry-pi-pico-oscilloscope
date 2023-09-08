#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import time

tablica = []

ser = serial.Serial("COM2", 115200, timeout=None)

ser.write(str.encode("ON\n"))

czas_startu = time.time()
print("Start")

while True:
    data_raw = ser.read(1)
    if data_raw == b'\x01':
        data_raw = ser.read(200000)
        print(data_raw)
        decoded_bytes = np.frombuffer(data_raw, dtype=np.uint8)
        break;


ser.close()
czas_konca = time.time()
print(f"CZAS: {czas_konca-czas_startu}")

np.save('samples.npy', decoded_bytes)
print("Done")
