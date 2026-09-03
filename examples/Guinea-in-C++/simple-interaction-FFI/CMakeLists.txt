# you can simply run 'cmake .', this isnt meant to have a build

cmake_minimum_required(VERSION 3.16)

project(CC_link_test LANGUAGES C CXX)

add_executable(example "example.cc" "../../../src/guinea.c")

if(UNIX)
    set_target_properties(example PROPERTIES SUFFIX ".out")
endif()