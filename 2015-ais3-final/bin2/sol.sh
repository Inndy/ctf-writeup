#!/bin/bash

gcc fuck.c -o fuck && ./fuck | grep Possible | grep -o '[[:digit:]]\+' | while read inp; do echo $inp | ./rootme; done | strings | grep ais3
