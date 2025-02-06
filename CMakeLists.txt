cmake_minimum_required(VERSION 3.17)

add_compile_options(-O0 -g)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR/protocol.c
    ${CMAKE_CURRENT_LIST_DIR/checksum.c
)

add_library(iamboot INTERFACE)

target_sources(iamboot INTERFACE ${SCRS})
