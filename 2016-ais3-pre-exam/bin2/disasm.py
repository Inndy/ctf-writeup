#!/usr/bin/env python2

import os

data = eval(open('magic.txt').read())

open('magic.bin', 'wb').write(data)

os.system('ndisasm -b64 magic.bin > magic.asm')
