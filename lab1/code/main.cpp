#include "tests.h"
#include "benchmarks.h"
#define NOMINMAX
#include <windows.h>
#include <cstdio>

int main(){
    SetConsoleOutputCP(CP_UTF8);
    int failed = runTests();
    runBenchmarks();
    return failed;
}
