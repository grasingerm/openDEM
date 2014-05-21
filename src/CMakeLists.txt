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
add_executable (odem-animate animate.c debug.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})

# c building in unix we need to link against math libraries
if(UNIX)
    target_link_libraries (odem-sim m)
endif(UNIX)

target_link_libraries (odem-sim sqlite3)
target_link_libraries (odem-animate ${GTK3_LIBRARIES} sqlite3)

install (TARGETS odem-sim DESTINATION bin)

