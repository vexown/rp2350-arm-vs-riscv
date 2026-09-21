#include "bench_common.h"

/* Multiply + divide heavy loop. Both Cortex-M33 and Hazard3 (RV32IMAC) have
 * a hardware M-extension/MUL-DIV unit on this chip, so this should be a
 * fair hardware-vs-hardware comparison rather than exposing a soft-div
 * fallback on one side. */
void bench_muldiv(uint32_t iterations)
{
    uint32_t a = bench_seed | 1u;
    uint32_t acc = 0;

    for (uint32_t i = 0; i < iterations; i++)
    {
        uint32_t m = a * 2654435761u;
        uint32_t d = m / (a | 1u);
        acc += d;
        a = (a * 1103515245u + 12345u) | 1u;
    }

    bench_sink = acc;
}
