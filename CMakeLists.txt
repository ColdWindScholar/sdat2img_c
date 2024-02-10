cmake_minimum_required(VERSION 3.2)
project(sdat2img C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-static")
add_executable(sdat2img main.c
        main.h)
