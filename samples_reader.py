#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import struct

ADC_VREF = 3.3
ADC_RANGE = (1 << 12)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))
TRIGGER_VOLTAGE = 2.00 #user
TRIGGER_VALUE = int(TRIGGER_VOLTAGE / ADC_CONVERT)
TRIGGER_CHANNEL = 0 #user

data_to_send = struct.pack('BH', TRIGGER_CHANNEL, TRIGGER_VALUE)
data_to_send = data_to_send + b'\n'

tablica = np.zeros((5,2,500),dtype=np.uint8)

ser = serial.Serial("COM2", 115200, timeout=None)

for i in range(5):
    ser.write(data_to_send)
    data_raw = ser.read(1000)
    decoded_bytes = np.frombuffer(data_raw, dtype=np.uint8)

    tablica[i, 0, :500] = decoded_bytes[::2]
    tablica[i, 1, :500] = decoded_bytes[1::2]

ser.close()

np.save('samples.npy', tablica)
print("Done")
