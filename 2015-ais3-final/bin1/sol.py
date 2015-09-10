#!/usr/bin/env python3

import sys
from subprocess import Popen, PIPE, STDOUT

cmd_line = "/home/ais/pin/pin -t /home/ais/pin/source/tools/ManualExamples/obj-intel64/inscount0.so -- ./evaluation64_a"

def run(input_data):
    to_process_data = input_data.ljust(24, '!')
    p = Popen(cmd_line.split() + [to_process_data], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
    dout = p.communicate()[0]
    r = dout.decode().strip()
    n = open('inscount.out', 'r').read().split()[1]
    return int(n, 10)

def bruteforce(prefix='', ins=0):
    possible = '_{}abcdefghijklmnopqrstuvwxyz'
    possible += 'I' # possible.upper()
    possible += '0123456789'

    if prefix[-1] == '}':
        print('Flag is %s' % prefix)
        sys.exit(0)

    print('From: %d' % ins)
    for ch in possible:
        ins_count = run(prefix + ch)
        print('Count: %d Trying: %s%s' % (ins_count, prefix, ch))
        if ins_count > ins:
            bruteforce(prefix + ch, ins_count)

last = sys.argv[1] if len(sys.argv) > 1 else ''
bruteforce(last, run(last))
