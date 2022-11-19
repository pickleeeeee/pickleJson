cmake_minimum_required(VERSION 3.19)
project(cJson C)

set(CMAKE_C_STANDARD 90)

add_executable(cJson picklejson.h test.c picklejson.c)