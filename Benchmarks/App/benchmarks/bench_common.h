#pragma once
#include <stdint.h>

/* Every benchmark writes its final result here. Reading/writing a volatile
 * stops the compiler from proving the loop has no observable effect and
 * deleting it under -O2/-O3. */
extern volatile uint32_t bench_sink;

/* Benchmarks derive their working data from this instead of a literal
 * constant, so the compiler can't constant-fold the whole loop at compile
 * time (it can't know a volatile's value ahead of execution). */
extern volatile uint32_t bench_seed;
