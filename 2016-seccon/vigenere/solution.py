#!/usr/bin/env python3

from re import match
from hashlib import md5 as md5_
from itertools import product
md5 = lambda x: md5_(x.encode()).hexdigest()

TABLE = "ABCDEFGHIJKLMNOPQRSTUVWXYZ{}"
ENCRYPT, DECRYPT = 1, -1

def vigenere(string, key, mode=ENCRYPT, table=TABLE):
    L = len(key)
    key = [ table.index(i) for i in key ]
    string = [ table.index(i) for i in string ]
    cipher = [ (v + mode * key[i % L]) % len(table) for i, v in enumerate(string) ]
    return ''.join(table[i] for i in cipher)

key        = ".......?????"
plaintext  = "SECCON{...................................}"
ciphertext = "LMIG}RPEDOEEWKJIQIWKJWMNDTSR}TFVUFWYOCBAJBQ"
md5_hash   = "f528a6ab914c1ecf856a1d93103948fe"

known_key = ''
for idx, char in enumerate('SECCON{'):
    for k in TABLE:
        if vigenere(char, k) == ciphertext[idx]:
            known_key += k
            break

print('known_key = %s' % known_key)

for poss in product(TABLE, repeat=len(key) - len(known_key)):
    try_key = known_key + ''.join(poss)
    decrypted = vigenere(ciphertext, try_key, DECRYPT)
    if match(r'SECCON\{[A-Z]{35}\}', decrypted) and \
       md5(decrypted) == 'f528a6ab914c1ecf856a1d93103948fe':
        print('key = %s, plaintext = %s' % (try_key, decrypted))
        exit()
