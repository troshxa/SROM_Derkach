#include "tests.h"
#include "benchmarks.h"
#include "bignumarifmetic.h"
#define NOMINMAX
#include <windows.h>
#include <cstdio>

int main(){
    SetConsoleOutputCP(CP_UTF8);


    BigNum A, B, N;
    FromHex("0xcebe60efd949039fa8d8bac7d3e17db03f3e060531a329396ff9f9ab618600487e6efa1059fa75c882f01ab7d0564b58030dfdf6846db25d7e02dd90b8f9c58350c7f5878858c561e5d2a665f3f51da1f590c1150948b33dade79a8f3868c7849aa74e2c2741705cfae461bc360850a4bf77df2a6a8b0c50bd40fa4a2e73527e", A);
    FromHex("0xd022e8f4fa56820673ae38cc8820c81662b720857e4fb620c7fcc6e3869e9435d24fe2b17f7a0769fcf5088580f8f7388ce84f8151cf034faeb7b9c9c5209c90ff0030191c71640436766c7d4a894774b05d2e9149278fa1c720bbf91d878e94db602352d033997bd73d8e81d5de0d3a3d43f4d0b4ac871c78a132618c1aca9e", B);
    FromHex("0xdcb6124729054a97aee4a7edffc78672de847485fb6db32472c1b47f0741328ce0469fa2602985b97518b05112d9d65ff664f742b16dbb7c9e863514f0003e66d0b4bf8e9607ab378aa30de8dc842d83a63c88ce01ab036763286ec63a60f30b052d78b79761d2b37fafad485eeda1d852ae3f0635840ea8a1bef5d3bd9dd94d", N);

    printf("A = %s\n", ToHex(A).c_str());
    printf("B = %s\n", ToHex(B).c_str());
    printf("N = %s\n\n", ToHex(N).c_str());

    printf("gcd(A, B) = %s\n\n", ToHex(LongGcd(A, B)).c_str());
    printf("lcm(A, B) = %s\n\n", ToHex(LongLcm(A, B)).c_str());

    BigNum q;
    LongDivMod(A, N, q, A);
    LongDivMod(B, N, q, B);

    BigNum mu = BarrettMu(N);
    BigNum sum, diff, prod, pw;
    LongAddMod(A, B, N, sum);
    LongSubMod(A, B, N, diff);          
    LongMulMod(A, B, N, mu, prod);      
    LongPowerMod(A, B, N, mu, pw);     

    printf("A + B mod N = %s\n\n", ToHex(sum).c_str());
    printf("A - B mod N = %s\n\n", ToHex(diff).c_str());
    printf("A * B mod N = %s\n\n", ToHex(prod).c_str());
    printf("A ^ B mod N = %s\n\n", ToHex(pw).c_str());

    //int failed = runTests();
    //runBenchmarks();
   //return failed;
}
