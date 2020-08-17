cmake_minimum_required(VERSION 3.16)
project(edU_API C)

set(CMAKE_C_STANDARD 11)

add_executable(edU_API main.c)

target_compile_options(edU_API PRIVATE -DEVAL -std=gnu11 -pipe -static -s -lm)
