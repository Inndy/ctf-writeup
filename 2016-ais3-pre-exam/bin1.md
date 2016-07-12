# bin1

``` python
#!/usr/bin/env python3

encrypted = bytearray(bytes.fromhex('CA7093C8067F23A1E0482A39AE54F279F2848B05A2521929C454AAF0CA'))
buffer = [0] * 29

for i in range(29):
    while encrypted[i] != ((((((i ^ buffer[i]) & 0xff) << ((i ^ 9) & 3)) & 0xff) | ((((i ^ buffer[i]) & 0xff) >> (8 - ((i ^ 9) & 3))) & 0xff)) + 8) & 0xff:
        buffer[i] += 1

print(bytes(buffer))
```

An angr solution: [bin1/angr-bin1.py](bin1/angr-bin1.py)
