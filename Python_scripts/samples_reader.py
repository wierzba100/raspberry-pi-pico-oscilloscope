#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import struct

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))

def prepare_string(acq_mode, channels_nr, trigger_channel, trigger_edge, trigger_voltage):
    acq_mode_value = 1 if acq_mode == "normal" else 0
    sampling_mode = 3 if channels_nr == 2 else trigger_channel
    trigger_value = int(trigger_voltage / ADC_CONVERT)
    trigger_edge_value = 1 if trigger_edge == "rising" else 0
    string = struct.pack('BBBBB', acq_mode_value, trigger_channel-1, sampling_mode, 
                         trigger_value, trigger_edge_value)
    string = string + b'\n'
    return string


def acquire_data(acq):
    
    tablica = np.empty((0, 10000), dtype=np.uint8)

    ser = serial.Serial("COM2", 115200, timeout=1)

    for trigger_option in acq:
        data_to_send = prepare_string(
            trigger_option["mode"],
            trigger_option["channels_nr"],
            trigger_option["trigger_channel"],
            trigger_option["trigger_edge"],
            trigger_option["trigger_level"]
        )
        ser.write(data_to_send)
        data_raw = ser.read(10000)
        if data_raw:
            decoded_bytes = np.frombuffer(data_raw, dtype=np.uint8)
            tablica = np.vstack([tablica, decoded_bytes])

    ser.close()

    np.save('samples.npy', tablica)

acq_params = [
    {
        "mode": "normal",
        "channels_nr": 1,
        "trigger_channel": 1,
        "trigger_edge": "rising",
        "trigger_level": 1.65
    },
    {
        "mode": "normal",
        "channels_nr": 1,
        "trigger_channel": 2,
        "trigger_edge": "falling",
        "trigger_level": 1.65
    },
    {
        "mode": "normal",
        "channels_nr": 2,
        "trigger_channel": 1,
        "trigger_edge": "rising",
        "trigger_level": 2.25
    },
    {
        "mode": "auto",
        "channels_nr": 1,
        "trigger_channel": 1,
        "trigger_edge": "rising",
        "trigger_level": 1.65
    },
    {
        "mode": "auto",
        "channels_nr": 1,
        "trigger_channel": 2,
        "trigger_edge": "rising",
        "trigger_level": 1.65
    },
    {
        "mode": "auto",
        "channels_nr": 2,
        "trigger_channel": 2,
        "trigger_edge": "rising",
        "trigger_level": 1.65
    },
    {
        "mode": "normal",
        "channels_nr": 2,
        "trigger_channel": 1,
        "trigger_edge": "falling",
        "trigger_level": 1.00
    },
    {
        "mode": "normal",
        "channels_nr": 2,
        "trigger_channel": 2,
        "trigger_edge": "rising",
        "trigger_level": 2.00
    },
]


acquire_data(acq_params)

print("Done")