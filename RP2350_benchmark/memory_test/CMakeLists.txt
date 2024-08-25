if (TARGET tinyusb_device)
    add_executable(memory_test
            memory_test.c
            )

    # pull in common dependencies
    target_link_libraries(memory_test pico_stdlib pico_rand pico_time)

    # enable usb output, disable uart output
    pico_enable_stdio_usb(memory_test 1)
    pico_enable_stdio_uart(memory_test 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_extra_outputs(memory_test)

    # add url via pico_set_program_url
    # example_auto_set_url(memory_test)

    # set custom linker script
    # pico_set_linker_script(memory_test ${CMAKE_CURRENT_SOURCE_DIR}/memmap_default.ld)

elseif(PICO_ON_DEVICE)
    message(WARNING "not building memory_test because TinyUSB submodule is not initialized in the SDK")
endif()
