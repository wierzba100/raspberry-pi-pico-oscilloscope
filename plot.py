#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))

data = np.load('samples.npy')

data = data.astype(float)

data = data * ADC_CONVERT

for i in range(5):
    for channel in range(2):
        plt.subplot(2, 1, channel + 1)
        label = ('10kHz' if channel == 0 else '5kHz')
        plt.plot(np.arange(len(data[i][channel])), data[i][channel], color='blue' if channel == 0 else 'red' )#,marker='o')
        plt.title(f"Channel {channel + 1}, Akwizycja nr: {i} 250kS/s")
        plt.xlabel("Samples")
        plt.ylabel("Volts")

plt.show()
