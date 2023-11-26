#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import numpy as np
import struct

ADC_VREF = 3.3
ADC_RANGE = (1 << 8)
ADC_CONVERT = (ADC_VREF / (ADC_RANGE - 1))

def prepare_string(acq_mode, channels_nr, trigger_channel, trigger_edge, trigger_voltage, acq_period):
    acq_mode_value = 1 if acq_mode == "normal" else 0
    sampling_mode = 3 if channels_nr == 2 else trigger_channel
    trigger_value = int(trigger_voltage / ADC_CONVERT)
    trigger_edge_value = 1 if trigger_edge == "rising" else 0
    string = struct.pack('BBBBBB', acq_mode_value, trigger_channel-1, sampling_mode, 
                         trigger_value, trigger_edge_value, acq_period)
    string = string + b'\n'
    return string


def acquire_data(acq_params):
    tablica = np.empty((0, 100), dtype=np.uint8)

    ser = serial.Serial("COM2", 115200, timeout=1)

    for trigger_option in acq_params:
        data_to_send = prepare_string(
            trigger_option["mode"],
            trigger_option["channels_nr"],
            trigger_option["trigger_channel"],
            trigger_option["trigger_edge"],
            trigger_option["trigger_level"],
            trigger_option["acq_period"]
        )
        ser.write(data_to_send)
        data_raw = ser.read(100)
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
        "trigger_level": 1.65,
        "acq_period": 10
    },
    {
        "mode": "normal",
        "channels_nr": 1,
        "trigger_channel": 2,
        "trigger_edge": "falling",
        "trigger_level": 1.65,
        "acq_period": 10
    },
    {
        "mode": "normal",
        "channels_nr": 2,
        "trigger_channel": 1,
        "trigger_edge": "rising",
        "trigger_level": 2.25,
        "acq_period": 10
    },
    {
        "mode": "auto",
        "channels_nr": 1,
        "trigger_channel": 1,
        "trigger_edge": "rising",
        "trigger_level": 1.65,
        "acq_period": 10
    },
    {
        "mode": "auto",
        "channels_nr": 1,
        "trigger_channel": 2,
        "trigger_edge": "rising",
        "trigger_level": 1.65,
        "acq_period": 10
    },
    {
        "mode": "auto",
        "channels_nr": 2,
        "trigger_channel": 2,
        "trigger_edge": "rising",
        "trigger_level": 1.65,
        "acq_period": 10
    },
    {
        "mode": "normal",
        "channels_nr": 2,
        "trigger_channel": 1,
        "trigger_edge": "falling",
        "trigger_level": 1.00,
        "acq_period": 10
    },
    {
        "mode": "normal",
        "channels_nr": 2,
        "trigger_channel": 2,
        "trigger_edge": "rising",
        "trigger_level": 2.00,
        "acq_period": 10
    },
]


acquire_data(acq_params)

print("Done")