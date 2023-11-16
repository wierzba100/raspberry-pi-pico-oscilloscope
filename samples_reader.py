#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import struct
from enum import Enum

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))

class Channel(Enum):
    Channel_1 = 0
    Channel_2 = 1

class SamplingMode(Enum):
    Off = 0
    Channel_1 = 1
    Channel_2 = 2
    Channel_1_and_2 = 3

def prepare_string(trigger_voltage, trigger_channel, sampling_mode):
    trigger_value = int(trigger_voltage / ADC_CONVERT)
    string = struct.pack('BBB', trigger_channel, sampling_mode, trigger_value)
    string = string + b'\n'
    return string
    
tablica = np.empty((0, 100), dtype=np.uint8)

ser = serial.Serial("COM2", 115200, timeout=1)
#ser_pulse = serial.Serial("COM5", 115200, timeout=None)

trigger_options = [
    [1.65, Channel.Channel_1.value, SamplingMode.Channel_1.value],
    [1.65, Channel.Channel_2.value, SamplingMode.Channel_2.value],
    [0.50, Channel.Channel_1.value, SamplingMode.Channel_1.value],
    [0.50, Channel.Channel_2.value, SamplingMode.Channel_2.value],
    [2.50, Channel.Channel_1.value, SamplingMode.Channel_1.value],
    [2.50, Channel.Channel_2.value, SamplingMode.Channel_2.value],
    [1.65, Channel.Channel_1.value, SamplingMode.Channel_1_and_2.value],
    [1.65, Channel.Channel_2.value, SamplingMode.Channel_1_and_2.value],
    [0.00, Channel.Channel_1.value, SamplingMode.Channel_1.value],
    [0.00, Channel.Channel_2.value, SamplingMode.Channel_2.value],
]

for trigger_option in trigger_options:
    data_to_send = prepare_string(trigger_option[0], trigger_option[1], trigger_option[2])
    ser.write(data_to_send)
    #ser_pulse.write(data_to_send)
    data_raw = ser.read(100)
    if data_raw:
        decoded_bytes = np.frombuffer(data_raw, dtype=np.uint8)
        tablica = np.vstack([tablica, decoded_bytes])
    
ser.close()
#ser_pulse.close()

np.save('samples.npy', tablica)
print("Done")