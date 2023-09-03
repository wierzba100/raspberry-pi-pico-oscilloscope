#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))

data = np.load('samples.npy')

channel_1 = []
channel_2 = []
time_1 = []
time_2 = []

for i, item in enumerate(data):
    if i % 2 == 0:
        channel_1.append(int.from_bytes(item, byteorder='big') * ADC_CONVERT)
    else:
        channel_2.append(int.from_bytes(item, byteorder='big') * ADC_CONVERT)

for i in range(100000):
    time_1.append(i * 4)
    time_2.append(2 + i * 4)

plt.subplot(2, 1, 1)
plt.plot(time_1, channel_1, label='100kHz', color='blue')
plt.title("Channel 1, 250kS/s")
plt.xlabel("Microseconds") 
plt.ylabel("Volts")
plt.legend()

plt.subplot(2, 1, 2)
plt.plot(time_2, channel_2, label='50kHz', color='red')
plt.title("Channel 2, 250kS/s")
plt.xlabel("Microseconds") 
plt.ylabel("Volts")
plt.legend()

plt.tight_layout()
plt.show()

