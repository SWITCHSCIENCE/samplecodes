add_executable(idle
    idle.c
    )

# pull in common dependencies
target_link_libraries(idle pico_stdlib)

if (PICO_CYW43_SUPPORTED)
    target_link_libraries(idle pico_cyw43_arch_none)
endif()

# create map/bin/hex file etc.
pico_add_extra_outputs(idle)

# add url via pico_set_program_url
# example_auto_set_url(idle)
