cmake_minimum_required (VERSION 2.8)

project (openDEM)

set(CMAKE_BUILD_TYPE debug)
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -g -std=c99 -pedantic -pedantic-errors")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -g -std=c99 -pedantic -pedantic-errors -O3")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -std=c++11 -pedantic -pedantic-errors")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -g -std=c++11 -pedantic -pedantic-errors -O3")

set(CMAKE_BINARY_DIR build)
set(EXECUTABLE_OUTPUT_PATH bin)

add_executable (odem-sim main.c particle.c debug.c record.c analysis.c)

# c building in unix we need to link against math libraries
if(UNIX)
    target_link_libraries (odem-sim m)
endif(UNIX)

target_link_libraries (odem-sim sqlite3)

install (TARGETS odem-sim DESTINATION bin)

