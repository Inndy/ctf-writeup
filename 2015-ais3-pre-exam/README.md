## misc-1

`The key is AIS3{hello_world}`

## misc-2

Use pkcrack to decrypt `key.txt`.

Here is the image:

![https://fbcdn-dragon-a.akamaihd.net/hphotos-ak-prn1/p960x960/851556_443281069111871_602278786_n.png](assets/851556_443281069111871_602278786_n.png)

``` sh
mkdir p960x960
curl https://fbcdn-dragon-a.akamaihd.net/hphotos-ak-prn1/p960x960/851556_443281069111871_602278786_n.png \
     -o p960x960/851556_443281069111871_602278786_n.png
zip fb_plain.zip p960x960/851556_443281069111871_602278786_n.png
pkcrack -C facebook.zip -c p960x960/851556_443281069111871_602278786_n.png \
        -P fb_plain.zip -p p960x960/851556_443281069111871_602278786_n.png \
        -d clean.zip -a
unzip clean.zip key.txt
```

## misc-3

c4 is a gzip compressed tarball and unpack it to get a bitmap file.

you will see some fuzzy words through fill tool and
use `hexdump c4.bmp` you will figure out a lot f all 0xff rows.

Here's a one-line solution:

``` sh
sed -i "s/\xff/\x00/g" b.bmp
```

![Cong, the key is AIS3{picture_in_picture_0xcc}](assets/c4-processed.bmp)

## web-1

There is a local file inclusion vulnerability:

`http://52.69.163.194/web1/?page=php://filter/convert.base64-encode/resource=index`

And you can see the flag in the comment of php code.

( @orangetw told me this :P )

## web-2

[JavaScript Deobfuscator](https://github.com/palant/jsdeobfuscator) plugin for Firefox is your friend.

![JavaScript Deobfuscator](assets/javascript-deobfuscator.png)

## web-3

Post payload is `username=root&password=\' or 1=1 #\`.

Don't ask me. I just fuzz it manually and I HATE WEB!

## bin-1

This is a packed PE with upx shell, but it was modified manually, so we can't just do `upx -d`.

You can just simply unpack it by
[this metohd](http://www.behindthefirewalls.com/2013/12/unpacking-upx-file-manually-with-ollydbg.html),
and you can analyze it now!

1. run it
2. open [Cheat Engine](http://cheatengine.org/)
3. scan string for `AIS3{` and you got the flag

## bin-2

This one is x64 shellcode.

``` sh
python2 -c "print \"$(tr -d '\r' < sc.txt | tr -d '\n')\"" | \
ndisasm -b64 -
```

```
// obviously, they are output content
00000000  48B8B5A3B9B1C641  mov rax,0x414141c6b1b9a3b5
         -4141
0000000A  50                push rax
0000000B  48B8BCA0A993AAA3  mov rax,0x93bea3aa93a9a0bc
         -BE93
00000015  50                push rax
00000016  48B8A993A5BF93BF  mov rax,0xa1a5bf93bfa593a9
         -A5A1
00000020  50                push rax
00000021  48B8BFA4A9A0A0AF  mov rax,0xa8a3afa0a0a9a4bf
         -A3A8
0000002B  50                push rax
0000002C  48B88D859FFFB7A3  mov rax,0x93a7a3b7ff9f858d
         -A793
00000036  50                push rax
00000037  4889E6            mov rsi,rsp
0000003A  4831D2            xor rdx,rdx
0000003D  803416CC          xor byte [rsi+rdx],0xcc // xor key
00000041  FEC2              inc dl
00000043  80FA25            cmp dl,0x25 // length
00000046  75F5              jnz 0x3d
00000048  4831C0            xor rax,rax
0000004B  48FFC0            inc rax
0000004E  4889C7            mov rdi,rax
00000051  0F05              syscall
00000053  6A3C              push byte +0x3c
00000055  58                pop rax
00000056  4831FF            xor rdi,rdi
00000059  0F05              syscall
0000005B  0A                db 0x0a

```

``` sh
python2 -c "print \"$(tr -d '\r' < sc.txt | tr -d '\n')\"" | \
ndisasm -b64 - | grep -o 'mov rax,\w\+' | \
tac | cut -b 11- | \
python2 -c 'import sys
data = sys.stdin.read().split()
data = map(lambda x: int(x, 16), data)
data = map(lambda x: x ^ 0xcccccccccccccccc, data)
data = map(lambda x: "%.16x" % x, data)
data = map(lambda x: x.decode("hex"), data)
data = map(lambda x: x[::-1], data)
print "".join(data)[:0x25]'
```

## bin-3

TBA

## pwn-1

TBA

## pwn-2

TBA

## pwn-3

TBA

## crypto-1

File name is a hint, `google("vigenere");` and you will find
[Vigenère cipher](https://en.wikipedia.org/wiki/Vigen%C3%A8re_cipher) on Wikipedia and
[Vigenère Cipher Codebreaker](http://www.mygeocachingprofile.com/codebreaker.vigenerecipher.aspx).

Here is the first result:

```
Based on repetitions in the encrypted text, the most probable key length is 27 characters.

...

-- MESSAGE w/Key #1 = 'jbhpsxohfjbnpoboomjbnpsbohf' ----------------
thk vikenere cithek bs a method of etcrcpting althauxtic text by usong e series oj diyyerent caesar iiplers baseh on mae letters of a qeyaord. it is e sifile form of polealthabetic wublmitution.thoumh tle cipher ms etly to understatd ard implemint, yhr three centuxiew it resisxed tel attempts to hreek it. many teoiee have tried tu imtlement ercrrition schemes zhax are essertitely vigenere cophirs. congretuettions, the key os ir http://ctf.eis3.hkg/files/thekeeofzigenerelahtaa.txt

-- MESSAGE w/Key #2 = 'jbhpsxohfjbnpoboomjbnpsbohp' ----------------
thk vikenere cithek bs a method ef etcrcpting althauxtic text bo usong e series oj diyyerent caeiar iiplers baseh on mae letters ef a qeyaord. it is e sifile form of folealthabetic wublmitution.txoumh tle cipher ms etly to underitatd ard implemint, yhr three cedtuxiew it resisxed tel attempti to hreek it. many teoiee have triud tu imtlement ercrrition scheces zhax are essertitely vigenehe cophirs. congretuettions, the aey os ir http://ctf.eis3.hkg/files/thukeeofzigenerelahtaa.txt

-- MESSAGE w/Key #3 = 'jbhpsxohfjbnpoboomjbnpsbosf' ----------------
thk vikenere cithek bs a methos of etcrcpting althauxtic text qy usong e series oj diyyerent catsar iiplers baseh on mae letterh of a qeyaord. it is e sifile form ou polealthabetic wublmitution.ihoumh tle cipher ms etly to undegstatd ard implemint, yhr three ctntuxiew it resisxed tel attempis to hreek it. many teoiee have trxed tu imtlement ercrrition schtmes zhax are essertitely vigentre cophirs. congretuettions, tht key os ir http://ctf.eis3.hkg/files/twekeeofzigenerelahtaa.txt
```

It seems the key length is much shorter, so we try again with the key size option. (len = 9)

```
-- MESSAGE w/Key #1 = 'jbnpsbohf' ----------------
the vigenere cipher is a method of encrypting alphabetic text by using a series of different caesar ciphers based on the letters of a keyword. it is a simple form of polyalphabetic substitution.though the cipher is easy to understand and implement, for three centuries it resisted all attempts to break it. many people have tried to implement encryption schemes that are essentially vigenere ciphers. congratulations, the key is in http://ctf.ais3.org/files/thekeyofvigenerehahaha.txt
```

Also, we can write a little program to perform known-plaintext attack if you notice that the pattern of blank is same as first paragraph of [Wikipedia page](https://en.wikipedia.org/wiki/Vigen%C3%A8re_cipher). 

[assets/crypto1.py](assets/crypto1.py)

```
Original:
the vigenere cipher is a method of encrypting alphabetic text by
cir kahsujaf pxhisy nb b ztlick to farjzdanwh nahioijcjp iwyh id
Cleaned:
thevigenerecipherisamethodofencryptingalphabetictextbyusingaseri
cirkahsujafpxhisynbbztlicktofarjzdanwhnahioijcjpiwyhiddtvcybglwr
Key length is 9
Key is [9, 1, 13, 15, 18, 1, 14, 7, 5] // IAMORANGE
Decrypted:
the vigenere cipher is a method of encrypting alphabetic text by using a series of different caesar ciphers based on the letters of a keyword. it is a simple form of polyalphabetic substitution.

though the cipher is easy to understand and implement, for three centuries it resisted all attempts to break it. many people have tried to implement encryption schemes that are essentially vigenere ciphers. congratulations, the key is in http://ctf.ais3.org/files/thekeyofvigenerehahaha.txt
```

## crypto-2

TBA

## crypto-3

Using [HashPump](https://github.com/bwall/HashPump). And this is the [exploit](assets/crypto3.py).
