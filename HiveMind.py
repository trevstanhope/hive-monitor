#!/usr/bin/env python

"""
HiveMind
Developed by Trevor Stanhope
Wireless sensor node for single-hive urban beekeepers.
"""

# Modules
import sys, os.path
import couchdb, cherrypy, serial, ast, time
import pyaudio
from serial import SerialException
from cherrypy.process.plugins import Monitor
from cherrypy import tools
import numpy as np
from ctypes import *

# Global Values
CHANNELS = 1
RATE = 44100
CHUNK = 1024
FORMAT = pyaudio.paInt16
ARDUINO_DEV = '/dev/ttyS0'
ARDUINO_BAUD = 9600
UPDATE_INTERVAL = 1 # seconds until next sensor update
QUERY_INTERVAL = 5 # seconds until next graphic update
GRAPH_INTERVAL = 86400 # seconds in the past to display
COUCHDB_DATABASE = 'hivemind'
ERROR_HANDLER_FUNC = CFUNCTYPE(None, c_char_p, c_int, c_char_p, c_int, c_char_p)

def py_error_handler(filename, line, function, err, fmt):
  pass
C_ERROR_HANDLER = ERROR_HANDLER_FUNC(py_error_handler)

# HiveNode
class HiveMind:

  ## Initialization
  def __init__(self):
  
    ### Configuration
    print(CHANNELS)
    print(RATE)
    print(CHUNK)
    print(ARDUINO_DEV)
    print(ARDUINO_BAUD)
    print(UPDATE_INTERVAL)
    print(QUERY_INTERVAL)
    print(GRAPH_INTERVAL)
    
    ### Monitors
    try:
      print('[Initializing Monitors]')
      self.start_time = time.time()
      Monitor(cherrypy.engine, self.update, frequency=UPDATE_INTERVAL).subscribe()
      Monitor(cherrypy.engine, self.query, frequency=QUERY_INTERVAL).subscribe()
    except Exception as error:
      print('--> ' + str(error))
    
    ### Setup Arduino
    try:
      print('[Initializing Arduino]')
      self.arduino = serial.Serial(ARDUINO_DEV,ARDUINO_BAUD)
    except Exception as error:
      print('--> ' + str(error))
      
    ### Setup CouchDB
    try:
      print('[Initializing CouchDB]')
      server = couchdb.Server()
      self.couch = server[COUCHDB_DATABASE]
    except Exception as error:
      self.couch = server.create(COUCHDB_DATABASE)
    except ValueError as error:
      print('--> ' + str(error))
      
    ### Setup Mic
    try:
      print('[Initializing Microphone]')
      asound = cdll.LoadLibrary('libasound.so.2')
      asound.snd_lib_error_set_handler(C_ERROR_HANDLER) # Set error handler
      mic = pyaudio.PyAudio()
      self.stream = mic.open(format=FORMAT,channels=CHANNELS,rate=RATE,input=True,frames_per_buffer=CHUNK)
      self.stream.stop_stream()
    except Exception as error:
      print('--> ' + str(error))
      
  ## Get Arduino data as JSON message
  def update(self):
    
    ### New Log
    try:
      print('[Creating New Log Entry]')
      log = {}
      log['Date'] = time.strftime("%Y-%m-%d-%H-%M-%S", time.localtime())
      log['Time'] = time.time()
    except Exception as error:
      print('--> ' + str(error))      

    ### Temp/Humidity
    try:
      print('[Reading Arduino Serial]')
      data = self.arduino.readline()
      json = ast.literal_eval(data)
      for key in json:
        if (json[key] > 100):
	        log[key] = 100 # limit to 100
        else:
	        log[key] = json[key] # store all items in arduino JSON to log
    except Exception as error:
      print('--> ' + str(error))

    ### Audio
    try:
      print('[Capturing Audio]')
      self.stream.start_stream()
      data = self.stream.read(CHUNK)
      self.stream.stop_stream()
      try:
        wave_array = np.fromstring(data, dtype='int16')
        wave_fft = np.fft.fft(wave_array)
        wave_freqs = np.fft.fftfreq(len(wave_fft))
        freqs = wave_freqs[np.argsort(np.abs(wave_fft)**2)]
        amplitude = np.sqrt(np.mean(np.abs(wave_fft)**2))
        log['Frequency'] = abs(freqs[1023]*RATE)
        log['Amplitude'] = 10*np.log10(amplitude)
      except ValueError as error:
        print('--> ' + str(error))
    except Exception as error:
      print('--> ' + str(error))

    ### CouchDB
    try:
      print('[Writing Log to CouchDB]')
      for key in log:
        print('--> ' + key + '\t:\t' + str(log[key]))
      self.couch.save(log)
    except Exception as error:
      print('--> ' + str(error))  
  
  ## Query Updated Data
  def query(self):
    
    ### Query
    print('[Querying Recent Values]')
    map_nodes = "function(doc) { if (doc.unix_time >= " + str(time.time() - GRAPH_INTERVAL) + ") emit(doc); }"
    matches = self.couch.query(map_nodes)
    values = []
    for row in matches:
      try:
        unix_time = row.key['Time']
        date = row.key['Date']
        int_T = row.key['Internal_C']
        ext_T = row.key['External_C']
        int_RH = row.key['Internal_RH']
        ext_RH = row.key['External_RH']
        freq = row.key['Frequency']
        amp = row.key['Amplitude']
        values.append([unix_time, date, int_T, ext_T, int_RH, ext_RH, freq, amp])
      except Exception as error:
        pass
    
    ### Sort to File
    print('[Writing Sorted Values to File]')
    temperature = open('static/temperature.tsv', 'w')
    humidity = open('static/humidity.tsv', 'w')
    sound = open('static/sound.tsv', 'w')
    temperature.write('date\tInternal\tExternal\n')
    humidity.write('date\tInternal\tExternal\n')
    sound.write('date\tFrequency (Hz)\tAmplitude (dB)\n')
    for sample in sorted(values):
      print sample
      try:
        temperature.write(str(sample[1]) + '\t' + str(sample[2]) + '\t' + str(sample[3]) + '\n')
        humidity.write(str(sample[1]) + '\t' + str(sample[4]) + '\t' + str(sample[5]) + '\n')
        sound.write(str(sample[1]) + '\t' + str(sample[6]) + '\t' + str(sample[7]) + '\n')
      except Exception as error:
        pass
        
  ## Render Index
  @cherrypy.expose
  def index(self):
      
    ### HTML
    try:
      print('[Loading Index]')
      page = open('static/index.html').read()
    except Exception as error:
      print('--> ' + str(error))
      page = ''
    return page
    
# Main
if __name__ == '__main__':
  root = HiveMind()
  currdir = os.path.dirname(os.path.abspath(__file__))
  cherrypy.server.socket_host = '0.0.0.0'
  conf = {'/static': {'tools.staticdir.on':True, 'tools.staticdir.dir':os.path.join(currdir,'static')}}
  cherrypy.quickstart(root, '/', config=conf)
