#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))
LEVEL_ZERO = 1.65
TRIGGER_VOLTAGE = 1.65 #user

data = np.load('samples.npy')
data = data.astype(float)
data = data * ADC_CONVERT

trigger_options_index = 6
plt.rc('font', size=11)
plt.subplot(1, 1, 1)
if(trigger_options_index < 6):
    plt.plot(np.arange(len(data[trigger_options_index])), data[trigger_options_index], color='blue', marker='o')
else:
    plt.plot(np.arange(len(data[trigger_options_index][::2])), data[trigger_options_index][::2], color='blue', marker='o')
    plt.plot(np.arange(len(data[trigger_options_index][1::2])), data[trigger_options_index][1::2], color='red', marker='o')

plt.title(f"Acquisition nr: {trigger_options_index}")    
plt.xlabel("Samples")
plt.ylabel("Volts")
plt.grid()
plt.axhline(TRIGGER_VOLTAGE, color='grey', linestyle='--')
plt.axhline(LEVEL_ZERO, color='black')
#plt.xticks(np.arange(0, 100, step=2))
plt.show()
