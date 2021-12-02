# clang_parse_ast_example
Simple clang AST parsing project for bugs identification and fixing.

# Build:
1. sudo apt-get install llvm-12-dev
2. sudo touch /usr/lib/llvm-12/bin/yaml-bench
3. sudo apt-get install libclang-12-dev
4. use Cmake and make

# Run:
Please see vs code config, dont forget to add "-std=c++11"

# Current bug:
ParsedAttr::getNumArgs() always return zero.
Attribute example: "[[kslicer::size(a_colorSize)]]"
