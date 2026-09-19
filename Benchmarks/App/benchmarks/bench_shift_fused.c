#include "bench_common.h"

/* Counterweight to bench_bitops.c: that one showed a RISC-V-favoring
 * asymmetry (Zbb gives Hazard3 free popcount/clz/rotate). This one was
 * written to probe an ARM-favoring asymmetry instead - Cortex-M33's
 * barrel shifter can fold "shift by a constant, then some other ALU op"
 * into one instruction via the shifted-register second operand (e.g.
 * `AND r3, ip, r2, LSR #6`), where RISC-V's base ISA always needs a
 * separate shift instruction first.
 *
 * Checked against the actual disassembly (Results/bench_suite_*.lst),
 * the result is a near-exact tie (13 instructions/iteration both sides)
 * for a more interesting reason than either side "winning": GCC applies
 * induction-variable strength reduction to two of the three shifted terms
 * on BOTH architectures - since `b` only ever changes by the same
 * compile-time constant each iteration, `b<<5`/`b<<7` get maintained as
 * running values updated by a plain add instead of re-shifted every time,
 * which sidesteps the barrel-shifter question entirely for those two
 * terms, identically on both sides. Only the third term (`a>>6`, where
 * `a`'s per-iteration delta is the loop counter, not a constant) still
 * needs a real shift each time - and there ARM's fusion does save one
 * instruction (`AND ... LSR #6` vs RISC-V's separate `srli`+`and`). But
 * RISC-V claws that exact instruction back elsewhere: its `bne` is a
 * fused compare-and-branch (two operands compared directly, no flags
 * register), while ARM needs a separate `cmp` before `bne.n`. Net: a
 * wash for this snippet.
 *
 * Kept as-is rather than "fixed" to win, because the lesson is more
 * useful than the win would have been: isolated ISA feature comparisons
 * (this instruction exists / that one doesn't) can get erased or
 * offset by whatever else the compiler and the rest of the instruction
 * stream are doing, so treat any single synthetic benchmark as a
 * starting hypothesis to check against real disassembly, not a verdict.
 */
void bench_shift_fused(uint32_t iterations) {
    uint32_t a = bench_seed;
    uint32_t b = bench_seed ^ 0x9E3779B9u;
    uint32_t acc = 0;

    for (uint32_t i = 0; i < iterations; i++) {
        acc += a ^ (b << 5);
        acc += b & (a >> 6);
        acc -= a | (b << 7);

        a += i;
        b += 0x9E3779B9u;
    }

    bench_sink = acc;
}
