cmake_minimum_required(VERSION 3.19)

project(irgen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SFML 2.5 COMPONENTS system graphics window REQUIRED)
find_package(LLVM REQUIRED CONFIG)

separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

add_library(sim STATIC ${CMAKE_CURRENT_SOURCE_DIR}/../life_game/sim/sim.cpp)
target_link_libraries(sim PUBLIC sfml-graphics sfml-system sfml-window)
target_include_directories(sim INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../life_game/sim)

add_executable(irgen main.cpp)
target_include_directories(irgen PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_definitions(irgen PRIVATE ${LLVM_DEFINITIONS_LIST})
target_link_libraries(irgen LLVM sim)

