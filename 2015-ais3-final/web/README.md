## Web 1

- race condition ([Reference](https://github.com/ctfs/write-ups-2015/tree/master/ghost-in-the-shellcode-2015/web/aart))

## Web 2

- truncate by `substr`

```
payload   -> "A" * 15 + "'"
escaped   -> "AAAAAAAAAAAAAAA\'"
truncated -> "AAAAAAAAAAAAAAA\"
sprintf   -> "VALUES('AAAAAAAAAAAAAAA\' ...... )"
```

- `0x737472696e67` style string for MySQL
- put payload in `$_SESSION` and include session file
