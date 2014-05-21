cmake_minimum_required(VERSION 2.8)

project(afemhw)

# compiler flags

if(UNIX)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang)
endif(UNIX)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -g -std=c99 -pedantic -pedantic-errors")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -g -std=c99 -pedantic -pedantic-errors -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -std=c++11 -pedantic -pedantic-errors")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -g -std=c++11 -pedantic -pedantic-errors -O3")

# build directories
set(CMAKE_BINARY_DIR build)
set(EXECUTABLE_OUTPUT_PATH bin)

# dependencies
add_executable(hw2 main.cpp element.cpp)

install(TARGETS hw2 DESTINATION bin)

