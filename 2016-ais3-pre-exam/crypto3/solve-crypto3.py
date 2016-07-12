import base64
import math

pub_e = 0xab1ce13
pubkeys = []

for i in range(100):
    data = ''.join(line for line in open('%d.pub' % i).read().split('\n') if 'KEY' not in line)
    n = (4 - len(data) % 4) % 4
    data += '=' * n
    data = base64.b64decode(data)
    assert data[:3] == bytes.fromhex('308201')
    assert data[-3:] == bytes.fromhex('b1ce13')
    pubkey = int.from_bytes(data[0x20:-6], 'big')
    pubkeys.append(pubkey)

pubkey_pq = [None] * len(pubkeys)

for i in range(len(pubkeys)):
    for j in range(i+1, len(pubkeys)):
        v = math.gcd(pubkeys[i], pubkeys[j])
        if v > 1:
            pubkey_pq[i] = (v, pubkeys[i] // v)
            pubkey_pq[j] = (v, pubkeys[j] // v)

assert all(pubkey_pq)

def extended_gcd(aa, bb):
    lastremainder, remainder = abs(aa), abs(bb)
    x, lastx, y, lasty = 0, 1, 1, 0
    while remainder:
        lastremainder, (quotient, remainder) = remainder, divmod(lastremainder, remainder)
        x, lastx = lastx - quotient*x, x
        y, lasty = lasty - quotient*y, y
    return lastremainder, lastx * (-1 if aa < 0 else 1), lasty * (-1 if bb < 0 else 1)

def modinv(a, m):
    g, x, y = extended_gcd(a, m)
    if g != 1:
        raise ValueError
    return x % m

flag_enc = int.from_bytes(open('../flag.enc', 'rb').read(), 'big')

for (p, q) in pubkey_pq:
    phi_n = (p - 1) * (q - 1)
    try:
        d = modinv(pub_e, phi_n)
    except ValueError:
        continue
    flag_dec = pow(flag_enc, d, p * q)
    flag_dec = flag_dec.to_bytes(256, 'big')
    if b'ais3' in flag_dec:
        print(flag_dec.strip(b'\0'))
