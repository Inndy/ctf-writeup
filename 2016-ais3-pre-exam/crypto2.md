# crypto2

Source Code:

``` php
<?php
    $path = "..";
    $file = "flag.txt";
    $authenticated = false;
    $secret = trim(file_get_contents("$path/$file"));
    assert(strlen($secret) <= 60);

    if(isset($_GET['expire']) && isset($_GET['auth'])) {
        $expire = $_GET['expire'];
        $auth = $_GET['auth'];
        $qstr = substr(strstr($_SERVER['REQUEST_URI'], '?'), 1);
        $qstr = preg_replace('/&auth=.*/', '', $qstr);
        $qstr = urldecode($qstr);
        if(sha1($secret . "$qstr") === $auth) {
            if($expire > time(0)) {
                $authenticated = true;
            }
        }
    } else {
        $expire = time(0) - 1000000;
        $auth = sha1($secret . "expire=$expire");
        $uri = preg_replace('/\?.*/', '', $_SERVER['REQUEST_URI']);
        header("HTTP/1.1 302 Found");
        header("Location: $uri?expire=$expire&auth=$auth");
        die;
    }
?>
```

使用 `length extension attack` 在 `expire=$TIME` 後面串上 `&expire=$NEW_TIME` 進行攻擊

使用的工具是 [hashpump](https://github.com/bwall/HashPump)

Solution:

``` python
#!/usr/bin/env python2
import hashpumpy
import requests
import time

def escape(x):
    return ''.join(i if 0x20 <= ord(i) < 0x7f else '%%%.2x' % ord(i) for i in x)

time = int(time.time()) + 1000

for i in range(16, 60):
    data, hash = 'expire=1467292770', 'b4773bef88453d149457af5ba469471b052eb69f'
    hash, data = hashpumpy.hashpump(hash, data, '&expire=%d' % time, i)
    data = escape(data)
    response = requests.get('https://quiz.ais3.org:8014/?%s&auth=%s' % (data, hash))
    print('trying length = %d' % i)
    if 'Unauthenticated. Expired?' not in response.text:
        print(response.text)
        break
```

`ais3{HasH.eXtension.@tt@ck!}`
