#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

ADC_VREF = 3.3
ADC_RANGE = (1 << 12)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))
LEVEL_ZERO = 1.65
TRIGGER_VOLTAGE = 1.00 #user
TRIGGER_CHANNEL = 1 #user

data = np.load('samples.npy')

data = data.astype(float)

data = data * ADC_CONVERT

plt.rc('font', size=11)
for channel in range(2):
    for i in range(5):
        plt.subplot(5, 2, i * 2 + channel + 1)
        plt.axhline(LEVEL_ZERO, color='black')
        if channel == TRIGGER_CHANNEL:
            plt.axhline(TRIGGER_VOLTAGE, color='grey', linestyle='--')
        label = f'Channel {channel + 1}'
        color = 'blue' if channel == 0 else 'red'
        plt.plot(np.arange(len(data[i][channel])), data[i][channel], color=color, label=label, marker='o')
        plt.title(f"Channel {channel + 1}, Acquisition {i + 1}, 250kS/s")
        plt.xlabel("Samples")
        plt.ylabel("Volts")
        plt.legend(loc='upper right')
        plt.grid()

plt.show()
