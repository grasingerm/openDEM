cmake_minimum_required (VERSION 2.8)

project (openDEM)

set(CMAKE_C_FLAGS "-Wextra -g")

add_executable (dem main.c)

install (TARGETS dem DESTINATION bin)
