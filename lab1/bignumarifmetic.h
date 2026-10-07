#pragma once
#include <cstdint>

constexpr int W = 32;
constexpr int BITS = 2048;
constexpr int WORDS = BITS / W; //w=32
constexpr uint64_t MASK = (1ULL << W) -1;

struct BigNum {
    uint32_t d[WORDS];
};

uint32_t LongAdd(const BigNum& a, const BigNum& b, BigNum& c);
