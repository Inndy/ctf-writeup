# crypto3

給的東西是一個 `flag.enc` 和很多的 public key

如果手上有很多的 RSA public key modulus，就有機會用找公因數的方法把 modulus 分解

想法是：把所有 public key 的 modulus 通通拿出來做 gcd，結果所有的 key 都被炸開了

接下來拿所有的 key 嘗試去解密 flag，答案就出來了

`ais3{Euc1id3an_a1g0ri7hm_i5_u53fu1}`
