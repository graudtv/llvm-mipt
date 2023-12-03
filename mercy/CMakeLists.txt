cmake_minimum_required(VERSION 3.19)

project(mercy)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(LLVM REQUIRED CONFIG)
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

flex_target(lexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc)
bison_target(parser
  src/parser.y
  ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cc
  DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h"
  )
add_flex_bison_dependency(lexer parser)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

include_directories(
  src
  ${LLVM_INCLUDE_DIRS}
)

# qrisc-asm
add_executable(mercy
  src/AST.cpp
  src/Codegen.cpp
  src/Sema.cpp
  src/Type.cpp
  src/mercy.cpp
  ${FLEX_lexer_OUTPUTS}
  ${BISON_parser_OUTPUTS}
)
target_compile_definitions(mercy PRIVATE ${LLVM_DEFINITIONS_LIST})
target_link_libraries(mercy LLVMCore)

add_custom_target(test ALL
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_tests
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS mercy
)
set_target_properties(test PROPERTIES EXCLUDE_FROM_ALL 1)
