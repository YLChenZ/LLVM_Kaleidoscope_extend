cmake_minimum_required(VERSION 3.13)
project(MyCompiler)

find_package(LLVM REQUIRED CONFIG)
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(Parser token.cpp ast.cpp lexer.cpp parser.cpp runparser.cpp)

llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(Parser ${llvm_libs})

