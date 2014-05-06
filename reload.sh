#!/bin/sh
sudo pkill python
make clean
make
make upload
