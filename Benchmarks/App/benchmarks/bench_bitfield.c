#include "bench_common.h"

/* Second ARM-favoring counterweight to bench_bitops.c. Cortex-M33 has
 * single-instruction bit-field extract (UBFX) and insert (BFI) - but
 * unlike the shifted-operand trick in bench_shift_fused.c, these encode
 * the field's bit position and width as immediates in the instruction,
 * so they only fire when both are compile-time constants (as they
 * realistically are for a fixed hardware register layout - which is why
 * this benchmark uses a #define'd shift/width instead of a runtime one).
 *
 * RISC-V's Zbs gives single-bit set/clear/invert/extract, not
 * arbitrary-width fields, so extracting/inserting a multi-bit field is
 * still plain shift+mask (extract) / shift+mask+and+or (insert) even with
 * Zbs. This pattern - unpacking/repacking a field inside a status or
 * config register - is extremely common in real peripheral driver code,
 * which is why it's worth its own benchmark rather than folding it into
 * bench_bitops.c.
 *
 * Checked against the actual disassembly: this one holds up as a clean
 * ARM win, unlike bench_shift_fused.c next to it. UBFX/BFI each fold the
 * extract/insert into a single instruction; RISC-V needs srli+andi for
 * the extract and slli+and+and+or for the insert - 8 instructions/
 * iteration on ARM vs 11 on RISC-V. */

#define BENCH_FIELD_SHIFT 12u
#define BENCH_FIELD_WIDTH 5u
#define BENCH_FIELD_MASK  ((1u << BENCH_FIELD_WIDTH) - 1u)

void bench_bitfield(uint32_t iterations)
{
    uint32_t reg = bench_seed;
    uint32_t acc = 0;

    for (uint32_t i = 0; i < iterations; i++)
    {
        /* Extract: candidate for UBFX on ARM. */
        uint32_t field = (reg >> BENCH_FIELD_SHIFT) & BENCH_FIELD_MASK;
        acc += field;

        /* Insert: candidate for BFI on ARM. */
        uint32_t new_field = (i ^ field) & BENCH_FIELD_MASK;
        reg = (reg & ~(BENCH_FIELD_MASK << BENCH_FIELD_SHIFT))
            | (new_field << BENCH_FIELD_SHIFT);

        reg += 0x9E3779B9u;
    }

    bench_sink = acc ^ reg;
}
