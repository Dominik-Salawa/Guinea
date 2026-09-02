cmake_minimum_required(VERSION 3.16)

project(guinea LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable("guinea" src/main.c)
set_target_properties(guinea PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(STATUS "Using Clang")
    target_compile_options(
        guinea PRIVATE 
        -Wpedantic -Wall -Wextra 
        -Werror -Wno-unused-parameter 
        -Wno-unused-variable 
        -Wno-unused-function -Os
        -fomit-frame-pointer -Wshadow
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    message(STATUS "Using GCC")
    target_compile_options(
        guinea PRIVATE 
        -Wpedantic -Wall -Wextra 
        -Werror -Wno-unused-parameter 
        -Wno-unused-variable -Wno-unused-function 
        -O0 -g -fomit-frame-pointer -Wshadow
    )
    #target_link_options(guinea PRIVATE -fsanitize=address)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Using MSVC")
    target_compile_options(
        guinea PRIVATE 
        /std:c11 /Za /W4 /WX 
        /wd4100 /wd4101 /wd4505
        /Os
    )
endif()

target_link_libraries(guinea PRIVATE m)