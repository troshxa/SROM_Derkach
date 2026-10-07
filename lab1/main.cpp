#include <cstdio>
#include "bignumarifmetic.h"


int main(){
    BigNum a, b, c;

    a = {}; b = {}; c = {};
    a.d[0] = 0xFFFFFFFF;; b.d[0] = 7; a.d[1] = 4; b.d[1] = 2;;

    uint32_t carry = LongAdd(a, b, c);

    printf("c.d[0] = %u\n", c.d[0]);
    printf("c.d[1] = %u\n", c.d[1]);
    printf("carry  = %u\n", carry);
}
