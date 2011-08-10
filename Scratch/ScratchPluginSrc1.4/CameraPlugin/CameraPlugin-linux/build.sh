#!/bin/sh
# Fedora Linux script
rm -f *.o CameraPlugin
gcc -fPIC -Wall -c *.c
gcc -shared -lv4l2 *.o -o CameraPlugin
rm -f *.o
