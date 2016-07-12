<?php

include "you_should_not_pass.php";

# general WAF
function waf()
{
    $keywords = [
        "union",
        "select",
        "insert",
        "where",
        "update",
        "order",
        # danger!!!!
        "flag",
    ];

    $uri = parse_url($_SERVER["REQUEST_URI"]);
    parse_str($uri['query'], $query);
    foreach($keywords as $token)
    {
        foreach($query as $k => $v)
        {
            if (stristr($k, $token))
                bad();
            if (stristr($v, $token))
                bad();
        }
    }
}

waf();
