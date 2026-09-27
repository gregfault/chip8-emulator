cmake_minimum_required(VERSION 3.28)
project(untitled1 C)

set(CMAKE_C_STANDARD 11)

add_executable(untilted1 main.c)

target_compile_options(untilted1 PRIVATE -Wall -Wextra)