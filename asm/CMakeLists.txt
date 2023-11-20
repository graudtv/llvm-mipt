cmake_minimum_required(VERSION 3.19)

project(qrisc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SFML 2.5 COMPONENTS system graphics window REQUIRED)
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

add_library(sim STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../life_game/sim/sim.cpp)
target_link_libraries(sim PUBLIC sfml-graphics sfml-system sfml-window)
target_include_directories(sim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../life_game/sim)

include_directories(
  src
  ${LLVM_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# qrisc-asm
add_executable(qrisc-asm
  src/qrisc-asm.cpp
  src/Assembler.cpp
  ${FLEX_lexer_OUTPUTS}
  ${BISON_parser_OUTPUTS}
)
target_compile_definitions(qrisc-asm PRIVATE ${LLVM_DEFINITIONS_LIST})
target_link_libraries(qrisc-asm LLVMCore)

# qrisc-sim
add_executable(qrisc-sim
  src/qrisc-sim.cpp
  src/Assembler.cpp
  ${FLEX_lexer_OUTPUTS}
  ${BISON_parser_OUTPUTS}
)
target_compile_definitions(qrisc-sim PRIVATE ${LLVM_DEFINITIONS_LIST})
target_link_libraries(qrisc-sim LLVMCore sim)

# qrisc-llvm-sim
add_executable(qrisc-llvm-sim
  src/qrisc-llvm-sim.cpp
  src/Assembler.cpp
  ${FLEX_lexer_OUTPUTS}
  ${BISON_parser_OUTPUTS}
)
target_compile_definitions(qrisc-llvm-sim PRIVATE ${LLVM_DEFINITIONS_LIST})
target_link_libraries(qrisc-llvm-sim LLVM sim)
