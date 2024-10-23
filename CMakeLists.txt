cmake_minimum_required(VERSION 3.17)

set(CMAKE_C_COMPILER clang)

project(iamboot)

add_compile_options(-O0 -g)

add_executable(iamboot
    main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/checksum/checksum.c
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol.c
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/checksum
)

