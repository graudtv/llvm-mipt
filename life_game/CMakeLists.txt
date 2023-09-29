cmake_minimum_required(VERSION 3.16)

project(life_game)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SFML 2.5 COMPONENTS system graphics window REQUIRED)

add_library(sim STATIC sim/sim.cpp)
target_link_libraries(sim PUBLIC sfml-graphics sfml-system sfml-window)
target_include_directories(sim INTERFACE sim)

add_executable(life_game src/life.c)
target_link_libraries(life_game sim)
