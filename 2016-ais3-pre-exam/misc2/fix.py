#!/usr/bin/env python3

import sys

with open(sys.argv[1], "rb+") as f:
    assert f.read(2) == b'7Z'
    f.seek(0)
    f.write(b'7z')
