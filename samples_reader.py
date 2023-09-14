#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np

tablica = np.zeros((5,2,400),dtype=np.uint8)

ser = serial.Serial("COM2", 115200, timeout=None)

for i in range(5):
    ser.write(str.encode("ON\n"))
    data_raw = ser.read(800)
    decoded_bytes = np.frombuffer(data_raw, dtype=np.uint8)
    
    tablica[i, 0, :400] = decoded_bytes[::2]
    tablica[i, 1, :400] = decoded_bytes[1::2]

ser.close()

np.save('samples.npy', tablica)
print("Done")
