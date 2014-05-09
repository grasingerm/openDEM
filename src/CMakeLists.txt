cmake_minimum_required (VERSION 2.8)

project (openDEM)

SET(CMAKE_BUILD_TYPE debug)
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -g")
set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -g -O3")

set(CMAKE_BINARY_DIR build)
set(EXECUTABLE_OUTPUT_PATH bin)

add_executable (odem-sim main.c particle.c debug.c record.c analysis.c)
add_executable (odem-animate animate4.c)

# c building in unix we need to link against math libraries
IF(UNIX)
    target_link_libraries (odem-sim m)
ENDIF(UNIX)

target_link_libraries (odem-sim sqlite3)
target_link_libraries (odem-animate allegro)

install (TARGETS odem-sim DESTINATION bin)

