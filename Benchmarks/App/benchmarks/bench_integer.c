#include "bench_common.h"

/* Tight loop of add/sub/xor/shift - no multiply or divide, so this isolates
 * plain ALU throughput from the muldiv benchmark.
 *
 * The split is deliberate. add/sub/xor/shift all issue to the same
 * single-cycle ALU, while multiply goes to a dedicated multiplier and divide
 * to an iterative sequencer costing an order of magnitude more. Mixed into
 * one loop the divide would dominate the cycle count and bury any ALU-level
 * difference - you'd be measuring the divider and calling it "integer
 * performance". Divide latency is also data-dependent (it varies with
 * operand magnitude on both cores), so a combined benchmark would drift with
 * bench_seed; kept separate, this one is deterministic.
 *
 * The two units can differ in opposite directions, too, so a combined number
 * would mostly reflect the arbitrary ratio of ALU ops to divides in the loop
 * rather than anything about the chip. Split, a measured delta can be
 * attributed to one functional unit - which is the whole point.
 *
 * What the -O3 disassembly actually shows: 10 instructions per iteration on
 * both cores, reached by opposite routes. ARM's barrel shifter folds both
 * shifts into the arithmetic (add.w r3, r1, r2, lsl #3 and sub.w r3, r3, r1,
 * lsr #2) but spends an instruction on a separate cmp before the branch.
 * RISC-V folds the <<3 into Zba's sh3add and needs a standalone srli for the
 * >>2, but its fused compare-and-branch means no cmp - the same effect
 * documented in bench_shift_fused.c. Two wins each; net tie on instruction
 * count, which is exactly the kind of result worth measuring rather than
 * predicting. */
void bench_integer_arith(uint32_t iterations)
{
    uint32_t a = bench_seed;
    uint32_t b = bench_seed ^ 0xA5A5A5A5u;

    for (uint32_t i = 0; i < iterations; i++)
    {
        a = (a + b) ^ (a - b);
        b = (b + 1u) | 1u;
        a += (b << 3) - (a >> 2);
    }

    bench_sink = a ^ b;
}
