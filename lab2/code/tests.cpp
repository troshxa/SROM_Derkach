#include "tests.h"
#include "bignumarifmetic.h"
#include <cstdio>
#include <random>
#include <string>

static std::mt19937 rng(20261104);

// модуль для Barrett: k <= 31 цифри, тобто не більше 992 біт (див. BarrettMu)
const int MAX_MOD_BITS = (WORDS / 2 - 1) * W;

static BigNum RandomBigNum(int maxBits = BITS){
    int bits = 1 + rng() % maxBits;
    BigNum x = {};
    for (int i = 0; i < WORDS && i * W < bits; i++) x.d[i] = rng();
    if (bits % W != 0) x.d[bits / W] &= (1u << (bits % W)) - 1;
    return x;
}

// рівно bits біт (старший біт = 1)
static BigNum RandomExact(int bits){
    BigNum x = {};
    for (int i = 0; i < WORDS && i * W < bits; i++) x.d[i] = rng();
    if (bits % W != 0) x.d[bits / W] &= (1u << (bits % W)) - 1;
    x.d[(bits - 1) / W] |= 1u << ((bits - 1) % W);
    return x;
}

struct Modulus { BigNum n, mu; };

static Modulus MakeModulus(const BigNum& n){ return { n, BarrettMu(n) }; }
static Modulus RandomModulus(){ return MakeModulus(RandomExact(2 + rng() % (MAX_MOD_BITS - 1))); }

static BigNum Add(const BigNum& a, const BigNum& b){ BigNum c; LongAdd(a, b, c); return c; }
static BigNum Sub(const BigNum& a, const BigNum& b){ BigNum c; LongSub(a, b, c); return c; }
static BigNum Mul(const BigNum& a, const BigNum& b){ BigNum c; LongMul(a, b, c); return c; }
static BigNum Rem(const BigNum& a, const BigNum& n){ BigNum q, r; LongDivMod(a, n, q, r); return r; }
static BigNum Quot(const BigNum& a, const BigNum& n){ BigNum q; LongDiv(a, n, q); return q; }
static BigNum Hex(const std::string& s){ BigNum a; FromHex(s, a); return a; }
static bool   Eq(const BigNum& a, const BigNum& b){ return LongCmp(a, b) == 0; }
static bool   IsZero(const BigNum& a){ return BitLength(a) == 0; }

static BigNum AddM(const BigNum& a, const BigNum& b, const Modulus& m){ BigNum c; LongAddMod(a, b, m.n, c); return c; }
static BigNum SubM(const BigNum& a, const BigNum& b, const Modulus& m){ BigNum c; LongSubMod(a, b, m.n, c); return c; }
static BigNum MulM(const BigNum& a, const BigNum& b, const Modulus& m){ BigNum c; LongMulMod(a, b, m.n, m.mu, c); return c; }
static BigNum SqrM(const BigNum& a, const Modulus& m){ BigNum c; LongSquareMod(a, m.n, m.mu, c); return c; }
static BigNum PowM(const BigNum& a, const BigNum& e, const Modulus& m){ BigNum c; LongPowerMod(a, e, m.n, m.mu, c); return c; }

// випадкове число з [0, n)
static BigNum RandomBelow(const BigNum& n){ return Rem(RandomBigNum(BitLength(n)), n); }

static void Show(const char* name, const BigNum& x){
    printf("  %s = %s\n", name, ToHex(x).c_str());
}

static int failed = 0;

static void Report(const char* name, bool ok){
    printf("%s: %s\n", name, ok ? "Passed!" : "Failed!");
    if (!ok) failed++;
}

template <class F>
static bool ForRandom(int count, F check){
    for (int t = 0; t < count; t++)
        if (!check()) return false;
    return true;
}

const int RANDOM = 1000;

int runTests(){
    printf("--- Running Tests (%d bit numbers, modulus up to %d bit) ---\n\n", BITS, MAX_MOD_BITS);
    failed = 0;
    const Modulus M = MakeModulus(Hex("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED"));
    BigNum a = Hex("6C1A4F0E3B2D59877A13C4E2F05B6D18A9C3E7F2014B5D6E8F90A1B2C3D4E5F6");
    BigNum b = Hex("3F2E1D0C9B8A79685746352413F2E1D0CFBEAD9C8B7A69584736251403F2E1D0");
    BigNum c = Hex("1234567890ABCDEF1234567890ABCDEF");
    Show("n ", M.n); Show("mu", M.mu);
    Show("a ", a); Show("b ", b); Show("c ", c);
    printf("\n");

    BigNum lhs = MulM(AddM(a, b, M), c, M);
    Show("(a+b)*c mod n  ", lhs);
    Show("c*(a+b) mod n  ", MulM(c, AddM(a, b, M), M));
    Show("a*c + b*c mod n", AddM(MulM(a, c, M), MulM(b, c, M), M));
    Report("Test 1 [(a+b)*c == c*(a+b) == a*c + b*c (mod n)]",
           Eq(lhs, Hex("CDCCA09184A1B5B1B90E37039BC68063AC88DFE8BB795659305430D1557A619")) &&
           Eq(lhs, MulM(c, AddM(a, b, M), M)) && Eq(lhs, AddM(MulM(a, c, M), MulM(b, c, M), M)) &&
           ForRandom(RANDOM, []{
               Modulus m = RandomModulus();
               BigNum x = RandomBelow(m.n), y = RandomBelow(m.n), z = RandomBelow(m.n);
               BigNum l = MulM(AddM(x, y, m), z, m);
               return Eq(l, MulM(z, AddM(x, y, m), m)) && Eq(l, AddM(MulM(x, z, m), MulM(y, z, m), m));
           }));

    BigNum sum = {};
    for (int k = 0; k < 100; k++) sum = AddM(sum, a, M);
    BigNum na = MulM(ToBigNum(100), a, M);
    Show("100*a mod n      ", na);
    Show("a + ... + a mod n", sum);
    Report("Test 2 [k*a == a+a+...+a (mod n), k = 100..1000]",
           Eq(na, sum) && Eq(na, Hex("3A46E18F1DB6F8EBAFB8E8A5E3B69DA250869A8881707F30147F29D47F29DA54")) &&
           ForRandom(RANDOM, []{
               Modulus m = RandomModulus();
               BigNum x = RandomBelow(m.n), s = {};
               uint32_t k = 100 + rng() % 901;
               for (uint32_t t = 0; t < k; t++) s = AddM(s, x, m);
               return Eq(MulM(Rem(ToBigNum(k), m.n), x, m), s);
           }));

    BigNum fermat = PowM(a, Sub(M.n, ToBigNum(1)), M);
    Show("a^(n-1) mod n, n = 2^255-19", fermat);
    Report("Test 3 [a^phi(n) == 1 (mod n), n prime: 2^255-19, Mersenne 2^127-1, 2^521-1, 2^607-1]",
           Eq(fermat, ToBigNum(1)) &&
           ForRandom(30, []{
               static const int P[] = { 127, 521, 607 };
               int p = P[rng() % 3];
               Modulus m = MakeModulus(Sub(LongShiftBitsToHigh(ToBigNum(1), p), ToBigNum(1)));
               BigNum x = RandomBelow(m.n);
               if (IsZero(x)) x = ToBigNum(1);
               return Eq(PowM(x, Sub(m.n, ToBigNum(1)), m), ToBigNum(1));
           }));

    BigNum p3; LongPower(ToBigNum(3), ToBigNum(100), p3);
    Modulus M3 = MakeModulus(p3);
    BigNum phi3 = Mul(ToBigNum(2), Quot(p3, ToBigNum(3)));
    BigNum a1 = Rem(Add(a, ToBigNum(1)), p3);           
    BigNum euler3 = PowM(a1, phi3, M3);
    Show("n = 3^100             ", p3);
    Show("(a+1)^phi(n) mod 3^100", euler3);
    Report("Test 4 [a^(2*3^(k-1)) == 1 (mod 3^k), a not divisible by 3]",
           Eq(euler3, ToBigNum(1)) &&
           ForRandom(30, []{
               int k = 1 + rng() % 620;                  // 3^620 < 2^992
               BigNum n3, x; LongPower(ToBigNum(3), ToBigNum(k), n3);
               Modulus m = MakeModulus(n3);
               BigNum phi = Mul(ToBigNum(2), Quot(n3, ToBigNum(3)));
               do x = RandomBelow(n3); while (IsZero(Rem(x, ToBigNum(3))));
               return Eq(PowM(x, phi, m), ToBigNum(1));
           }));
    BigNum x = Mul(Mul(a, b), Rem(c, M.n));
    x = Rem(Mul(x, a), Mul(M.n, M.n));                   // x < n^2
    Show("x               ", x);
    Show("Barrett(x)      ", BarrettReduction(x, M.n, M.mu));
    Show("x mod n (DivMod)", Rem(x, M.n));
    Report("Test 5 [BarrettReduction(x) == x mod n, x < n^2]",
           Eq(BarrettReduction(x, M.n, M.mu), Rem(x, M.n)) &&
           Eq(BarrettReduction(Sub(Mul(M.n, M.n), ToBigNum(1)), M.n, M.mu), Sub(M.n, ToBigNum(1))) &&
           Eq(BarrettReduction(M.n, M.n, M.mu), ToBigNum(0)) &&
           ForRandom(RANDOM, []{
               Modulus m = RandomModulus();
               BigNum y = Rem(RandomBigNum(2 * BitLength(m.n)), Mul(m.n, m.n));
               return Eq(BarrettReduction(y, m.n, m.mu), Rem(y, m.n));
           }));

    BigNum g = LongGcd(a, b), l = LongLcm(a, b);
    Show("gcd(a, b)", g); Show("lcm(a, b)", l);
    Report("Test 6 [a, b divisible by gcd, gcd(a/g, b/g) == 1, gcd * lcm == a * b]",
           Eq(g, ToBigNum(6)) &&
           Eq(l, Hex("47253132B2A40A502D805AE83B8FF272E9D0FE2264B1CE6B9D44CFE9FC3728E360F348133AB5F8E368E37EE8AF11C7687455B4EC0A9B60640C7706C15460250")) &&
           Eq(LongGcd(a, ToBigNum(0)), a) && Eq(LongGcd(ToBigNum(0), a), a) && Eq(LongGcd(a, a), a) &&
           Eq(LongGcd(a, ToBigNum(1)), ToBigNum(1)) && Eq(LongLcm(a, ToBigNum(0)), ToBigNum(0)) &&
           ForRandom(RANDOM, []{
               BigNum common = RandomBigNum(200);
               if (IsZero(common)) common = ToBigNum(1);
               BigNum u = Mul(RandomBigNum(400), common), v = Mul(RandomBigNum(400), common);
               if (IsZero(u) || IsZero(v)) return true;
               BigNum gg = LongGcd(u, v);
               return IsZero(Rem(u, gg)) && IsZero(Rem(v, gg)) && IsZero(Rem(gg, common)) &&
                      Eq(LongGcd(Quot(u, gg), Quot(v, gg)), ToBigNum(1)) &&
                      Eq(Mul(gg, LongLcm(u, v)), Mul(u, v));
           }));

    Show("a + b mod n", AddM(a, b, M));
    Show("a - b mod n", SubM(a, b, M));
    Show("b - a mod n", SubM(b, a, M));
    Report("Test 7 [(a+b)-b == a, (a-b)+b == a, a-a == 0 (mod n), results < n]",
           Eq(AddM(a, b, M), Hex("2B486C1AD6B7D2EFD159FA07044E4EE97982958E8CC5C6C6D6C6C6C6C7C7C7D9")) &&
           Eq(SubM(a, b, M), Hex("2CEC32019FA2E01F22CD8FBEDC688B47DA053A5575D0F416485A7C9EBFE20426")) &&
           Eq(SubM(b, a, M), Hex("5313CDFE605D1FE0DD327041239774B825FAC5AA8A2F0BE9B7A58361401DFBC7")) &&
           ForRandom(RANDOM, []{
               Modulus m = RandomModulus();
               BigNum u = RandomBelow(m.n), v = RandomBelow(m.n);
               BigNum s = AddM(u, v, m), d = SubM(u, v, m);
               return LongCmp(s, m.n) < 0 && LongCmp(d, m.n) < 0 &&
                      Eq(SubM(s, v, m), u) && Eq(AddM(d, v, m), u) && IsZero(SubM(u, u, m)) &&
                      Eq(s, Rem(Add(u, v), m.n));
           }));

    Show("a * b mod n", MulM(a, b, M));
    Show("a^2 mod n  ", SqrM(a, M));
    Show("a^b mod n  ", PowM(a, b, M));
    Report("Test 8 [a*b mod n == (a*b) % n, a^2 == a*a (mod n), a^2 == a*a]",
           Eq(MulM(a, b, M), Hex("3A2D3819A7A5D04BCFBA5F049269E9ACAF8249429EE001A305C1354EC9B61516")) &&
           Eq(SqrM(a, M), Hex("48A031F31C028489B8944CE4F0A3CE196D47FC161F1867E73DEE4227380B1A41")) &&
           ForRandom(RANDOM, []{
               Modulus m = RandomModulus();
               BigNum u = RandomBelow(m.n), v = RandomBelow(m.n), sq;
               LongSquare(u, sq);
               return Eq(MulM(u, v, m), Rem(Mul(u, v), m.n)) && Eq(SqrM(u, m), MulM(u, u, m)) &&
                      Eq(sq, Mul(u, u));
           }));

    Report("Test 9 [a^(e+f) == a^e * a^f, a^(e*f) == (a^e)^f (mod n), a^0 == 1, a^1 == a]",
           Eq(PowM(a, b, M), Hex("41FA9242A62F71FCBD59F3368FFA8400A445268C9A885C66B47D8ECEDBD20AD8")) &&
           Eq(PowM(a, ToBigNum(0), M), ToBigNum(1)) && Eq(PowM(a, ToBigNum(1), M), a) &&
           ForRandom(100, []{
               Modulus m = RandomModulus();
               BigNum u = RandomBelow(m.n), e = RandomBigNum(400), f = RandomBigNum(400), small = RandomBigNum(32);
               BigNum ue = PowM(u, e, m);
               return Eq(PowM(u, Add(e, f), m), MulM(ue, PowM(u, f, m), m)) &&
                      Eq(PowM(u, Mul(e, small), m), PowM(ue, small, m));
           }));

    printf("\n%s (failed: %d)\n------------------------------\n\n",
           failed ? "SOME TESTS FAILED" : "All tests passed", failed);
    return failed;
}
