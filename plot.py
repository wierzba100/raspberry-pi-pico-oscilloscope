#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))
LEVEL_ZERO = 1.65
TRIGGER_VOLTAGE = 0.5 #user
TRIGGER_CHANNEL = 1 #user

data = np.load('samples.npy')

data = data.astype(float)

data = data * ADC_CONVERT

plt.rc('font', size=14)
for channel in range(2):
    plt.subplot(2, 1, channel + 1)
    plt.axhline(LEVEL_ZERO, color='black')
    if channel == TRIGGER_CHANNEL:
        plt.axhline(TRIGGER_VOLTAGE, color='grey',linestyle='--')
    label=f'Channel {channel + 1}'
    color = 'blue' if channel == 0 else 'red'
    for i in range(1):
        plt.plot(np.arange(len(data[i][channel])), data[i][channel], color=color, label=label if i == 0 else '_nolegend_')
        plt.title(f"Channel {channel + 1}, 5 acquisitions, 250kS/s")
        plt.xlabel("Samples")
        plt.ylabel("Volts")
        plt.legend(loc='upper right')
        plt.grid()

plt.show()
