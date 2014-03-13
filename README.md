# HiveMonitor
Arduino-based beehive sensor monitor.

## Setup
To install HiveMind, first connect the pi via an ethernet cable, then clone this repository:

    $ git clone https://github.com/trevstanhope/HiveMonitor.git
    
To install dependencies and libraries, run the install script.

    chmod +x install.sh
    ./install.sh

Before data can be read from the arduino, the monitor code needs to be uploaded to it.  
First, navigate to the monitor/ directory:

    $ cd code/monitor

Then, clean the directory:

    $ make clean
    
After the directory is clean, compile the .ino:

    $ make
    
Lastly, upload the compiled code to the arduino:

    $ make upload
