#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import time

tablica = []

ser = serial.Serial("COM2", 115200, timeout=None)

ser.write(str.encode("ON\n"))

print("Start")
czas_startu = time.time()

data_raw = ser.read(200000)
decoded_bytes = np.frombuffer(data_raw, dtype=np.uint8)

czas_konca = time.time()

line = ser.readline()
line = int(line)

ser.close()
print(f"Czas nadawania:  {line * 0.000001} s")
print(f"Czas odbierania: {czas_konca-czas_startu} s")

np.save('samples.npy', decoded_bytes)
print("Done")
