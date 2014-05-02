cmake_minimum_required (VERSION 2.8)

project (openDEM)

set(CMAKE_C_FLAGS "-Wextra -g")

set(EXECUTABLE_OUTPUT_PATH bin)

add_executable (dem main.c)

# if building in unix we need to link against math libraries
IF(UNIX)
    TARGET_LINK_LIBRARIES(dem m)
ENDIF(UNIX)

install (TARGETS dem DESTINATION bin)
