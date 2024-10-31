from scipy import signal
import matplotlib.pyplot as plt
import numpy as np

b4, a4 = signal.butter(4, 100, 'low', analog=True)
b6, a6 = signal.butter(6, 100, 'low', analog=True)
b8, a8 = signal.butter(8, 100, 'low', analog=True)
b10, a10 = signal.butter(10, 100, 'low', analog=True)
w4, h4 = signal.freqs(b4, a4)
w6, h6 = signal.freqs(b6, a6)
w8, h8 = signal.freqs(b8, a8)
w10, h10 = signal.freqs(b10, a10)
plt.semilogx(w4, 20 * np.log10(abs(h4)), label='4')
plt.semilogx(w6, 20 * np.log10(abs(h6)), label='6')
plt.semilogx(w8, 20 * np.log10(abs(h8)), label='8')
plt.semilogx(w10, 20 * np.log10(abs(h10)), label='10')
plt.title('Butterworth filter frequency response')
plt.xlabel('Frequency [radians / second]')
plt.ylabel('Amplitude [dB]')
plt.margins(0, 0.1)
plt.grid(which='both', axis='both')
plt.axvline(100, color='green') # cutoff frequency
plt.legend()
plt.show()
