# remote3

fastbin + heap overflow，fastbin 機制可以在
[angelboy 的投影片](http://www.slideshare.net/AngelBoy1/heap-exploitation-51891400) 裡面找到

值得注意的地方是，這題的 NX 沒開，可以直接跳 shellcode，省下了不少麻煩

```
inndy $ checksec.py remote3
File: remote3
[*] Stack is executable!
RELRO:         Partial RELRO
Stack Canary:  Canary found
NX:            NX disabled
PIE:           No PIE
RPATH:         No RPATH
RUNPATH:       No RUNPATH
```

反編譯後的程式碼：

``` c
typedef struct
{
    char *(*fn_get_name)(STUDENT *);
    int (*fn_get_id)(STUDENT *);
    int id;
    char name[8];
} STUDENT;

STUDENT *current_stu;
char *note[10];
char info[100];

int get_id(STUDENT *stu)
{
    return stu->id;
}

char *get_name(STUDENT *stu)
{
    return stu->name;
}

void menu()
{
    puts("-------------------------");
    puts(" 1. Add student          ");
    puts(" 2. Show a student       ");
    puts(" 3. Add a note           ");
    puts(" 4. Show a note          ");
    puts(" 5. Delete a note        ");
    puts(" 6. exit                 ");
    puts("-------------------------");
    printf("Your choice :");
}

void addstu()
{
    STUDENT *stu; // ST1C_4@1

    stu = (STUDENT *)malloc(0x14u);
    printf("ID:");
    scanf("%d", &stu->id);
    printf("Name:");
    scanf("%7s", stu->name);
    stu->fn_get_name = get_name;
    stu->fn_get_id = get_id;
    current_stu = stu;
}

void showstu()
{
    int stu_id; // eax@2
    int stu_name; // eax@2

    if ( current_stu )
    {
        stu_id = current_stu->fn_get_id(current_stu);
        printf("ID:%d\n", stu_id);
        stu_name = current_stu->fn_get_name(current_stu);
        printf("Name:%s\n", stu_name);
    }
}

void addnote()
{
    size_t size; // [sp+18h] [bp-10h]@5
    int i; // [sp+1Ch] [bp-Ch]@1

    for ( i = 0; i <= 9 && note[i]; ++i )
        ;
    printf("Size:");
    scanf("%d", &size);
    note[i] = (char *)malloc(size);
    printf("Content:");
    scanf("%s", note[i]);
}

void shownote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    if ( index >= 0 && index <= 10 )
    {
        if ( note[index] )
            printf("Content:%s\n", note[index]);
        else
            puts("No such note");
    }
}

void delnote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    free(note[index]);
}

int main(int argc, const char **argv, const char **envp)
{
    char buff[20]; // [sp+18h] [bp-18h]@2

    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    printf("Welcome, please leave some message for me:");
    read(0, info, 100u);

    while ( 1 )
    {
        menu();
        read(0, buff, 20u);
        switch ( atoi(buff) )
        {
            case 1:
                addstu();
                break;
            case 2:
                showstu();
                break;
            case 3:
                addnote();
                break;
            case 4:
                shownote();
                break;
            case 5:
                delnote();
                break;
            case 6:
                puts("exit");
                return 0;
            default:
                puts("Invalid choice");
                break;
        }
    }
}
STUDENT *current_stu;
char *note[10];
char info[100];

int get_id(STUDENT *stu)
{
    return stu->id;
}

char *get_name(STUDENT *stu)
{
    return stu->name;
}

void menu()
{
    puts("-------------------------");
    puts(" 1. Add student          ");
    puts(" 2. Show a student       ");
    puts(" 3. Add a note           ");
    puts(" 4. Show a note          ");
    puts(" 5. Delete a note        ");
    puts(" 6. exit                 ");
    puts("-------------------------");
    printf("Your choice :");
}

void addstu()
{
    STUDENT *stu; // ST1C_4@1

    stu = (STUDENT *)malloc(0x14u);
    printf("ID:");
    scanf("%d", &stu->id);
    printf("Name:");
    scanf("%7s", stu->name);
    stu->fn_get_name = get_name;
    stu->fn_get_id = get_id;
    current_stu = stu;
}

void showstu()
{
    int stu_id; // eax@2
    int stu_name; // eax@2

    if ( current_stu )
    {
        stu_id = current_stu->fn_get_id(current_stu);
        printf("ID:%d\n", stu_id);
        stu_name = current_stu->fn_get_name(current_stu);
        printf("Name:%s\n", stu_name);
    }
}

void addnote()
{
    size_t size; // [sp+18h] [bp-10h]@5
    int i; // [sp+1Ch] [bp-Ch]@1

    for ( i = 0; i <= 9 && note[i]; ++i )
        ;
    printf("Size:");
    scanf("%d", &size);
    note[i] = (char *)malloc(size);
    printf("Content:");
    scanf("%s", note[i]);
}

void shownote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    if ( index >= 0 && index <= 10 )
    {
        if ( note[index] )
            printf("Content:%s\n", note[index]);
        else
            puts("No such note");
    }
}

void delnote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    free(note[index]);
}

int main(int argc, const char **argv, const char **envp)
{
    char buff[20]; // [sp+18h] [bp-18h]@2

    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    printf("Welcome, please leave some message for me:");
    read(0, info, 100u);

    while ( 1 )
    {
        menu();
        read(0, buff, 20u);
        switch ( atoi(buff) )
        {
            case 1:
                addstu();
                break;
            case 2:
                showstu();
                break;
            case 3:
                addnote();
                break;
            case 4:
                shownote();
                break;
            case 5:
                delnote();
                break;
            case 6:
                puts("exit");
                return 0;
            default:
                puts("Invalid choice");
                break;
        }
    }
}
STUDENT *current_stu;
char *note[10];
char info[100];

int get_id(STUDENT *stu)
{
    return stu->id;
}

char *get_name(STUDENT *stu)
{
    return stu->name;
}

void menu()
{
    puts("-------------------------");
    puts(" 1. Add student          ");
    puts(" 2. Show a student       ");
    puts(" 3. Add a note           ");
    puts(" 4. Show a note          ");
    puts(" 5. Delete a note        ");
    puts(" 6. exit                 ");
    puts("-------------------------");
    printf("Your choice :");
}

void addstu()
{
    STUDENT *stu; // ST1C_4@1

    stu = (STUDENT *)malloc(0x14u);
    printf("ID:");
    scanf("%d", &stu->id);
    printf("Name:");
    scanf("%7s", stu->name);
    stu->fn_get_name = get_name;
    stu->fn_get_id = get_id;
    current_stu = stu;
}

void showstu()
{
    int stu_id; // eax@2
    int stu_name; // eax@2

    if ( current_stu )
    {
        stu_id = current_stu->fn_get_id(current_stu);
        printf("ID:%d\n", stu_id);
        stu_name = current_stu->fn_get_name(current_stu);
        printf("Name:%s\n", stu_name);
    }
}

void addnote()
{
    size_t size; // [sp+18h] [bp-10h]@5
    int i; // [sp+1Ch] [bp-Ch]@1

    for ( i = 0; i <= 9 && note[i]; ++i )
        ;
    printf("Size:");
    scanf("%d", &size);
    note[i] = (char *)malloc(size);
    printf("Content:");
    scanf("%s", note[i]);
}

void shownote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    if ( index >= 0 && index <= 10 )
    {
        if ( note[index] )
            printf("Content:%s\n", note[index]);
        else
            puts("No such note");
    }
}

void delnote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    free(note[index]);
}

int main(int argc, const char **argv, const char **envp)
{
    char buff[20]; // [sp+18h] [bp-18h]@2

    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    printf("Welcome, please leave some message for me:");
    read(0, info, 100u);

    while ( 1 )
    {
        menu();
        read(0, buff, 20u);
        switch ( atoi(buff) )
        {
            case 1:
                addstu();
                break;
            case 2:
                showstu();
                break;
            case 3:
                addnote();
                break;
            case 4:
                shownote();
                break;
            case 5:
                delnote();
                break;
            case 6:
                puts("exit");
                return 0;
            default:
                puts("Invalid choice");
                break;
        }
    }
}
STUDENT *current_stu;
char *note[10];
char info[100];

int get_id(STUDENT *stu)
{
    return stu->id;
}

char *get_name(STUDENT *stu)
{
    return stu->name;
}

void menu()
{
    puts("-------------------------");
    puts(" 1. Add student          ");
    puts(" 2. Show a student       ");
    puts(" 3. Add a note           ");
    puts(" 4. Show a note          ");
    puts(" 5. Delete a note        ");
    puts(" 6. exit                 ");
    puts("-------------------------");
    printf("Your choice :");
}

void addstu()
{
    STUDENT *stu; // ST1C_4@1

    stu = (STUDENT *)malloc(0x14u);
    printf("ID:");
    scanf("%d", &stu->id);
    printf("Name:");
    scanf("%7s", stu->name);
    stu->fn_get_name = get_name;
    stu->fn_get_id = get_id;
    current_stu = stu;
}

void showstu()
{
    int stu_id; // eax@2
    int stu_name; // eax@2

    if ( current_stu )
    {
        stu_id = current_stu->fn_get_id(current_stu);
        printf("ID:%d\n", stu_id);
        stu_name = current_stu->fn_get_name(current_stu);
        printf("Name:%s\n", stu_name);
    }
}

void addnote()
{
    size_t size; // [sp+18h] [bp-10h]@5
    int i; // [sp+1Ch] [bp-Ch]@1

    for ( i = 0; i <= 9 && note[i]; ++i )
        ;
    printf("Size:");
    scanf("%d", &size);
    note[i] = (char *)malloc(size);
    printf("Content:");
    scanf("%s", note[i]);
}

void shownote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    if ( index >= 0 && index <= 10 )
    {
        if ( note[index] )
            printf("Content:%s\n", note[index]);
        else
            puts("No such note");
    }
}

void delnote()
{
    int index; // [sp+1Ch] [bp-Ch]@1

    printf("index:");
    scanf("%d", &index);
    free(note[index]);
}

int main(int argc, const char **argv, const char **envp)
{
    char buff[20]; // [sp+18h] [bp-18h]@2

    setvbuf(stdin, 0, 2, 0);
    setvbuf(stdout, 0, 2, 0);
    printf("Welcome, please leave some message for me:");
    read(0, info, 100u);

    while ( 1 )
    {
        menu();
        read(0, buff, 20u);
        switch ( atoi(buff) )
        {
            case 1:
                addstu();
                break;
            case 2:
                showstu();
                break;
            case 3:
                addnote();
                break;
            case 4:
                shownote();
                break;
            case 5:
                delnote();
                break;
            case 6:
                puts("exit");
                return 0;
            default:
                puts("Invalid choice");
                break;
        }
    }
}
```

`addnote()` 會 heap overflow，除此之外還有 double free 的問題存在，我們的目標是這樣：

```
[ note data ] [ student strucutre ]
```

然後讓 `note data` overflow 蓋到 `student structure` 上面的 function pointer，就可以跳 shellcode

過程如下：

```
1. addnote(32), addnote(20)
[ (chunk-meta) note0 (32b) ] [ (chunk-meta) note1   (20b) ]
2. delnote(0), delnote(1)
[ fastbin            (32b) ] [ fastbin              (20b) ]
3. addstu()
[ fastbin            (32b) ] [ (chunk-meta) student (20b) ]
4. addnote(32)
[ (chunk-meta) note0 (32b) ] [ (chunk-meta) student (20b) ]
overflooooooooooooooooow
[ (chunk-meta) note0000000000000000000000000deadc0de00000 ]
```

exploit: [remote3.py](remote3.py)
