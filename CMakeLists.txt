cmake_minimum_required(VERSION 3.17)

add_compile_options(-O0 -g)

add_library(iamboot INTERFACE)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/protocol.c
    ${CMAKE_CURRENT_LIST_DIR}/checksum.c
)

target_sources(iamboot INTERFACE ${SCRS})

if(BUILD_EXECUTABLE)
    project(iamboot_app)
    add_executable(iamboot_app ${SCRS} main.c)
endif()
