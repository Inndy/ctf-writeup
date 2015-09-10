#include <stdio.h>
#include <stdint.h>

uint32_t i = 1;
uint32_t r = 1;
uint32_t rr = 1;
uint32_t ni = 4;
uint32_t shit = 0;

int main()
{
    // 2757 * r == 3542 * (input_number - r * r) + 1337
    while ( i <= 2147483647u )
    {
        if ( i == ni )
        {
            do {
                r++;
                // next r
            } while((2757 * r - 1337) % 3542 != 0);

            rr = r * r;
            i = rr;
            ni = (r + 1) * (r + 1);
            shit = 2757 * r - 1337;
            printf("[*] next_r, i = %u, r = %u, rr = %u\n", i, r, rr);
        }
        if ( shit == 3542 * (i - rr) )
            printf("[+] Possible solution -> %d\n", i);
        i++;
    }
    return 0;
}
