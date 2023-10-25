cmake_minimum_required(VERSION 3.19)

project(trace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

find_package(LLVM REQUIRED CONFIG)
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

add_library(TraceGen MODULE TraceGenPass.cpp)
target_include_directories(TraceGen PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(TraceGen PRIVATE ${LLVM_DEFINITIONS_LIST})

# build life.ll and life_game utilizing TraceGenPass
add_custom_target(life_game ALL
  COMMAND clang ${CMAKE_CURRENT_SOURCE_DIR}/../life_game/src/life.c
                -I ${CMAKE_CURRENT_SOURCE_DIR}/../life_game/sim
                -O2 -S -emit-llvm -o life.ll
  COMMAND clang life.ll
                -Xclang -load -Xclang ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:TraceGen>
                -flegacy-pass-manager -O2 -c -o life.o
  COMMAND clang++ ${CMAKE_CURRENT_SOURCE_DIR}/../life_game/sim/sim.cpp life.o 
                -lsfml-system -lsfml-window -lsfml-graphics -O2 -o life_game
  DEPENDS TraceGen
  )
