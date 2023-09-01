#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

# Usage:
# eg. python3 plotter /dev/ttyACM0 10

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
            tablica.append(line)
print("CZAS")
print(czas_konca-czas_startu)
print("\n")
print("\n")
print("\n")

counter_1 = 0
counter_2 = 2

tablica_1 = np.zeros((100000, 2), dtype=np.byte)
tablica_2 = np.zeros((100000, 2), dtype=np.byte)

j=0

for i in range(0, len(tablica), 2):
    tablica_1[j, 0] = counter_1
    tablica_1[j, 1] = tablica[i]
    j = j + 1;
    #counter_1 = counter_1 + 4

j=0

for i in range(1, len(tablica), 2):
    tablica_2[j, 0] = counter_2
    tablica_2[j, 1] = tablica[i]
    j = j + 1;
    #counter_2 = counter_2 + 4

np.set_printoptions(suppress=True)
np.set_printoptions(precision=2)
np.set_printoptions(threshold=sys.maxsize)

np.save('channel_1.npy', tablica_1)
np.save('channel_2.npy', tablica_2)

data = np.load('channel_1.npy')
print("Channel 1, 100KHz PWM, 50%, 100000 samples")
#print(data)
data = np.load('channel_2.npy')
print("\n")
print("\n")
print("\n")
print("\n")
print("Channel 2, 50KHz PWM, 50%, 100000 samples")
#print(data)

print("Done")
