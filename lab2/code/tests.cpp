#include "tests.h"
#include "bignumarifmetic.h"
#include <cstdio>
#include <random>
#include <string>

static std::mt19937 rng(20261009);

static BigNum RandomBigNum(int maxBits = BITS){
    int bits = 1 + rng() % maxBits;
    BigNum x = {};
    for (int i = 0; i < WORDS && i * W < bits; i++) x.d[i] = rng();
    if (bits % W != 0) x.d[bits / W] &= (1u << (bits % W)) - 1;
    return x;
}

static BigNum Add(const BigNum& a, const BigNum& b){ BigNum c; LongAdd(a, b, c); return c; }
static BigNum Sub(const BigNum& a, const BigNum& b){ BigNum c; LongSub(a, b, c); return c; }
static BigNum Mul(const BigNum& a, const BigNum& b){ BigNum c; LongMul(a, b, c); return c; }
static BigNum Sqr(const BigNum& a){ BigNum c; LongSquare(a, c); return c; }
static BigNum Pow(const BigNum& a, const BigNum& b){ BigNum c; LongPower(a, b, c); return c; }
static BigNum Hex(const std::string& s){ BigNum a; FromHex(s, a); return a; }
static bool   Eq(const BigNum& a, const BigNum& b){ return LongCmp(a, b) == 0; }

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
    printf("--- Running Tests (%d bit, all operations mod 2^%d) ---\n\n", BITS, BITS);
    failed = 0;


    BigNum a = Hex("123456789ABCDEF0FEDCBA9876543210");
    BigNum b = Hex("0F1E2D3C4B5A69788796A5B4C3D2E1F0");
    BigNum c = Hex("1234567890ABCDEF");
    Show("a", a); Show("b", b); Show("c", c);
    printf("\n");

    Show("(a+b)*c  ", Mul(Add(a, b), c));
    Show("a*c + b*c", Add(Mul(a, c), Mul(b, c)));
    Report("Test 1 [(a+b)*c == c*(a+b) == a*c + b*c]",
           ForRandom(RANDOM, []{
               BigNum x = RandomBigNum(), y = RandomBigNum(), z = RandomBigNum();
               BigNum l = Mul(Add(x, y), z);
               return Eq(l, Mul(z, Add(x, y))) && Eq(l, Add(Mul(x, z), Mul(y, z)));
           }));

    BigNum sum = {};
    for (int k = 0; k < 100; k++) sum = Add(sum, a);
    BigNum na; LongMulOneDigit(a, 100, na);
    Show("100*a      ", na);
    Show("a + ... + a", sum);
    Report("Test 2 [n*a == a+a+...+a, n = 100..1000]",
           Eq(na, sum) && ForRandom(RANDOM, []{
               BigNum x = RandomBigNum(), s = {}, m;
               uint32_t n = 100 + rng() % 901;
               for (uint32_t k = 0; k < n; k++) s = Add(s, x);
               LongMulOneDigit(x, n, m);
               return Eq(m, s) && Eq(Mul(ToBigNum(n), x), s);
           }));

    Show("a + b", Add(a, b)); Show("a - b", Sub(a, b));
    BigNum max = Hex(std::string(BITS / 4, 'F')), r;
    uint32_t carry = LongAdd(max, ToBigNum(1), r);
    bool overflow = Eq(r, ToBigNum(0)) && carry == 1;                
    uint32_t borrow = LongSub(ToBigNum(0), ToBigNum(1), r);
    bool underflow = Eq(r, max) && borrow == 1;                     
    Report("Test 3 [a+b == b+a, (a+b)-b == a, a-a == 0, carry/borrow]",
           Eq(Add(a, b), Hex("215283B4E61748698673604D3A271400")) &&
           Eq(Sub(a, b), Hex("316293C4F627578774614E3B2815020")) && overflow && underflow &&
           ForRandom(RANDOM, []{
               BigNum x = RandomBigNum(), y = RandomBigNum();
               return Eq(Add(x, y), Add(y, x)) && Eq(Sub(Add(x, y), y), x) && Eq(Sub(x, x), ToBigNum(0));
           }));

    Show("a * b", Mul(a, b)); Show("a^2  ", Sqr(a));
    Report("Test 4 [a*b == b*a, a^2 == a*a, (a+b)^2 == a^2 + 2ab + b^2]",
           Eq(Mul(a, b), Hex("113366AB0066DE57C0175D92B6C9CBBB8A47F4901A93FD55EF9A562300EFF00")) &&
           Eq(Sqr(a), Hex("14B66DC33F6ACDCCA2148A6A1A009454495D294750DF8CCDEEC6CD7A44A4100")) &&
           Eq(Sqr(ToBigNum(0xFFFFFFFF)), Hex("FFFFFFFE00000001")) &&
           ForRandom(RANDOM, []{
               BigNum x = RandomBigNum(), y = RandomBigNum();
               BigNum two_xy = LongShiftBitsToHigh(Mul(x, y), 1);
               return Eq(Mul(x, y), Mul(y, x)) && Eq(Sqr(x), Mul(x, x)) &&
                      Eq(Sqr(Add(x, y)), Add(Add(Sqr(x), two_xy), Sqr(y)));
           }));

    BigNum a2 = Add(Sqr(a), ToBigNum(12345)), q;
    LongDivMod(a2, b, q, r);
    Show("a^2 + 12345", a2); Show("q = (a^2 + 12345) / b", q); Show("r = (a^2 + 12345) % b", r);
    Report("Test 5 [a == b*q + r, 0 <= r < b, div by 0 -> error]",
           Eq(q, Hex("15EBD2CAD3EE192EC2C99D3DAAE4FCFD")) && Eq(r, Hex("61F7E44949059EAB1A44063ECB8E709")) &&
           !LongDivMod(a, ToBigNum(0), q, r) &&
           ForRandom(RANDOM, []{
               BigNum x = RandomBigNum(), y, qq, rr;
               do y = RandomBigNum(); while (BitLength(y) == 0);
               return LongDivMod(x, y, qq, rr) && Eq(Add(Mul(y, qq), rr), x) && LongCmp(rr, y) < 0;
           }));

    Show("3^200", Pow(ToBigNum(3), ToBigNum(200)));
    Report("Test 6 [a^(m+n) == a^m * a^n, a^(m*n) == (a^m)^n, a^0 == 1, a^1 == a]",
           Eq(Pow(ToBigNum(3), ToBigNum(200)),
              Hex("1FD5863C3EB0469EC21A937A76F3432FFD73D97E447606B683ECF6F6E4A7AE225BFAFF1EAAF8B0A1")) &&
           Eq(Pow(a, ToBigNum(0)), ToBigNum(1)) && Eq(Pow(a, ToBigNum(1)), a) &&
           Eq(Pow(ToBigNum(2), ToBigNum(BITS)), ToBigNum(0)) &&               
           ForRandom(100, []{
               BigNum x = RandomBigNum();
               uint32_t m = rng() % 100000, n = rng() % 100000;
               BigNum xm = Pow(x, ToBigNum(m));
               return Eq(Pow(x, ToBigNum((uint64_t)m + n)), Mul(xm, Pow(x, ToBigNum(n)))) &&
                      Eq(Pow(x, ToBigNum((uint64_t)m * n)), Pow(xm, ToBigNum(n)));
           }));

    Show("a << 100", LongShiftBitsToHigh(a, 100)); Show("a >> 8  ", LongShiftBitsToLow(a, 8));
    Report("Test 7 [a << k == a*2^k, a >> k == a / 2^k, BitLength(2^k) == k+1]",
           BitLength(ToBigNum(0)) == 0 && BitLength(ToBigNum(1)) == 1 && BitLength(a) == 125 &&
           ForRandom(300, []{
               int k = rng() % BITS;
               BigNum x = RandomBigNum(), p = Pow(ToBigNum(2), ToBigNum(k)), qq, rr;
               LongDivMod(x, p, qq, rr);
               return Eq(LongShiftBitsToHigh(x, k), Mul(x, p)) && Eq(LongShiftBitsToLow(x, k), qq) &&
                      BitLength(p) == k + 1;
           }));

    printf("  ToBin(c) = %s\n", ToBin(c).c_str());
    BigNum t;
    Report("Test 8 [FromHex(ToHex(a)) == a, FromBin(ToBin(a)) == a, bad strings rejected]",
           FromHex("0x1f", t) && Eq(t, ToBigNum(31)) && ToHex(ToBigNum(0)) == "0" &&
           FromBin("0b101", t) && Eq(t, ToBigNum(5)) &&
           !FromHex("12G4", t) && !FromBin("102", t) && !FromHex("", t) &&
           !FromHex("1" + std::string(BITS / 4, '0'), t) &&                 
           ForRandom(RANDOM, []{
               BigNum x = RandomBigNum(), y;
               return FromHex(ToHex(x), y) && Eq(x, y) && FromBin(ToBin(x), y) && Eq(x, y);
           }));

    printf("\n%s (failed: %d)\n------------------------------\n\n",
           failed ? "SOME TESTS FAILED" : "All tests passed", failed);
    return failed;
}
