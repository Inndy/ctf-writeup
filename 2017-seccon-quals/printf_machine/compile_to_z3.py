#!/usr/bin/env python3
import re

SWAP, BEGIN, SEQ, END = range(4)

HEAD ="""\
#!/usr/bin/env python2
from z3 import *

s = Solver()
b = [0] * 17
flag = [ BitVec('b%d' % i, 32) for i in range(16) ]
for c in flag:
    s.add(c < 0x7f)
    s.add(c >= 0x20)

b += flag

"""

TAIL = """
print(s.check())
m = s.model()

F = ''
for i in flag:
    F += chr(m[i].as_long())
print('SECCON{%s}' % F)
"""

state = SWAP

def emit(fmt, args):
    args = [i if i < 16 else '&%d' % (i - 16) for i in args]
    print(fmt.format(*args))

def proc(t, args, fmt):
    global state

    # print('/* %d */' % t, end=' ')

    if state == SWAP:
        if t == 2:
            if 4 not in args:
                #print('swap({1}, {2});'.format(*args))
                print('b[{1}], b[{2}] = b[{2}], b[{1}];'.format(*args))
        else:
            state = BEGIN
            return proc(t, args, fmt)
    elif state == BEGIN:
        if t == 0:
            print(fmt.format(*args))
        elif t == 3:
            print(fmt.format(*args))
            #print('b[{3}] = b[{2}]'.format(*args))
        elif t == 5:
            print(fmt.format(*args))
        elif t == 4:
            state = END
            return proc(t, args, fmt)
    elif state == END:
        r = fmt.format(*args)
        #print(r, args, end = ' // ')
        if t == 4:
            assert r.startswith('b[1] = b[3] + ')
            print('s.add((b[3] + %d) %% 256 == 0)' % args[3])
        elif t == 1:
            #print('** 1')
            pass
        elif t == 5:
            #print('** 5')
            pass
            state = BEGIN

print(HEAD)

for line in open('default.fs'):
    line = line.rstrip('\n')
    r = re.sub(r'\d+', '1', line)
    args = [ int(i) for i in re.findall(r'\d+', line) ]

    if r == "%1$hhn":
        t = 0
        fmt = 'b[{0}] = 0;'

    elif r == "%1$s%1$hhn":
        """
        %0$s
        %1$hhn
        """
        t = 1
        if args[0] == 2:
            assert False
            fmt = 'b[{1}] = 0'
        else:
            fmt = 'b[{1}] = strlen(&b[{0}]);'

    elif r == "%1$*1$s%1$hhn":
        """
        %0$*1$s
        %2$hhn
        """
        t = 2
        # fmt = 'b[{2}] = max(strlen(&b[{0}]), b[{1}]);'
        args[1] -= 32
        fmt = 'b[{2}] = b[{1}]'

    elif r == "%1$hhn%1$*1$s%1$hhn":
        """
        %0$hhn
        %1$*2$s
        %3$hhn
        """
        t = 3
        args[2] -= 32
        if args[1] == 2:
            fmt = 'b[{0}] = 0; b[{3}] = b[{2}];'
        else:
            fmt = 'b[{0}] = 0; b[{3}] = max(strlen(&b[{1}]), b[{2}]);'

    elif r == "%1$*1$s%1$1s%1$hhn":
        """
        %0$*1$s
        %2$3s
        %4$hhn
        """
        t = 4
        args[1] -= 32

        if args[0] == 2:
            fmt1 = 'b[{4}] = b[{1}]'
        else:
            fmt1 = 'b[{4}] = max(strlen(&b[{0}]), b[{1}])'

        if args[2] == 2:
            fmt2 = ' + {3}'
        else:
            fmt2 = ' + max(strlen(&b[{2}]), {3})'

        fmt = fmt1 + fmt2

    elif r == "%1$*1$s%1$*1$s%1$hhn":
        """
        %0$*1$s
        %2$*3$s
        %4$hhn
        """
        t = 5
        args[1] -= 32
        args[3] -= 32
        if args[0] == 2:
            fmt1 = 'b[{4}] = b[{1}]'
        else:
            fmt1 = 'b[{4}] = max(strlen(&b[{0}]), b[{1}])'
        if args[2] == 2:
            fmt2 = ' + b[{3}];'
        else:
            fmt2 = ' + max(strlen(&b[{2}]), b[{3}]);'
        fmt = fmt1 + fmt2

    #print('/* %d */ ' % t + fmt.format(*args))
    proc(t, args, fmt)

print(TAIL)
