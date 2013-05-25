#!/bin/sh

./waf clean
./waf configure
./waf build
cp ./build/adventuretracker.pbw ~/Google\ Drive/adventuretracker/watchface/adventuretracker.pbw 

#/Users/njackson/Documents/Code/Python/libpebble-master/p.py --pebble_id 6302 reinstall ./build/adventuretracker.pbw