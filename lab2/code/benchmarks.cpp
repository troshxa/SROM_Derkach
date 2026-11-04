#include "benchmarks.h"
#include "bignumarifmetic.h"
#include <cstdio>
#include <chrono>
#include <random>
#include <string>
#include <intrin.h>

static std::mt19937 rng(20261104);

static BigNum RandomBigNum(int bits){
    BigNum x = {};
    for (int i = 0; i < WORDS && i * W < bits; i++) x.d[i] = rng();
    if (bits % W != 0) x.d[bits / W] &= (1u << (bits % W)) - 1;
    return x;
}

static BigNum RandomExact(int bits){
    BigNum x = RandomBigNum(bits);
    x.d[(bits - 1) / W] |= 1u << ((bits - 1) % W);
    return x;
}

static BigNum Rem(const BigNum& a, const BigNum& n){ BigNum q, r; LongDivMod(a, n, q, r); return r; }

const int POOL = 256;
const int P = POOL - 1;
const int D = WORDS - 1;

struct ModSet {
    int bits;
    BigNum n[POOL], mu[POOL];
    BigNum a[POOL], b[POOL];          
    BigNum x[POOL];                   
    BigNum g1[POOL], g2[POOL];    

    void Init(int bitsN){
        bits = bitsN;
        for (int i = 0; i < POOL; i++){
            n[i] = RandomExact(bits);
            mu[i] = BarrettMu(n[i]);
            a[i] = Rem(RandomBigNum(bits), n[i]);
            b[i] = Rem(RandomBigNum(bits), n[i]);
            LongMul(a[i], b[i], x[i]);
            g1[i] = RandomExact(bits);
            g2[i] = RandomExact(bits);
        }
    }
};

static ModSet S256, S512, S992;

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
    printf("| %-36s | %10d | %14.1f | %14.0f |\n", name, iterations, ns, (double)(c1 - c0) / iterations);
}

static void Header(const char* title){
    printf("\n%s\n", title);
    printf("| %-36s | %10s | %14s | %14s |\n", "Operation", "Iterations", "ns / op", "cycles / op");
    printf("|--------------------------------------|------------|----------------|----------------|\n");
}

static void BenchSet(ModSet& S, int scale){
    char name[64];
    #define NAME(op) (snprintf(name, sizeof name, "%s (%d bit)", op, S.bits), name)

    Bench(NAME("LongGcd"), 2000 * scale, [&](int i){
        sink = sink ^ LongGcd(S.g1[i & P], S.g2[i & P]).d[0]; });
    Bench(NAME("LongLcm"), 300 * scale, [&](int i){
        sink = sink ^ LongLcm(S.g1[i & P], S.g2[i & P]).d[i & D]; });
    Bench(NAME("LongAddMod"), 1000000, [&](int i){
        BigNum c; LongAddMod(S.a[i & P], S.b[i & P], S.n[i & P], c); sink = sink ^ c.d[0]; });
    Bench(NAME("LongSubMod"), 1000000, [&](int i){
        BigNum c; LongSubMod(S.a[i & P], S.b[i & P], S.n[i & P], c); sink = sink ^ c.d[0]; });
    Bench(NAME("BarrettMu"), 300 * scale, [&](int i){
        sink = sink ^ BarrettMu(S.n[i & P]).d[0]; });
    Bench(NAME("BarrettReduction"), 50000, [&](int i){
        sink = sink ^ BarrettReduction(S.x[i & P], S.n[i & P], S.mu[i & P]).d[0]; });
    Bench(NAME("x mod n via LongDivMod"), 300 * scale, [&](int i){
        sink = sink ^ Rem(S.x[i & P], S.n[i & P]).d[0]; });
    Bench(NAME("LongMulMod"), 50000, [&](int i){
        BigNum c; LongMulMod(S.a[i & P], S.b[i & P], S.n[i & P], S.mu[i & P], c); sink = sink ^ c.d[0]; });
    Bench(NAME("LongPowerMod"), 20 * scale, [&](int i){
        BigNum c; LongPowerMod(S.a[i & P], S.b[i & P], S.n[i & P], S.mu[i & P], c); sink = sink ^ c.d[0]; });

    #undef NAME
}

static void BenchSquare(ModSet& S){
    char name[64];
    #define NAME(op) (snprintf(name, sizeof name, "%s (%d bit)", op, S.bits), name)

    Bench(NAME("LongMul(a, a)"), 100000, [&](int i){
        BigNum c; LongMul(S.a[i & P], S.a[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench(NAME("LongSquare(a)"), 100000, [&](int i){
        BigNum c; LongSquare(S.a[i & P], c); sink = sink ^ c.d[i & D]; });
    Bench(NAME("LongMulMod(a, a)"), 50000, [&](int i){
        BigNum c; LongMulMod(S.a[i & P], S.a[i & P], S.n[i & P], S.mu[i & P], c); sink = sink ^ c.d[0]; });
    Bench(NAME("LongSquareMod(a)"), 50000, [&](int i){
        BigNum c; LongSquareMod(S.a[i & P], S.n[i & P], S.mu[i & P], c); sink = sink ^ c.d[0]; });

    #undef NAME
}

void runBenchmarks(){
    printf("--- Running Benchmarks (%d bit numbers, %d random moduli per size) ---\n", BITS, POOL);

    S256.Init(256);
    S512.Init(512);
    S992.Init((WORDS / 2 - 1) * W);  

    Header("Modular arithmetic");
    BenchSet(S256, 8);
    BenchSet(S512, 3);
    BenchSet(S992, 1);

    Header("Variant 1: squaring vs multiplication");
    BenchSquare(S256);
    BenchSquare(S512);
    BenchSquare(S992);

    printf("\nLongPowerMod: exponent has the same bit length as the modulus\n");
    printf("cycles = TSC (__rdtsc), counts at the nominal CPU frequency\n");
    printf("------------------------------\n\n");
}
