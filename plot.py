#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))

data = np.load('samples.npy')
data = data.astype(float)

time_1 = []
time_2 = []

for i in range(len(data)):
    for j in range(len(data[i])):
        for k in range(len(data[i][j])):
            data[i][j][k] = data[i][j][k] * ADC_CONVERT


for i in range(400):
    time_1.append(i * 4)
    time_2.append(2 + i * 4)

for i in range(0, 5, 1):
    plt.subplot(5, 2, (2*i)+1)
    plt.plot(time_1, data[0][i], label='10kHz', color='blue', marker='o')
    plt.title(f"Channel 1,Akwizycja nr: {i} 250kS/s")
    plt.xlabel("Microseconds")
    plt.ylabel("Volts")
    plt.legend()
    
    plt.subplot(5, 2, (2*i)+2)
    plt.plot(time_2, data[1][i], label='5kHz', color='red', marker='o')
    plt.title(f"Channel 2,Akwizycja nr: {i} 250kS/s")
    plt.xlabel("Microseconds")
    plt.ylabel("Volts")
    plt.legend()

plt.show()

