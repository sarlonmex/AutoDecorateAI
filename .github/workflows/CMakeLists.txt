cmake_minimum_required(VERSION 3.21)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AutoDecorateAI)

find_package(Geode REQUIRED)

add_geode_mod(
    AutoDecorateAI
    src/main.cpp
)
