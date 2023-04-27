cmake_minimum_required(VERSION 3.24)
project(mk61emu)

set(CMAKE_CXX_STANDARD 17)

add_executable(mk61emu main.cpp mk61commander.cpp mk61emu.cpp mk_common.cpp)
