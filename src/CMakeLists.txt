cmake_minimum_required (VERSION 2.8)

project (openDEM)

SET(CMAKE_BUILD_TYPE debug)
set(CMAKE_C_FLAGS_DEBUG "-Wextra -g")
set(CMAKE_C_FLAGS_RELEASE "-Wextra -g -O3")

set(CMAKE_BINARY_DIR build)
set(EXECUTABLE_OUTPUT_PATH bin)

add_executable (dem main.c particle.c debug.c record.c analysis.c)

# c building in unix we need to link against math libraries
IF(UNIX)
    target_link_libraries (dem m)
ENDIF(UNIX)

target_link_libraries (dem sqlite3)

install (TARGETS dem DESTINATION bin)
