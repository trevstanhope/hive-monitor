# Hivemind
Wireless sensor node based on RaspberryPi and Arduino.
HiveMind allows any amateur bee-keeper to monitor their hives in real-time.

## Formating SD
Download latest raspbian image file and unzip it:

    $ wget http://downloads.raspberrypi.org/raspbian_latest
    $ unzip *rasbian.zip
    
Unmount all partitions of the SD card:

    $ sudo umount /dev/mmcblk0p*
    
Write the image file to the SD card:

    $ dd bs=4M if=yyyy-mm-dd-wheezy-raspbian.img of=/dev/mmcblk0

## First Boot
The first time the device is booted, it will prompt to set configurations.
This process can be repeated later with the command:

    $ raspi-config
    
When in the configuration editor, the following are ideal settings:

1. Expand the Filesystem --> yes
2. Internationalization Options --> Keyboard Layout --> English US
3. Advanced Options --> SSH --> enable
4. Enable Desktop on Boot --> no

## Setup
To install HiveMind, first connect the pi via an ethernet cable, then clone this repository:

    $ git clone https://github.com/osag/HiveMind.git

Navigate to the newly cloned directory and run setup.py:

    $ cd HiveMind/
    $ sudo python setup.py

You will be prompted for several options, but on a clean installation simply enter 'y' for each.

**IMPORTANT: Copy down the random essid and password, this is how you'll connect to the box**

## Compile to Arduino
Before data can be read from the arduino, the monitor could needs to be uploaded to it.

**IMPORTANT: The pi must have been rebooted after running setup.py**

First, navigate to the monitor/ directory:

    $ cd code/monitor

Then, clean the directory:

    $ make clean
    
After the directory is clean, compile the .ino:

    $ make
    
Lastly, upload the compiled code to the arduino:

    $ make upload




    
    

