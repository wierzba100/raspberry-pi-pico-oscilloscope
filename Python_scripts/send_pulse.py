#!/usr/bin/env python3

# Grabs raw data from the Pico's UART

import serial
import struct

ser_pulse = serial.Serial("COM5", 115200, timeout=None)

string = struct.pack('B', 1)
data_to_send = string + b'\n'
ser_pulse.write(data_to_send)

ser_pulse.close()