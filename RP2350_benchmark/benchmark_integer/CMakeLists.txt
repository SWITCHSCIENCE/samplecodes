if (TARGET tinyusb_device)
    add_executable(benchmark_integer
            benchmark_integer.c
            )

    # pull in common dependencies
    target_link_libraries(benchmark_integer pico_stdlib pico_rand pico_time)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(benchmark_integer 1)
    pico_enable_stdio_uart(benchmark_integer 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(benchmark_integer)

    # add url via pico_set_program_url
    # example_auto_set_url(benchmark_integer)

    # set custom linker script
    # pico_set_linker_script(benchmark_integer ${CMAKE_CURRENT_SOURCE_DIR}/memmap_default.ld)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building benchmark_integer because TinyUSB submodule is not initialized in the SDK")
endif()
