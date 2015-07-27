# http://www.mygeocachingprofile.com/codebreaker.vigenerecipher.aspx with key length = 9

import string

known_plain = "the vigenere cipher is a method of encrypting alphabetic text by using a series of different caesar ciphers based on the letters of a keyword. It is a simple form of polyalphabetic substitution."
encrypted = open('vigenere.txt', 'r').read()

a, b = known_plain, encrypted[:len(known_plain)]

print('Original:')
print(a[:64])
print(b[:64])

# find the different characters
a, b = list(zip(*[ (x, y) for x, y in zip(a, b) if x != y ]))

print('Cleaned:')
print(''.join(a[:64]))
print(''.join(b[:64]))

# known-plaintext attack to extract key
a, b = map(ord, a), map(ord, b)
key_sequence = [ (y - x) % 26 for x, y in zip(a, b) ]

for l in range(1, len(key_sequence) // 2):
    for i in range(l):
        if key_sequence[i] != key_sequence[i + l]:
            break
    else:
        print('Key length is %d' % l)
        break

key = key_sequence[:l]
print('Key is %s' % key)

class VigenereDecrypter(object):
    def __init__(self, key):
        self.key = key
        self.index = 0

    def __call__(self, ch):
        if ch not in string.ascii_lowercase:
            return ch
        code = ord(ch) - ord('a') - self.key[self.index]
        self.index = (self.index + 1) % len(self.key)
        return chr(ord('a') + code % 26)

decrypted = ''.join(map(VigenereDecrypter(key), encrypted))

print('Decrypted:')
print(decrypted)
