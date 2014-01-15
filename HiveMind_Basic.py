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
ARDUINO_TIMEOUT = 1
UPDATE_INTERVAL = 10 # seconds until next sensor update
QUERY_INTERVAL = 1 # seconds until next graphic update
GRAPH_INTERVAL = 3600 # seconds in the past to display
COUCHDB_DATABASE = 'hivemind'
ERROR_HANDLER_FUNC = CFUNCTYPE(None, c_char_p, c_int, c_char_p, c_int, c_char_p)

def py_error_handler(filename, line, function, err, fmt):
  pass
C_ERROR_HANDLER = ERROR_HANDLER_FUNC(py_error_handler)

# HiveNode
class HiveMind:

  ## Initialization
  def __init__(self):
    
    ### Monitors
    try:
      print('\n[Initializing Monitors]')
      self.start_time = time.time()
      Monitor(cherrypy.engine, self.update, frequency=UPDATE_INTERVAL).subscribe()
      Monitor(cherrypy.engine, self.query, frequency=QUERY_INTERVAL).subscribe()
    except Exception as error:
      print('--> ' + str(error))
    
    ### Setup Arduino
    try:
      print('\n[Initializing Arduino]')
      self.arduino = serial.Serial(ARDUINO_DEV,ARDUINO_BAUD,timeout=ARDUINO_TIMEOUT)
    except Exception as error:
      print('--> ' + str(error))
      
    ### Setup CouchDB
    try:
      print('\n[Initializing CouchDB]')
      server = couchdb.Server()
      self.couch = server[COUCHDB_DATABASE]
      print('--> Using existing database: ' + COUCHDB_DATABASE)
    except Exception as error:
      print('--> Creating new database: ' + COUCHDB_DATABASE)
      self.couch = server.create(COUCHDB_DATABASE)
    except ValueError as error:
      print('--> ' + str(error))
      
    ### Setup Mic
    try:
      print('\n[Initializing Microphone]')
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
      print('\n[Creating New Log Entry]')
      print('--> Values default to 0')
      log = {
        'Internal_C':0,
        'External_C':0,
        'Internal_RH':0,
        'External_RH':0,
        'Frequency':0,
        'Amplitude':0
        }
      for key in log:
        print(key + '\t:\t' + str(log[key]))
    except Exception as error:
      print('--> ' + str(error))

    ### Time
    try:
      print('\n[Getting Current Time]')
      local_time = time.strftime("%Y-%m-%d-%H-%M-%S", time.localtime())
      unix_time = time.time()
      print('local_time\t:\t' + local_time)
      print('unix_time\t:\t' + str(unix_time))
      log['local_time'] = local_time
      log['unix_time'] = unix_time
    except Exception as error:
      pass
      

    ### Temp/Humidity
    try:
      print('\n[Reading Arduino Serial]')
      data = self.arduino.readline()
      json = ast.literal_eval(data)
      for key in json:
        print(str(key) + '\t:\t' + str(json[key]))
        if (json[key] > 100):
          print(' --> Temp/Humidity cannot exceed 100')
          log[key] = 100 # limit to 100
        else:
	      log[key] = json[key] # store all items in arduino JSON to log
    except Exception as error:
      print('--> ' + str(error))

    ### Audio
    try:
      print('\n[Capturing Audio]')
      self.stream.start_stream()
      data = self.stream.read(CHUNK)
      self.stream.stop_stream()
      try:
        wave_array = np.fromstring(data, dtype='int16')
        wave_fft = np.fft.fft(wave_array)
        wave_freqs = np.fft.fftfreq(len(wave_fft))
        freqs = wave_freqs[np.argsort(np.abs(wave_fft)**2)]
        amplitude = np.sqrt(np.mean(np.abs(wave_fft)**2))
        hertz = abs(freqs[1023]*RATE)
        decibels = 10*np.log10(amplitude)
        log['Frequency'] = hertz
        if (amplitude == 0):
          print('--> Amplitude cannot be infinite')
          decibels = 0
        else:
          decibels = 10*np.log10(amplitude)
        log['Frequency'] = hertz
        log['Amplitude'] = decibels
        print('Amplitude' + ' \t:\t' + str(decibels))
        print('Frequency' + ' \t:\t' + str(hertz))
      except ValueError as error:
        print('--> ' + str(error))
    except Exception as error:
      print('--> ' + str(error))

    ### CouchDB
    try:
      print('\n[Writing Log to CouchDB]')
      self.couch.save(log) # modifies log, so do it last
      print('--> Log saved successfully')
      print('_id' + '\t:\t' + str(log['_id']))
      print('_rev' + '\t:\t' + str(log['_rev']))
    except Exception as error:
      print('--> ' + str(error))  
  
  ## Query Updated Data
  def query(self):
    
    ### Query
    print('\n[Querying Recent Values]')
    map_nodes = "function(doc) { if (doc.unix_time >= " + str(time.time() - GRAPH_INTERVAL) + ") emit(doc); }"
    matches = self.couch.query(map_nodes)
    print('--> %d matches found' % len(matches))
    values = [] # will fill with data points
    for row in matches:
      try:
        local_time = row.key['local_time']
        unix_time = row.key['unix_time']
        int_T = row.key['Internal_C']
        ext_T = row.key['External_C']
        int_RH = row.key['Internal_RH']
        ext_RH = row.key['External_RH']
        freq = row.key['Frequency']
        amp = row.key['Amplitude']
        values.append([unix_time, local_time, int_T, ext_T, int_RH, ext_RH, freq, amp])
      except Exception as error:
        pass
        
    ### Open Files
    print('\n[Opening Files]')
    print('--> Temperature: ' + 'static/temperature.tsv')
    print('--> Humidity: ' + 'static/humidity.tsv')
    print('--> Amplitude: ' + 'static/amplitude.tsv')
    print('--> Frequency: ' + 'static/frequency.tsv')
    temperature = open('static/temperature.tsv', 'w')
    humidity = open('static/humidity.tsv', 'w')
    frequency = open('static/frequency.tsv', 'w')
    amplitude = open('static/amplitude.tsv', 'w')

    ### Headers
    print('\n[Writing Headers]')
    temperature_headers = ['date','Internal','External']
    humidity_headers = ['date','Internal','External']
    frequency_headers = ['date','Internal']
    amplitude_headers = ['date','Internal']
    print('--> Temperature: ' + str(temperature_headers))
    print('--> Humidity: ' + str(humidity_headers))
    print('--> Amplitude: ' + str(amplitude_headers))
    print('--> Frequency: ' + str(frequency_headers))
    temperature.write('\t'.join(temperature_headers) + '\n')
    humidity.write('\t'.join(humidity_headers) + '\n')
    frequency.write('\t'.join(frequency_headers) + '\n')
    amplitude.write('\t'.join(amplitude_headers) + '\n')

    ### Sort Values
    print('\n[Sorting Data]')
    print('--> Sorting %d data-points' % len(values))
    errors = 0
    for sample in sorted(values):
      try:
        temperature.write(str(sample[1]) + '\t' + str(sample[2]) + '\t' + str(sample[3]) + '\n')
        humidity.write(str(sample[1]) + '\t' + str(sample[4]) + '\t' + str(sample[5]) + '\n')
        frequency.write(str(sample[1]) + '\t' + str(sample[6]) + '\n')
        amplitude.write(str(sample[1]) + '\t' + str(sample[7]) + '\n')
      except Exception as error:
        errors += 1
        pass
    print('--> %d errors encountered' % errors)
        
  ## Render Index
  @cherrypy.expose
  def index(self):
      
    ### HTML
    try:
      print('\n[Loading Index]')
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
  conf = {
    '/static': {'tools.staticdir.on':True, 'tools.staticdir.dir':os.path.join(currdir,'static')},
    '/favicon': {'tools.staticfile.on':True, 'tools.staticfile.filename':os.path.join(currdir,'static/favicon.ico')}
   }
  print('\n[Starting CherryPy Server]')
  cherrypy.quickstart(root, '/', config=conf)
