cmake_minimum_required(VERSION 3.20)
project(strike_counter LANGUAGES C VERSION 0.0.1)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(GLFW_BUILD_DOCS CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES CACHE BOOL "" FORCE)

set(SC_SRC "${PROJECT_SOURCE_DIR}/src")
set(SC_EXT "${PROJECT_SOURCE_DIR}/third_party")

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
file(GLOB_RECURSE strike_counter_lib_src "${SC_SRC}/common/*.c")
add_library(strike_counter_lib ${strike_counter_lib_src})
target_include_directories(strike_counter_lib PUBLIC "${SC_SRC}/")

add_subdirectory("${SC_EXT}/glfw")
add_subdirectory("${SC_EXT}/cglm")

add_executable(strike_counter)
target_sources(
    strike_counter
    PRIVATE
    "${SC_SRC}/client/main.c"
    "${SC_SRC}/client/bsp.c"
    "${SC_SRC}/client/wad.c"
    "${SC_SRC}/client/camera.c"
    "${SC_SRC}/client/console.c"
    "${SC_SRC}/client/window.c"
    "${SC_SRC}/client/gl_common.c"
    "${SC_SRC}/client/bsp_gfx.c"
)

target_include_directories(
    strike_counter
    PRIVATE
    "${SC_SRC}/"
    "${SC_EXT}/nuklear"
)
target_link_libraries(strike_counter PRIVATE glfw glad cglm strike_counter_lib stb)
target_compile_definitions(strike_counter PUBLIC SC_ASSET_PATH="${PROJECT_SOURCE_DIR}/assets")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # target_compile_options(strike_counter PRIVATE -ggdb -Wall)
    # target_link_options(strike_counter PRIVATE)
    target_compile_options(strike_counter PRIVATE -fsanitize=address,undefined,leak -ggdb -Wall -fno-omit-frame-pointer)
    target_link_options(strike_counter PRIVATE -fsanitize=address,undefined,leak)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # target_compile_options(strike_counter PRIVATE -O3)
    target_compile_options(strike_counter PRIVATE -ggdb)
    target_link_options(strike_counter PRIVATE)
endif()
