cmake_minimum_required(VERSION 3.12)
project(ice C)

set(CMAKE_C_STANDARD 99)
add_definitions(-DCOMPUTER_ICE)

include_directories(${CEDEV}/include)
include_directories(include)

add_executable(ice src/ast.c src/data.c src/errors.c src/export.c src/functions.c src/main_comp.c
        src/operator.c src/parse.c src/prescan.c src/routines.c src/stack.c)