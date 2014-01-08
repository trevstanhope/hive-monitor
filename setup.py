#!/usr/bin/env python
# Configure HiveMind on RaspberryPi

# Modules
import string
import random
import shutil
import subprocess
import os

# Functions
def key_generator(size=6, chars=string.ascii_uppercase + string.digits):
  return ''.join(random.choice(chars) for x in range(size))
  
def bash(script='ls'):
  print('BASH: ' + script)
  action = subprocess.Popen(script, shell=True, stdin=None, stdout=open(os.devnull,"wb"), stderr=None, executable="/bin/sh")
  action.wait()

# Installation
try:

  ## Setup dependencies
  check = raw_input('(Re)install dependencies? [y/n]: ')
  if check == 'y':
    bash('sudo apt-get update')
    bash('sudo apt-get install -y couchdb isc-dhcp-server arduino arduino-mk')
    bash('sudo apt-get install -y python-serial python-cherrypy3 python-couchdb python-matplotlib python-numpy python-pyaudio python-alsaaudio')
  else:
    pass

  ## Setup adhoc network
  check = raw_input('Setup adhoc? This will overwrite dhcpd.conf and interfaces [y/n]: ')
  if check == 'y':
    bash('mv config/interfaces .')
    with open('interfaces', 'a') as interfaces:
      random_essid = key_generator()
      random_psk = key_generator()
      interfaces.write('  wireless-essid ' + random_essid + '\n')
      interfaces.write('  wireless-psk ' + random_psk + '\n')
      print('ESSID: ' + random_essid)
      print('PSK: ' + random_psk)
    bash('sudo mv interfaces /etc/network/interfaces')
    bash('sudo cp -f config/dhcpd.conf /etc/dhcp/')
  else:
    pass

  ## Setup start on boot
  check = raw_input('Start HiveMind on boot? This will overwrite rc.local [y/n]: ')
  if check == 'y':
    bash('sudo cp -f config/rc.local /etc/')
    bash('sudo cp -f HiveMind.py /usr/bin/')
  else:
    pass

  ## Setup CouchDB
  check = raw_input('Create new CouchDB database? This will overwrite local.ini [y/n]: ')
  if check == 'y':
    import couchdb
    server = couchdb.Server()
    couch = server.create('hivemind_log')
    bash('sudo cp -f config/local.ini /etc/couchdb/')
  else:
    pass
    
  ## Setup Arduino
  check = raw_input('Setup Arduino? This will overwrite /usr/share/arduino/libraries [y/n]: ')
  if check == 'y':
    bash('cd config/alamode-setup && sudo ./setup')
    bash('sudo cp -rf libs/* /usr/share/arduino/libraries')
  else:
    pass
  
  ## Reboot
  check = raw_input('Reboot Now? [y/n]: ')
  if check == 'y':
    bash('sudo reboot')
  else:
    pass
    
except KeyboardInterrupt as error:
  print('Quitting Installation')
