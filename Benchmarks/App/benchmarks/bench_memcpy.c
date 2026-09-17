#include <string.h>
#include "bench_common.h"

#define BENCH_MEMCPY_BUF_SIZE 256

/* Repeated memcpy of a fixed-size buffer - exercises libc's memcpy
 * implementation for each toolchain/target rather than a hand-rolled copy
 * loop, since in real firmware that's what actually runs. */
void bench_memcpy_workload(uint32_t iterations) {
    static uint8_t src[BENCH_MEMCPY_BUF_SIZE];
    static uint8_t dst[BENCH_MEMCPY_BUF_SIZE];

    for (uint32_t i = 0; i < BENCH_MEMCPY_BUF_SIZE; i++) {
        src[i] = (uint8_t)(bench_seed + i);
    }

    for (uint32_t i = 0; i < iterations; i++) {
        memcpy(dst, src, BENCH_MEMCPY_BUF_SIZE);
        src[i % BENCH_MEMCPY_BUF_SIZE] ^= dst[(i + 1) % BENCH_MEMCPY_BUF_SIZE];
    }

    bench_sink = dst[0];
}
