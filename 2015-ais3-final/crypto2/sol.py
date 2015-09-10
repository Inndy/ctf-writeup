import binascii

range_lower = range(ord('a'), ord('z') + 1)

flag, *lower_case = [ binascii.unhexlify(l.strip()) for l in open('list.txt', 'r').readlines() ]

key = []
for i in range(32):
    possible = []
    for k in range(256):
        for l in lower_case:
            if l[i] ^ k not in range_lower:
                break
        else:
            possible.append(k)
    if len(possible) > 1:
        print('Index #%d -> %s' % (i, possible))
    key.append(possible[0])

print(''.join( chr(char ^ k) for char, k in zip(key, flag) ))
