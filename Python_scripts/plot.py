#!/usr/bin/env python3

import numpy as np
from matplotlib import pyplot as plt

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))
LEVEL_ZERO = 1.65
TRIGGER_VOLTAGE = 2.00 #user

data = np.load('samples.npy')
data = data.astype(float)
data = data * ADC_CONVERT

acq_nr = 0
plt.rc('font', size=14)
plt.subplot(1, 1, 1)

if(acq_nr < 5):
    plt.plot(np.arange(len(data[acq_nr])), data[acq_nr], color='blue', marker='o')
else:
    plt.plot(np.arange(len(data[acq_nr][::2])), data[acq_nr][::2], color='blue', marker='o', label='Channel 1')
    plt.plot(np.arange(len(data[acq_nr][1::2])), data[acq_nr][1::2], color='red', marker='o', label='Channel 2')
    plt.legend()

plt.title(f"Acquisition nr: {acq_nr}")
plt.xlabel("Samples")
plt.ylabel("Volts")
plt.grid()
plt.axhline(TRIGGER_VOLTAGE, color='grey', linestyle='--')
plt.axhline(LEVEL_ZERO, color='black')
#plt.xticks(np.arange(0, 100, step=2))
plt.show()
