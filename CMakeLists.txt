cmake_minimum_required(VERSION 3.0)
project(pl0)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_CXX_FLAGS_DEBUG -g)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs all)

add_executable(pl0 main.cpp lexer.cpp compiler.cpp table.cpp vm.cpp)
add_executable(llvmpl0 llvm_frontend.cpp lexer.cpp)
target_link_libraries(llvmpl0 ${llvm_libs})
