#include "benchmarks.h"
#include "bignumarifmetic.h"
#include <cstdio>
#include <chrono>
#include <random>
#include <string>
#include <intrin.h>

static std::mt19937 rng(20261012);

static BigNum RandomBigNum(int bits){
    BigNum x = {};
    for (int i = 0; i < WORDS && i * W < bits; i++) x.d[i] = rng();
    if (bits % W != 0) x.d[bits / W] &= (1u << (bits % W)) - 1;
    return x;
}

const int POOL = 1024;
const int P = POOL - 1;
const int D = WORDS - 1;
static BigNum A[POOL], B[POOL], Half[POOL], Small[POOL], E64[POOL];
static BigNum A128[POOL], B128[POOL], A1024[POOL], B1024[POOL];
static int K[POOL];
static std::string HexStr[POOL];

static volatile uint32_t sink;

template <class F>
static void Bench(const char* name, int iterations, F op){
    for (int i = 0; i < iterations / 10 + 1; i++) op(i);
    auto start = std::chrono::steady_clock::now();
    uint64_t c0 = __rdtsc();
    for (int i = 0; i < iterations; i++) op(i);
    uint64_t c1 = __rdtsc();
    auto end = std::chrono::steady_clock::now();
    double ns = std::chrono::duration<double, std::nano>(end - start).count() / iterations;
    printf("| %-30s | %10d | %12.1f | %12.0f |\n", name, iterations, ns, (double)(c1 - c0) / iterations);
}

void runBenchmarks(){
    printf("--- Running Benchmarks (%d bit, %d random numbers per operand) ---\n\n", BITS, POOL);

    for (int i = 0; i < POOL; i++){
        A[i] = RandomBigNum(BITS);
        B[i] = RandomBigNum(BITS);
        Half[i] = RandomBigNum(BITS / 2);  Half[i].d[WORDS / 2 - 1] |= 1u << 31;
        Small[i] = RandomBigNum(W);        Small[i].d[0] |= 1;
        E64[i] = RandomBigNum(64);
        K[i] = rng() % BITS;
        HexStr[i] = ToHex(A[i]);
        A128[i] = RandomBigNum(128);       B128[i] = RandomBigNum(128);
        A1024[i] = RandomBigNum(1024);     B1024[i] = RandomBigNum(1024);
    }

    printf("| %-30s | %10s | %12s | %12s |\n", "Operation", "Iterations", "ns / op", "cycles / op");
    printf("|--------------------------------|------------|--------------|--------------|\n");

    Bench("LongAdd", 1000000, [](int i){
        BigNum c; LongAdd(A[i & P], B[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("LongSub", 1000000, [](int i){
        BigNum c; LongSub(A[i & P], B[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("LongCmp", 1000000, [](int i){
        sink = sink ^ (uint32_t)LongCmp(A[i & P], B[i & P]); });
    Bench("BitLength", 1000000, [](int i){
        sink = sink ^ (uint32_t)BitLength(A[i & P]); });
    Bench("LongMulOneDigit", 1000000, [](int i){
        BigNum c; LongMulOneDigit(A[i & P], B[i & P].d[0], c); sink = sink ^ c.d[i & D]; });
    Bench("LongShiftBitsToHigh", 1000000, [](int i){
        sink = sink ^ LongShiftBitsToHigh(A[i & P], K[i & P]).d[i & D]; });
    Bench("LongShiftBitsToLow", 1000000, [](int i){
        sink = sink ^ LongShiftBitsToLow(A[i & P], K[i & P]).d[i & D]; });
    Bench("LongMul (128 bit)", 200000, [](int i){
        BigNum c; LongMul(A128[i & P], B128[i & P], c); sink = sink ^ c.d[i & 7]; });
    Bench("LongMul (1024 bit)", 50000, [](int i){
        BigNum c; LongMul(A1024[i & P], B1024[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("LongMul (2048 bit)", 50000, [](int i){
        BigNum c; LongMul(A[i & P], B[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("LongSquare (128 bit)", 1000000, [](int i){
        BigNum c; LongSquare(A128[i & P], c); sink = sink ^ c.d[i & 7]; });
    Bench("LongSquare (1024 bit)", 200000, [](int i){
        BigNum c; LongSquare(A1024[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("LongSquare (2048 bit)", 200000, [](int i){
        BigNum c; LongSquare(A[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("LongDivMod (2048 / 1024 bit)", 2000, [](int i){
        BigNum q, r; LongDivMod(A[i & P], Half[i & P], q, r); sink = sink ^ q.d[i & 31] ^ r.d[i & 31]; });
    Bench("LongDivMod (2048 / 32 bit)", 1000, [](int i){
        BigNum q, r; LongDivMod(A[i & P], Small[i & P], q, r); sink = sink ^ q.d[i & D] ^ r.d[0]; });
    Bench("LongPower (exponent 64 bit)", 1000, [](int i){
        BigNum c; LongPower(A[i & P], E64[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("LongPower (exponent 2048 bit)", 30, [](int i){
        BigNum c; LongPower(A[i & P], B[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench("ToHex", 200000, [](int i){
        std::string s = ToHex(A[i & P]); sink = sink ^ (uint32_t)s[i % s.size()]; });
    Bench("FromHex", 200000, [](int i){
        BigNum c; FromHex(HexStr[i & P], c); sink = sink ^ c.d[i & D]; });

    printf("\ncycles = TSC (__rdtsc), counts at the nominal CPU frequency\n");
    printf("------------------------------\n\n");
}
