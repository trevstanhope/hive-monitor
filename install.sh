#!/bi/sh
## Dependencies
sudo apt-get update
sudo apt-get install -y couchdb isc-dhcp-server arduino arduino-mk
sudo apt-get install -y python-serial python-cherrypy3 python-couchdb python-numpy python-pyaudio python-alsaaudio

## Setup adhoc network
sudo mv interfaces /etc/network/interfaces
sudo cp -f config/dhcpd.conf /etc/dhcp/

## Setup start on boot
sudo cp -f config/rc.local /etc/
sudo cp -f HiveMind.py /usr/bin/

## Setup CouchDB
sudo cp -f config/local.ini /etc/couchdb/
    
## Setup Arduino
cd config/alamode-setup && sudo ./setup
sudo cp -rf libs/* /usr/share/arduino/libraries

## Reboot
sudo reboot
