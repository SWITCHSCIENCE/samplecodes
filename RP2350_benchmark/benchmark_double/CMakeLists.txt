if (TARGET tinyusb_device)
    add_executable(benchmark_double
            benchmark_double.c
            )

    # pull in common dependencies
    target_link_libraries(benchmark_double pico_stdlib pico_rand pico_time)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(benchmark_double 1)
    pico_enable_stdio_uart(benchmark_double 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(benchmark_double)

    # add url via pico_set_program_url
    # example_auto_set_url(benchmark_double)

    # set custom linker script
    # pico_set_linker_script(benchmark_double ${CMAKE_CURRENT_SOURCE_DIR}/memmap_default.ld)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building benchmark_double because TinyUSB submodule is not initialized in the SDK")
endif()
