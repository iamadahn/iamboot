cmake_minimum_required(VERSION 3.17)

add_compile_options(-O0 -g)

add_library(iamboot INTERFACE)

if(IAMBOOT_LOW_LEVEL)
    set(SCRS
        ${CMAKE_CURRENT_LIST_DIR}/protocol_low_level.c
        ${CMAKE_CURRENT_LIST_DIR}/checksum.c
    )
elseif(IAMBOOT_HIGH_LEVEL)
    set(SCRS
        ${CMAKE_CURRENT_LIST_DIR}/protocol_high_level.c
        ${CMAKE_CURRENT_LIST_DIR}/checksum.c
    )
else()
    message(ERROR "Я симейк и я долбоеб")
endif()

target_sources(iamboot INTERFACE ${SCRS})
