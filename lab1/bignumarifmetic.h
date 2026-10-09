#pragma once
#include <cstdint>
#include <string>

constexpr int W = 32;
constexpr int BITS = 2048;
constexpr int WORDS = BITS / W; //w=32
constexpr uint64_t BASE = 1ULL << W;
constexpr uint64_t MASK = BASE -1;

struct BigNum {
    uint32_t d[WORDS];
};

uint32_t LongAdd(const BigNum& a, const BigNum& b, BigNum& c);
uint32_t LongSub(const BigNum& a, const BigNum& b, BigNum& c);
int LongCmp(const BigNum& a, const BigNum& b);
BigNum ToBigNum(uint64_t x);
uint32_t LongMulOneDigit(const BigNum& a, uint32_t b, BigNum& c);
BigNum LongShiftDigitsToHigh(const BigNum& a, int x);
BigNum LongShiftDigitsToLow(const BigNum& a, int x);
BigNum LongShiftBitsToHigh(const BigNum& a, int x);
BigNum LongShiftBitsToLow(const BigNum& a, int x);
void LongMul(const BigNum& a, const BigNum& b, BigNum& c);
void LongSquare(const BigNum& a, BigNum& b);
int BitLength(const BigNum& a);
bool LongDivMod(const BigNum& a, const BigNum& b, BigNum& q, BigNum& r);
void LongPower(const BigNum& a, const BigNum& b, BigNum& c);
bool FromHex(const std::string& s, BigNum& a);   
std::string ToHex(const BigNum& a);
bool FromBin(const std::string& s, BigNum& a);  
std::string ToBin(const BigNum& a);

