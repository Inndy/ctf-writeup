#!/usr/bin/env python2

from pwn import *
from hashlib import md5
from hashpumpy import hashpump

HOST, PORT = '127.0.0.1', 1234

def challenge_bruteforce(x):
    x = x.decode('hex')
    for i in xrange(1000000):
        if md5(x + str(i)).hexdigest().startswith('fffff'):
            return str(i)

def fast_challenge():
    p = remote(HOST, PORT)

    challenge = p.recvline_startswith('challenge: 0x')
    p.recv()

    challenge = challenge.split()[1][2:]
    print '[+] Challenge accepted: %s' % challenge
    res = challenge_bruteforce(challenge)
    if res == None:
        print '[*] Retry for slow reason'
        p.close()
        return None
    print '[+] Challenge cracked: %s' % res
    p.sendline(res)
    return p

def success_to_buy_or_not(p):
    while True:
        line = p.recvline()
        if "not correct" in line:
            return False
        elif "You buy" in line:
            return True

def exploit():
    p = None
    while p == None:
        p = fast_challenge()

    p.recvline_contains('-----------------------------------')  # wait motd

    p.sendline('1')  # Pick oranges
    p.sendline('1')  # 1 onrage

    item, checksum = p.recvline_startswith('num=').split('-----', 1)
    print "[+] hash = %s" % checksum
    print "[+] item = %s" % item

    for length in range(16, 0, -1):
        print '[*] Trying with length = %d' % length
        new_hash, value = hashpump(checksum, item, '&price=-10000000', length)
        payload = '%s-----%s' % (value, new_hash)
        print '[+] payload = %s' % repr(payload)

        p.sendline('2')
        p.sendline(payload)

        if success_to_buy_or_not(p):
            break

    p.sendline('4')  # flag?

    print
    print p.recvline_contains('the flag is').split(' ', 4)[-1]
    print

    p.close()

exploit()
