# remote1

反編譯內容：

``` c
int __cdecl main(int argc, const char **argv, const char **envp)
{
  unsigned int time_seconds; // eax@1
  int result; // eax@2
  __int64 canary_check; // rbx@8
  int random_number; // [sp+0h] [bp-2030h]@1
  const char *path; // [sp+8h] [bp-2028h]@4
  char buffer[8192]; // [sp+10h] [bp-2020h]@1
  __int64 stack_canary; // [sp+2018h] [bp-18h]@1

  stack_canary = *MK_FP(__FS__, 40LL);
  time_seconds = time(0LL);
  srand(time_seconds);
  random_number = rand();
  setvbuf(stdin, 0LL, 2, 0LL);
  setvbuf(stdout, 0LL, 2, 0LL);
  printf("Enter passcode: ");
  if ( fgets(buffer, 0x2000, stdin) )
  {
    if ( (random_number ^ (unsigned int)strtol(buffer, 0LL, 0)) == 538354003 )
    {
      path = getenv("HOME");
      if ( path )
        chdir(path);
      puts("Correct!");
      system("/bin/cat flag.txt");
      result = 0;
    }
    else
    {
      puts("Incorrect passcode!");
      result = 0;
    }
  }
  else
  {
    result = -1;
  }
  canary_check = *MK_FP(__FS__, 40LL) ^ stack_canary;
  return result;
}
```

這題的解法是基於 `time(NULL)` 可以被猜中， `glibc` 用的 PRNG algorithm 基本上都是一樣的

Solution:

``` c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    int fds[2], status, fix = 0;

    if(argc > 1) {
        fix = atoi(argv[1]);
    }

    pipe(fds); // create fd

    pid_t pid = fork();
    if(pid > 0) {
        // this is parent process
        close(fds[0]);
        dup2(fds[1], 1);
        close(fds[1]);

        srand(time(NULL) + fix);
        printf("%d\n", (int)(rand() ^ 0x2016A153));
        fflush(stdout);
        waitpid(pid, &status, 0);
    } else {
        // this is child process
        close(fds[1]);
        dup2(fds[0], 0);
        close(fds[0]);

        return execlp("/bin/nc", "nc", "quiz.ais3.org", "2154", NULL);
    }
}
```

`ais3{sEEd_is_cRiTiCaL_@_@}`
