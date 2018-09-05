#include <stdint.h>
#include <stdio.h>

uint64_t write(uint64_t n) { return printf("%lld\n", n); }

uint64_t writeln() { return printf("\n"); }
