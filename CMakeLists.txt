cmake_minimum_required(VERSION 3.7.2)
project(altair_teletype C)

include_directories(lib8080/src)

set(CMAKE_C_STANDARD 99)

add_executable(altair_teletype lib8080/src/i8080.c teletype.c)
