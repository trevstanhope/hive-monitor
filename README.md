# HiveMonitor
Arduino-based beehive sensor monitor.

## Setup
To install HiveMind, first connect the pi via an ethernet cable, then clone this repository:

    $ git clone https://github.com/trevstanhope/HiveMonitor.git
    
To install dependencies and libraries, run the install script.

    chmod +x install.sh
    ./install.sh

## Reloading Code
First, stop any serial communication by killing the python script:

    $ sudo pkill python
    
Then, clean the directory:

    $ make clean
    
After the directory is clean, compile the .ino:

    $ make
    
Lastly, upload the compiled code to the arduino:

    $ make upload
