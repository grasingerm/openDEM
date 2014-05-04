cmake_minimum_required (VERSION 2.8)

project (openDEM)

SET(CMAKE_BUILD_TYPE debug)
set(CMAKE_C_FLAGS_DEBUG "-Wextra -g")
set(CMAKE_C_FLAGS_RELEASE "-Wextra -g -O3")

set(CMAKE_BINARY_DIR build)
set(EXECUTABLE_OUTPUT_PATH bin)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})

add_executable (odem-sim main.c particle.c debug.c record.c analysis.c)
add_executable (odem-animate animate.c)

# c building in unix we need to link against math libraries
IF(UNIX)
    target_link_libraries (odem-sim m)
ENDIF(UNIX)

target_link_libraries (odem-sim sqlite3)
target_link_libraries (odem-animate ${GTK3_LIBRARIES})

install (TARGETS odem-sim DESTINATION bin)

