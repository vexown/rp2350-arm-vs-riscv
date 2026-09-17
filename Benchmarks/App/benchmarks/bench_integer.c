#include "bench_common.h"

/* Tight loop of add/sub/xor/shift - no multiply or divide, so this isolates
 * plain ALU throughput from the muldiv benchmark. */
void bench_integer_arith(uint32_t iterations) {
    uint32_t a = bench_seed;
    uint32_t b = bench_seed ^ 0xA5A5A5A5u;

    for (uint32_t i = 0; i < iterations; i++) {
        a = (a + b) ^ (a - b);
        b = (b + 1u) | 1u;
        a += (b << 3) - (a >> 2);
    }

    bench_sink = a ^ b;
}
