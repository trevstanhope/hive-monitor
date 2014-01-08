#!/usr/bin/env python
import alsaaudio
import numpy as np
import time
CHANNEL = 1
RATE = 44100
CHUNK = 1024
microphone = alsaaudio.PCM(alsaaudio.PCM_CAPTURE)
microphone.setchannels(CHANNEL)
microphone.setrate(RATE)
microphone.setformat(alsaaudio.PCM_FORMAT_S16_LE)
microphone.setperiodsize(CHUNK)
while True:
  time.sleep(1)
  try:
    (success, data) = microphone.read()
    if success:
      try:
        wave_array = np.fromstring(data, dtype='int16')
        wave_fft = np.fft.fft(wave_array)
        wave_freqs = np.fft.fftfreq(len(wave_fft))
        freq = wave_freqs[np.argmax(np.abs(wave_fft)**2)]
        amplitude = np.sqrt(np.mean(np.abs(wave_fft)**2))
        print 'Frequency: ' + str(abs(freq*RATE))
        print 'Decibels' + str(10*np.log10(amplitude))
      except ValueError as error:
          print str(error)
    else:
      print 'Capture Error'
  except Exception as error:
    print error
