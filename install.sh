#!/bi/sh
## Dependencies
sudo apt-get update
sudo apt-get install -y couchdb isc-dhcp-server arduino arduino-mk
sudo apt-get install -y python-serial python-cherrypy3 python-couchdb python-numpy python-pyaudio python-alsaaudio

## Setup adhoc network
sudo mv config/interfaces /etc/network/interfaces
sudo cp -f config/dhcpd.conf /etc/dhcp/

## Setup start on boot
sudo cp -f config/rc.local /etc/

## Setup CouchDB
sudo cp -f config/local.ini /etc/couchdb/
    
## Setup Arduino
sudo cp -rf libs/* /usr/share/arduino/libraries
sudo cp config/avrdude /usr/bin/avrdude
sudo cp config/avrdude /usr/share/arduino/hardware/tools
sudo cp config/avrdude.conf  /usr/share/arduino/hardware/tools
sudo cp config/boards.txt  /usr/share/arduino/hardware/arduino
sudo cp config/cmdline.txt /boot
sudo cp config/inittab /etc
sudo cp config/80-alamode.rules /etc/udev/rules.d
sudo chown root /usr/bin/avrdude /usr/share/arduino/hardware/tools/avrdude
sudo chgrp root /usr/bin/avrdude /usr/share/arduino/hardware/tools/avrdude
sudo chmod a+s /usr/bin/avrdude /usr/share/arduino/hardware/tools/avrdude

## Reboot
sudo reboot
