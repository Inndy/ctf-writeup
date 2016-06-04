base64 = 'qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789/'
enc = '\'&+\x05\x16Mp\x14=*\x07&\x0c\tX\x1f L\x0c[ 3D8\x15\x13\x04\n"#\x19\x18\x1f\t\x1f' 

ascii_chars = ''.join(map(chr, range(0x20, 0x7f)))
lowercase = ''.join(chr(i) for i in range(ord('a'), ord('z') + 1))
uppercase = lowercase.upper()
alphabet = lowercase + uppercase
digits = ''.join(str(i) for i in range(10))
hostname = lowercase + digits + '-_.'

def find_possible(c):
    c = ord(c)
    return ''.join(sorted(chr(c ^ k) for k in map(ord, base64) if 0x20 <= c^k < 0x7f))

def expand(s, n):
    while n > 0:
        n -= 1
        s = s.encode('base64')
    return s

#for i, c in enumerate(enc):
#    print '%2d : %s' % (i, find_possible(c))

#for s in base64[:-2]:
#    r = ('WTBkbmR' + s).decode('base64')
#    if r[-1] in base64[:-2]:
#        print repr((r+'AA').decode('base64'))
#exit()
import itertools
possible = lowercase + uppercase + digits + '.-_'

def exp_key(hn):
    i = 0
    while True :
        i += 1
        hn = hn.encode('base64')
        if len(hn) > len(enc):
            return hn

def xor(a, b):
    for i, j in zip(a, b):
        yield chr(ord(i)^ord(j))

def decrypt(hn):
    hn = exp_key(hn)
    #print 'hn = ' + hn
    executeMe = ""
    return ''.join(xor(hn[:len(enc)], enc))

domains = 'google trendmicro trend gov google yahoo localhost example facebook gmail msn wretch'.split()
prefix = ('', 'www.')
suffix = '.tw .com .com.tw .org .org.tw .cc .us .gg .local'.split()
suffix = tuple(suffix + [''])

def expand(bn):
    for p in prefix:
        for s in suffix:
            yield p + bn + s

def check(s):
    return any(c not in ascii_chars for c in s)

for a in ascii_chars:
    hn = 'ph-marvinc-' + a + ''
    r = decrypt(hn)
    if r.startswith('print "\\nFound my home '):
        print hn, r

# ph-marvinc-m print "\nFound my home at last!!\n"
exit()

for a in hostname:
    for b in hostname:
        hn = 'ph-marvin' + a + b + 'c'
        r = decrypt(hn)
        if r.startswith('print "\\nFound my home'):
            print hn, r
exit()

for a in 'lmn2367-_': # 'o' for \n
    for b in hostname:
        r = decrypt('ph-' + a + b + 'aaaaa')
        q = r[:12]
        if ']' not in q and '[' not in q and '^' not in q:
            print a, b, r
