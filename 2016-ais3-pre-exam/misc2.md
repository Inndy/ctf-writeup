# misc2

```
inndy $ xxd UNPACK_ME | head
00000000: 375a bcaf 271c 0003 97fa 34cd bf75 0400  7Z..'.....4..u..
00000010: 0000 0000 2400 0000 0000 0000 0f6d 9320  ....$........m.
00000020: 7fc7 1bd6 3232 8b34 c4a3 bda1 35ff 1bfe  ....22.4....5...
00000030: 7606 9e02 4b37 0f8a 77d6 0fba c7d4 3364  v...K7..w.....3d
00000040: 71f1 cc64 a788 ee07 1d2e 4e63 da79 3394  q..d......Nc.y3.
00000050: 9dc3 16d0 e647 7f52 b7a1 647c fbc4 2742  .....G.R..d|..'B
00000060: 8e22 2d36 6608 bb4a ce54 68eb c037 efd2  ."-6f..J.Th..7..
00000070: c22d 25a9 f93f 64f2 2858 0884 04ea 3691  .-%..?d.(X....6.
00000080: bb33 f9e8 b353 8f09 d9a5 787b c91d 48a8  .3...S....x{..H.
00000090: 851d f1ef 4443 f359 e11a f75e 8e71 8e70  ....DC.Y...^.q.p
```

看到 7Z 大概就知道了，不過沒辦法解壓縮，因為真正的 7z header 是 `7z` 不是 `7Z`

``` python
#!/usr/bin/env python3

import sys

with open(sys.argv[1], "rb+") as f:
    assert f.read(2) == b'7Z'
    f.seek(0)
    f.write(b'7z')
```

```
inndy $ ./fix.py UNPACK_ME
inndy $ 7z l UNPACK_ME

7-Zip [64] 15.14 : Copyright (c) 1999-2015 Igor Pavlov : 2015-12-31
p7zip Version 15.14.1 (locale=utf8,Utf16=on,HugeFiles=on,64 bits,4 CPUs x64)

Scanning the drive for archives:
1 file, 292355 bytes (286 KiB)

Listing archive: UNPACK_ME

--
Path = UNPACK_ME
Type = 7z
Physical Size = 292355
Headers Size = 227
Method = LZMA:384k 7zAES
Solid = -
Blocks = 1

   Date      Time    Attr         Size   Compressed  Name
------------------- ----- ------------ ------------  ------------------------
2016-05-27 00:29:08 ....A       288200       292128  tArdCNLMPjLxqs5USx3T
2016-05-27 00:29:08 ....A            0            0  UDJRRDVRJyfbWBxEMLEX
------------------- ----- ------------ ------------  ------------------------
2016-05-27 00:29:08             288200       292128  2 files
```

接者可以猜密碼是 `UDJRRDVRJyfbWBxEMLEX`

```
inndy $ 7z -p'UDJRRDVRJyfbWBxEMLEX' x UNPACK_ME

7-Zip [64] 15.14 : Copyright (c) 1999-2015 Igor Pavlov : 2015-12-31
p7zip Version 15.14.1 (locale=utf8,Utf16=on,HugeFiles=on,64 bits,4 CPUs x64)

Scanning the drive for archives:
1 file, 292355 bytes (286 KiB)

Extracting archive: UNPACK_ME
--
Path = UNPACK_ME
Type = 7z
Physical Size = 292355
Headers Size = 227
Method = LZMA:384k 7zAES
Solid = -
Blocks = 1

Everything is Ok

Files: 2
Size:       288200
Compressed: 292355
inndy $ ls -l
total 576
-rw-r--r-- 1 inndy staff      0  5 27 00:29 UDJRRDVRJyfbWBxEMLEX
-rw-r--r-- 1 inndy staff 292355  7 12 13:21 UNPACK_ME
-rwxr-xr-x 1 inndy staff    135  7 12 13:21 fix.py
-rw-r--r-- 1 inndy staff 288200  5 27 00:29 tArdCNLMPjLxqs5USx3T
```

真的解出來了！

```
inndy $ ./fix.py tArdCNLMPjLxqs5USx3T
inndy $ 7z l -p'UDJRRDVRJyfbWBxEMLEX' tArdCNLMPjLxqs5USx3T

7-Zip [64] 15.14 : Copyright (c) 1999-2015 Igor Pavlov : 2015-12-31
p7zip Version 15.14.1 (locale=utf8,Utf16=on,HugeFiles=on,64 bits,4 CPUs x64)

Scanning the drive for archives:
1 file, 288200 bytes (282 KiB)

Listing archive: tArdCNLMPjLxqs5USx3T

--
Path = tArdCNLMPjLxqs5USx3T
Type = 7z
Physical Size = 288200
Headers Size = 248
Method = Copy 7zAES
Solid = -
Blocks = 2

   Date      Time    Attr         Size   Compressed  Name
------------------- ----- ------------ ------------  ------------------------
2016-05-27 00:29:08 ....A       287912       287920  GkBBGdDtgCbEaSxRL4Bv
2016-05-27 00:29:08 ....A           21           32  secret.txt
------------------- ----- ------------ ------------  ------------------------
2016-05-27 00:29:08             287933       287952  2 files
```

這次的密碼跟 `UNPACK_ME` 是一樣的，剩下的東西密碼在 `secret.txt`，另一個是壞掉的壓縮檔

用這個 script 解壓縮

``` shell
#!/bin/bash

mkdir -p tmp
cd tmp
NEXT="$(7z l -p"$2" ../"$1" | grep '^2016' | grep -v ' files$' | awk '{print $6}' | tee /dev/stderr | head -n1)"
7z x -p"$2" ../"$1"
PASS="$(cat secret.txt)"
echo "$PASS"

ls -l

../fix.py "$NEXT"
mv * ..

cd ..
exec ./unpack.sh "$NEXT" "$PASS"
```

最後會解出 `flag.txt`

`ais3{7zzZzzzZzzZzZzzZiP}`
