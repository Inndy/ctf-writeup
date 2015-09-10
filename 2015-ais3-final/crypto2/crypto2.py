#!/usr/bin/env python

from Crypto.Cipher import AES
import string
import random
import sys
import hashlib


key = "XXXXXXXXXXXXXXXX"
iv = ''.join(random.choice(string.hexdigits) for _ in range(16))
flag = "ais3{NEVERPHDNEVERPHDNEVERPHD..}" # Not real flag ...

def encrypt(p):
    return AES.new(key, AES.MODE_OFB, iv).encrypt(p)


print \
"""
    _    ___ ____ _____   ____   _    ____ ____   ____ ___  ____  _____
   / \  |_ _/ ___|___ /  |  _ \ / \  / ___/ ___| / ___/ _ \|  _ \| ____|
  / _ \  | |\___ \ |_ \  | |_) / _ \ \___ \___ \| |  | | | | | | |  _|
 / ___ \ | | ___) |__) | |  __/ ___ \ ___) |__) | |__| |_| | |_| | |___
/_/   \_\___|____/____/  |_| /_/   \_\____/____/ \____\___/|____/|_____|
"""

print "Try to decode the cipher:"
print encrypt(flag).encode("hex")
print "===================================================================="
sys.stdout.flush()



while True:
    print "Calcuate the passcode...(Press any key to continue)"
    sys.stdout.flush()
    raw_input()
    p = ''.join(random.choice(string.lowercase) for _ in range(32))
    print encrypt(p).encode("hex")
    sys.stdout.flush()
