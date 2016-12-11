# anti-debugging

- category: binary
- points: 100

Since code was not protected, we just analyze it statically.

Here's a impossible condition (0x401652) which is hidden from decompiler:

Just patch it and make it jump, then you will get flag.

![Figure 1 -- dead condition](1.png)

And these code decode the flag:

![Figure 2 -- strange string](2.png)
