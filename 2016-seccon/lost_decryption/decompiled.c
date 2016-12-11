#include "defs.h" // things from IDA Pro decompiler
#include <stdio.h>
#include <string.h>

unsigned __int64 sub_700(unsigned __int64 a1, unsigned __int64 a2);
void swap(__int64 *a1);
void encrypt(char *data, char *key);

unsigned char mapping[256] = {
    0x0a, 0x09, 0x03, 0x0f, 0x0c, 0x08, 0x0d, 0x01, 0x0e, 0x0b, 0x07, 0x02, 0x06, 0x00, 0x04, 0x05,
    0x28, 0x25, 0x1c, 0x24, 0x26, 0x1d, 0x27, 0x1d, 0x4e, 0x49, 0x3d, 0x41, 0x48, 0x3a, 0x49, 0x41,
    0x96, 0x8f, 0x7b, 0x88, 0x92, 0x7c, 0x96, 0x85, 0x0c, 0x01, 0xe2, 0xf4, 0x06, 0xe3, 0x0d, 0xf5,
    0xd2, 0xc1, 0x8f, 0xaf, 0xcc, 0x94, 0xd9, 0xb1, 0x16, 0xfb, 0xab, 0xde, 0x0e, 0xb4, 0x24, 0xe5,
    0x28, 0xfd, 0x7c, 0xd0, 0x1e, 0x8d, 0x43, 0xdd, 0x86, 0x41, 0x71, 0xf9, 0x78, 0x8e, 0xb5, 0x11,
    0xfe, 0x8f, 0x3f, 0x1c, 0xea, 0x70, 0x4e, 0x45, 0xdc, 0x29, 0x0a, 0x70, 0xbe, 0x5b, 0x61, 0xb5,
    0x3a, 0x19, 0xab, 0xef, 0x0c, 0x30, 0x15, 0x61, 0x7e, 0xab, 0x1f, 0xca, 0x36, 0xf8, 0xe4, 0x85,
    0x28, 0x35, 0x3c, 0x2c, 0xb6, 0x9d, 0x6f, 0x5d, 0x1e, 0x59, 0xd5, 0x71, 0x68, 0x12, 0xd1, 0x61,
    0x8a, 0xec, 0x49, 0xa0, 0x18, 0x71, 0xd3, 0x42, 0xa6, 0x06, 0x75, 0xd5, 0x3a, 0xa1, 0x02, 0x6e,
    0xf8, 0x10, 0x86, 0xdb, 0x42, 0xd4, 0x09, 0x8e, 0xb6, 0x14, 0xff, 0x66, 0xc4, 0xcb, 0x1f, 0xfa,
    0xb6, 0x0a, 0x91, 0xc7, 0x26, 0x65, 0xe8, 0x66, 0x44, 0xac, 0xf0, 0x83, 0xda, 0xa2, 0xab, 0xfe,
    0xa2, 0x14, 0xb5, 0x30, 0xc8, 0xfd, 0x17, 0x82, 0x8e, 0x06, 0xc9, 0x19, 0x22, 0x25, 0xd2, 0xbe,
    0xe8, 0xd8, 0xf6, 0x3f, 0x4a, 0x34, 0x5d, 0x5e, 0x6e, 0x0c, 0xc3, 0xee, 0x04, 0x6f, 0xf3, 0xba,
    0xee, 0xfa, 0xe5, 0x23, 0x4e, 0x79, 0x20, 0x96, 0xe4, 0xb4, 0x08, 0x17, 0x72, 0x6a, 0xf7, 0x4e,
    0x5a, 0xdc, 0x81, 0x60, 0xf8, 0x69, 0xab, 0x42, 0xf6, 0x46, 0x4d, 0x7d, 0x2a, 0x79, 0x02, 0xce,
    0x98, 0x20, 0x46, 0xf3, 0xf2, 0x34, 0x61, 0x2e, 0xc6, 0x64, 0x77, 0x76, 0x04, 0xc3, 0xe7, 0xfa
};

//----- (0000000000000700) ----------------------------------------------------
unsigned __int64 sub_700(unsigned __int64 a1, unsigned __int64 a2)
{
    unsigned __int64 v2; // r12@1
    unsigned __int64 v3; // rbx@1
    signed int v4; // ebp@1
    signed __int64 v5; // rax@2
    unsigned __int64 v6; // rbx@2
    unsigned __int64 v7; // rax@2
    unsigned __int64 v8; // rax@2
    unsigned __int64 v9; // rbx@2
    unsigned __int64 v10; // rsi@2
    unsigned __int64 v11; // rbx@2
    unsigned __int64 v12; // rdx@2
    signed __int64 v13; // rax@2
    unsigned __int64 v14; // rdx@2
    unsigned __int64 v15; // rax@2
    unsigned __int64 v16; // rcx@2
    unsigned __int64 v17; // rdx@2
    unsigned __int64 v18; // rax@2
    unsigned __int64 v19; // rcx@2
    int v20; // er14@4
    unsigned __int8 v21; // al@5
    char v22; // cl@5

    v2 = a1;
    v3 = a2;
    v4 = 5;
    do
    {
        v5 = (v2 ^ 0x200000000000000FLL * v2) + v3;
        v6 = 0xFA94B1238C6DD663LL * v3 + 0x2F3942D23A31A317LL;
        v7 = ((8 * v5 ^ v5) + 0x6ED0153C8F6D2B11LL) ^ v6 ^ 8 * v5 ^ v5;
        v8 = v6 ^ (((v7 >> 17) & 0xB78BC70454E32323LL ^ v7) - v6);
        v9 = v8 ^ v6;
        v10 = v8 ^ (v9 >> 1);
        v11 = 2 * v10 ^ v9;
        v12 = v10 ^ (v11 >> 2);
        v13 = v11 ^ 4 * v12;
        v14 = ((v11 ^ 4 * v12) >> 3) ^ v12;
        v15 = 8 * v14 ^ v13;
        v16 = 32 * (((v15 ^ 16 * (v14 ^ (v15 >> 4))) >> 5) ^ v14 ^ (v15 >> 4)) ^ v15 ^ 16 * (v14 ^ (v15 >> 4));
        v17 = ((v15 ^ 16 * (v14 ^ (v15 >> 4))) >> 5) ^ v14 ^ (v15 >> 4) ^ (v16 >> 6);
        v18 = v16 ^ (v17 << 6);
        v19 = v17 ^ (v18 >> 7);
        v2 = v19;
        v19 <<= 7;
        v3 = v19 ^ v18;
        if ( v18 != v19 )
            v2 *= v3;
        v20 = 0;
        do
        {
            // remove this function by static array since it's a byte-to-byte mapping function
            v21 = mapping[(unsigned __int8)(v3 >> v20)];
            v22 = v20;
            v20 += 8;
            v2 ^= v21 << v22;
        }
        while ( v20 != 64 );
        --v4;
    }
    while ( v4 );
    return v2;
}

//----- (0000000000000880) ----------------------------------------------------
void swap(__int64 *a1)
{
    __int64 v2; // rax@2
    __int64 v3; // rdx@2

    v2 = a1[1];
    v3 = a1[0];
    a1[1] = v3;
    a1[0] = v2;
}

//----- (00000000000008A0) ----------------------------------------------------
void encrypt(char *data, char *key)
{
    signed int i; // ebx@1
    __int64 v3; // rdx@1
    __int64 v4; // rdx@1
    __int64 v6; // rdx@3
    __int64 data_block[2]; // [sp+0h] [bp+0h]@1
    __int64 key_block[2]; // [sp+10h] [bp+10h]@1

    v3 = *((_QWORD *)data + 1);
    data_block[0] = *(_QWORD *)data;
    data_block[1] = v3;
    v4 = *((_QWORD *)key + 1);
    key_block[0] = *(_QWORD *)key;
    key_block[1] = v4;
    swap(data_block);
    for(i = 0; i < 14; i++)
    {
        data_block[0] ^= sub_700(data_block[1], key_block[1]);
        key_block[1] = sub_700(key_block[1], 0x9104F95DE694DC50LL);
        swap(data_block);
        swap(key_block);
    }
    swap(data_block);
    v6 = data_block[1];
    *(_QWORD *)data = data_block[0];
    *((_QWORD *)data + 1) = v6;
}

// copy and paste, then modify it to make a decrypt function
void decrypt(char *data, char *key)
{
    signed int i, j; // ebx@1
    __int64 data_block[2]; // [sp+0h] [bp+0h]@1
    __int64 key_block[2]; // [sp+10h] [bp+10h]@1

    memcpy(data_block, data, 16);
    swap(data_block);
    for(i = 0; i < 14; i++)
    {
        memcpy(key_block, key, 16);
        for(j = 13; j > i; j--) {
            key_block[1] = sub_700(key_block[1], 0x9104F95DE694DC50LL);
            swap(key_block);
        }
        swap(data_block);
        data_block[0] ^= sub_700(data_block[1], key_block[1]);
    }
    swap(data_block);
    memcpy(data, data_block, 16);
}

// too lazy to play with argv
#define KEY "key.bin"
#define INP "flag.enc"
#define OUT "flag"
#define FUN decrypt

int main()
{
    FILE *keyfile = fopen(KEY, "r");
    char key[16], k2[16];
    fread(key, 16, 1, keyfile);

    FILE *infile = fopen(INP, "r");
    FILE *outfile = fopen(OUT, "w");
    char block[16];
    for(int i = 0; i < 3; i++) {
        fread(block, 16, 1, infile);
        FUN(block, key);
        fwrite(block, 16, 1, outfile);
    }
}
