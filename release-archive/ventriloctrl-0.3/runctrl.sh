#!/bin/sh

# Config, see README for instructions
EVENT_DEVICE="/dev/input/event0"
INPUT_KEY="86"


# Don't touch
./ventriloctrl $EVENT_DEVICE $INPUT_KEY
