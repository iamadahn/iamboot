cmake_minimum_required(VERSION 3.17)

set(CMAKE_C_COMPILER clang)

project(iamboot)

add_executable(iamboot main.c)

add_compile_options(-O0 -g)
