cmake_minimum_required(VERSION 3.0)
project(pl0)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS_DEBUG -g)

add_executable(pl0 main.cpp lexer.cpp)
