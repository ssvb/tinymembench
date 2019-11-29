cmake_minimum_required(VERSION 3.0)
project(tinymembench ASM C)
set(CMAKE_C_STANDARD 11)

set(asm_files
    aarch64-asm.S
    arm-neon.S
    mips-32.S
    x86-sse2.S
)

set(sources_files
    asm-opt.c
    main.c
    util.c
)

add_executable(tinymembench ${asm_files} ${sources_files})
target_include_directories(tinymembench PRIVATE .)
target_link_libraries(tinymembench m)

INSTALL(TARGETS tinymembench RUNTIME DESTINATION /usr/bin/)
