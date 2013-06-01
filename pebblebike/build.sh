#!/bin/sh

./waf clean
./waf configure
./waf build
cp ./build/pebblebike.pbw ~/Google\ Drive/adventuretracker/watchface/pebblebike.pbw 

#/Users/njackson/Documents/Code/Python/libpebble-master/p.py --pebble_id 6302 reinstall ./build/adventuretracker.pbw