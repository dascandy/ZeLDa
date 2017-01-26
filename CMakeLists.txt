cmake_minimum_required(VERSION 3.5)

add_definitions(-g -std=c++14 -Wall -Wextra -Wpedantic -Werror -pedantic-errors)

add_subdirectory(elf)
add_subdirectory(file)
add_subdirectory(zelda)

