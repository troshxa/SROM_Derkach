#include "tests.h"
#include "benchmarks.h"
#include "bignumarifmetic.h"
#define NOMINMAX
#include <windows.h>
#include <cstdio>

int main(){
    SetConsoleOutputCP(CP_UTF8);

    BigNum A, B, C;
    FromHex("0x202171731bd25f23cb5723b759bce0c00632334c4e9917fc14be434bf9db4b8653c39df5c563ea9c5e314de587a68261a407ad8512ba79993b280022e269ed74f37bfe8e74bafb86682da9034d76c812556e60745617ca65ab62ea46746ef58b96c852f237bf18c53e30f21eacdc84fd42ebaad5c247490cb7d55e879a12707c", A);
    FromHex("0x1cfe4177fcb132f70cea7cd2b8fc2ab59995f89f6b84e3802e01883cd2aabac09263f8c6c86dff224b40c7f8dea2002edbccae774c33c947ef9761fd446cca6b24df6d2cdf45936f411c089d3039d86d028247ba8586960bdb4eea57d648576564883c85c735d1290c07872f9e1ff57688009b83071c2d3cd5918acf86fc0e84", B);
    FromHex("0xb406f842b945341d32d8eceaddedd3046cf46347d09fca1f528785532eb8f16e5338791703dbc418dfa130c310bf6225627231bcd78ce64b3d4dcac838f16318", C);

    printf("A = %s\n", ToHex(A).c_str());
    printf("B = %s\n", ToHex(B).c_str());
    printf("C = %s\n\n", ToHex(C).c_str());

    BigNum sum, diff, prod, q, r;
    LongAdd(A, B, sum);
    LongSub(A, B, diff);
    LongMul(A, B, prod);
    LongDivMod(A, C, q, r);

    printf("A + B = %s\n\n", ToHex(sum).c_str());
    printf("A - B = %s\n\n", ToHex(diff).c_str());
    printf("A * B = %s\n\n", ToHex(prod).c_str());
    printf("A / C = %s\n\n", ToHex(q).c_str());
    printf("A mod C = %s\n\n", ToHex(r).c_str());

    // int failed = runTests();
    // runBenchmarks();
    return 0;
}
