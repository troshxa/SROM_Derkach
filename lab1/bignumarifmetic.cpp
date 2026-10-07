#include "bignumarifmetic.h"

uint32_t LongAdd(const BigNum& a, const BigNum& b, BigNum& c){
    uint32_t carry = 0;
    uint64_t temp = 0;
    for (int i = 0; i < WORDS; i++){
        temp = (uint64_t)a.d[i] +b.d[i] + carry;
        c.d[i] = temp & MASK;
        carry = temp >> W;
    }
    return carry;
};
