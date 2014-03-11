# HiveMonitor
Arduino-based beehive sensor monitor.

## Setup
To install HiveMind, first connect the pi via an ethernet cable, then clone this repository:

    $ git clone https://github.com/trevstanhope/HiveMonitor.git

Before data can be read from the arduino, the monitor could needs to be uploaded to it.

First, navigate to the monitor/ directory:

    $ cd code/monitor

Then, clean the directory:

    $ make clean
    
After the directory is clean, compile the .ino:

    $ make
    
Lastly, upload the compiled code to the arduino:

    $ make upload
