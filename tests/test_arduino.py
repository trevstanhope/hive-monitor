#!/usr/bin/env python
import serial, ast, time
ARDUINO_DEV = raw_input('Enter Arduino device path: ')
ARDUINO_BAUD = int(raw_input('Enter Arduino baudrate: '))
try:
  arduino = serial.Serial(ARDUINO_DEV,ARDUINO_BAUD)
except Exception as error:
  print str(error)
while True:
  try:
    data = arduino.readline()
    print ast.literal_eval(data)
  except Exception as error:
    print str(error)

