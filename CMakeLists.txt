cmake_minimum_required (VERSION 3.8)
project (test_ast)
set (CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories("include")
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_INCLUDE_DIRS}/../lib)
add_definitions(${LLVM_DEFINITIONS})

message("${LLVM_INCLUDE_DIRS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(test 
               main.cpp
               attributes.cpp)

target_link_libraries(test PRIVATE -lclang -lLLVM-12 
                                   -lclangAST -lclangTooling -lclangBasic -lclangASTMatchers 
                                   -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization 
                                   -lclangCodeGen -lclangParse -lclangSema -lclangStaticAnalyzerFrontend 
                                   -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis
                                   -lclangIndex -lclangARCMigrate -lclangRewrite -lclangAST -lclangLex -lclangEdit
                     ) 
