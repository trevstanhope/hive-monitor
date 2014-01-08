#!/usr/bin/env python
import pyaudio
import numpy as np
import time
CHANNELS = 1
RATE = 44100
CHUNK = 1024
FORMAT = pyaudio.paInt16
microphone = pyaudio.PyAudio()
stream = microphone.open(format=FORMAT,channels=CHANNELS,rate=RATE,input=True,frames_per_buffer=CHUNK)
while True:
  time.sleep(1)
  try:
    stream.start_stream()
    data = stream.read(CHUNK)
    stream.stop_stream()
    try:
      wave_array = np.fromstring(data, dtype='int16')
      wave_fft = np.fft.fft(wave_array)
      wave_freqs = np.fft.fftfreq(len(wave_fft))
      freq = wave_freqs[np.argmax(np.abs(wave_fft)**2)]
      amplitude = np.sqrt(np.mean(np.abs(wave_fft)**2))
      print 'Frequency: ' + str(abs(freq*RATE))
      print 'Decibels: ' + str(10*np.log10(amplitude))
    except ValueError as error:
      print str(error)
  except Exception as error:
    print error
