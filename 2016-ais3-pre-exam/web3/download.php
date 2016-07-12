<?php
include "waf.php";

if (!isset($_GET['p']))
    die("missing parameters");

$p = $_GET['p'];


// contain at most 1 of "..".
// You are not allowed to go outside root directory. If you can bypass, tell admin!!
$b = substr(strstr($p, ".."), 2);
if (strstr($b, "../"))
    die("Too many ../");

$p = "resource/" . $p;
if (!file_exists($p))
    die("file not found");

header('Content-Type: image');

echo file_get_contents($p);
?>
