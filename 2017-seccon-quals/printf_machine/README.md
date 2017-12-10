# printf_machine

It's a printf language [script](default.fs) and a simple
[interpreter](fsmachine.c) which checks flag.

## solution

Count how many format are there by:

```
$ sed -e 's/[[:digit:]]\+/1/g' default.fs | sort | uniq -c | sort -nr
   3058 %1$*1$s%1$*1$s%1$hhn
    256 %1$hhn%1$*1$s%1$hhn
     33 %1$*1$s%1$hhn
     16 %1$s%1$hhn
     16 %1$hhn
     16 %1$*1$s%1$1s%1$hhn
```

So we have 6 different type of instructions, after that I wrote a
[decompiler](decompile.py) translate it to C-style code.

```
$ ./decompile.py
/* 2 */ b[4] = max(strlen(&b[2]), b[17]);
/* 2 */ b[17] = max(strlen(&b[2]), b[30]);
/* 2 */ b[30] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[18]);
/* 2 */ b[18] = max(strlen(&b[2]), b[24]);
/* 2 */ b[24] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[19]);
/* 2 */ b[19] = max(strlen(&b[2]), b[27]);
/* 2 */ b[27] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[22]);
/* 2 */ b[22] = max(strlen(&b[2]), b[27]);
/* 2 */ b[27] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[23]);
/* 2 */ b[23] = max(strlen(&b[2]), b[31]);
/* 2 */ b[31] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[24]);
/* 2 */ b[24] = max(strlen(&b[2]), b[32]);
/* 2 */ b[32] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[26]);
/* 2 */ b[26] = max(strlen(&b[2]), b[31]);
/* 2 */ b[31] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[27]);
/* 2 */ b[27] = max(strlen(&b[2]), b[32]);
/* 2 */ b[32] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[29]);
/* 2 */ b[29] = max(strlen(&b[2]), b[31]);
/* 2 */ b[31] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[30]);
/* 2 */ b[30] = max(strlen(&b[2]), b[32]);
/* 2 */ b[32] = max(strlen(&b[2]), b[4]);
/* 2 */ b[4] = max(strlen(&b[2]), b[31]);
/* 2 */ b[31] = max(strlen(&b[2]), b[32]);
/* 2 */ b[32] = max(strlen(&b[2]), b[4]);
/* 0 */ b[3] = 0;
/* 3 */ b[4] = 0; b[9] = max(strlen(&b[2]), b[17]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[4] = max(strlen(&b[2]), b[4]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[4] = max(strlen(&b[2]), b[4]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[4] = max(strlen(&b[2]), b[4]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[4] = max(strlen(&b[2]), b[4]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[4] = max(strlen(&b[2]), b[4]) + max(strlen(&b[2]), b[9]);
/* 5 */ b[9] = max(strlen(&b[2]), b[9]) + max(strlen(&b[2]), b[9]);

(.. and more)
```

After some analyze,  we can perform some optimize since we know that ins 2 is simple assignment, ins 5 is add...etc.

There's a pattern in instruction sequances, if we write instruction sequance into regular expression, if would be: `2+(0(35+)+415)+`.

At first the script will swap flag chars (`2+` part), then check each character (`(35+)+` part), save compare result to `b[16]` (`415` part)

Finally we wrote a [optimized decompiler](compile_to_z3.py) with a simple finite state machine to generate z3 SMT sovler script.

```
$ ./compile_to_z3.py | python2
sat
SECCON{Tur!n9-C0mpl3t3?}
```
