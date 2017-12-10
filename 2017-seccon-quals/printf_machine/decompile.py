#!/usr/bin/env python3
import re

for line in open('default.fs'):
    line = line.rstrip('\n')
    r = re.sub(r'\d+', '1', line)
    # notice that index starts at 1, not 0
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
        fmt = 'b[{1}] = strlen(&b[{0}]);'

    elif r == "%1$*1$s%1$hhn":
        """
        %0$*1$s
        %2$hhn
        """
        t = 2
        args[1] -= 32
        fmt = 'b[{2}] = max(strlen(&b[{0}]), b[{1}]);'

    elif r == "%1$hhn%1$*1$s%1$hhn":
        """
        %0$hhn
        %1$*2$s
        %3$hhn
        """
        t = 3
        args[2] -= 32
        fmt = 'b[{0}] = 0; b[{3}] = max(strlen(&b[{1}]), b[{2}]);'

    elif r == "%1$*1$s%1$1s%1$hhn":
        """
        %0$*1$s
        %2$3s
        %4$hhn
        """
        t = 4
        args[1] -= 32
        fmt = 'b[{4}] = max(strlen(&b[{0}]), b[{1}]) + max(strlen(&b[{2}]), {3})'

    elif r == "%1$*1$s%1$*1$s%1$hhn":
        """
        %0$*1$s
        %2$*3$s
        %4$hhn
        """
        t = 5
        args[1] -= 32
        args[3] -= 32
        fmt = 'b[{4}] = max(strlen(&b[{0}]), b[{1}]) + max(strlen(&b[{2}]), b[{3}]);'

    print('/* %d */ ' % t + fmt.format(*args))
