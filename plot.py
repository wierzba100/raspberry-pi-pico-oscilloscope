import numpy as np
from matplotlib import pyplot as plt

dict_data = np.load('channel_1.npz')
data_1 = dict_data['arr_0']
dict_data = np.load('channel_2.npz')
data_2 = dict_data['arr_0']

x_1 = data_1[:, 0]
y_1 = data_1[:, 1]

x_2 = data_2[:, 0]
y_2 = data_2[:, 1]

plt.subplot(2, 1, 1)
plt.plot(x_1, y_1, label='100kHz', color='blue')
plt.title("Channel 1, 250kS/s")
plt.xlabel("Microseconds") 
plt.ylabel("Volts")
plt.legend()

plt.subplot(2, 1, 2)
plt.plot(x_2, y_2, label='50kHz', color='red')
plt.title("Channel 2, 250kS/s")
plt.xlabel("Microseconds") 
plt.ylabel("Volts")
plt.legend()

plt.tight_layout()
plt.show()

