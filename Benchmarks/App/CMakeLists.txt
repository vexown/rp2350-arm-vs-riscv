add_executable(bench_suite
        main.c
        benchmarks/bench_common.c
        benchmarks/bench_integer.c
        benchmarks/bench_muldiv.c
        benchmarks/bench_memcpy.c
        benchmarks/bench_bitops.c
)

target_include_directories(bench_suite PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/benchmarks
)

target_link_libraries(bench_suite PRIVATE
        pico_stdlib
)

# Results are read back over the USB CDC serial port - no extra UART wiring
# needed to get started. See Tools/ and the README for how to swap to a wired
# UART if you want to correlate with a logic analyzer/scope.
pico_enable_stdio_usb(bench_suite 1)
pico_enable_stdio_uart(bench_suite 0)

pico_add_extra_outputs(bench_suite)
