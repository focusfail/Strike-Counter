cmake_minimum_required(VERSION 3.20)
project(strike_counter LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(GLFW_BUILD_DOCS CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES CACHE BOOL "" FORCE)

set(SC_SRC "${PROJECT_SOURCE_DIR}")
set(SC_EXT "${SC_SRC}/external")

# glad
add_library(glad  "${SC_EXT}/glad/src/gl.c")
target_include_directories(glad PUBLIC
    "${SC_EXT}/glad/include/")

# inih
add_library(inih "${SC_EXT}/inih/ini.c")
target_include_directories(inih PUBLIC "${SC_EXT}/inih/inih")

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${SC_EXT}/stb/")

# common strike_counter lib
file(GLOB_RECURSE strike_counter_lib_src "${PROJECT_SOURCE_DIR}/common/*.c")
add_library(strike_counter_lib ${strike_counter_lib_src})
target_include_directories(strike_counter_lib PUBLIC "${PROJECT_SOURCE_DIR}/common/")

add_subdirectory("${SC_EXT}/glfw")
add_subdirectory("${SC_EXT}/cglm")

add_executable(strike_counter)
target_sources(
    strike_counter
    PRIVATE
    "${SC_SRC}/client/client/main.c"
    "${SC_SRC}/client/client/bsp.c"
    "${SC_SRC}/client/client/wad.c"
    "${SC_SRC}/client/client/camera.c"
    "${SC_SRC}/client/client/window.c"
    "${SC_SRC}/client/client/gl_common.c"
    "${SC_SRC}/client/client/bsp_gfx.c"
)

target_include_directories(
    strike_counter
    PRIVATE
    "${PROJECT_SOURCE_DIR}/client/"
    "${SC_EXT}/nuklear"
)
target_link_libraries(strike_counter PRIVATE glfw glad cglm strike_counter_lib stb)
target_compile_definitions(strike_counter PUBLIC SC_ASSET_PATH="${PROJECT_SOURCE_DIR}/client/assets")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # target_compile_options(strike_counter PRIVATE -ggdb -Wall)
    # target_link_options(strike_counter PRIVATE)
    target_compile_options(strike_counter PRIVATE -fsanitize=address -ggdb -Wall)
    target_link_options(strike_counter PRIVATE -fsanitize=address)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # target_compile_options(strike_counter PRIVATE -O3)
    target_compile_options(strike_counter PRIVATE -ggdb)
    target_link_options(strike_counter PRIVATE)
endif()
