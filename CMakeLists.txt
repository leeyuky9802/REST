cmake_minimum_required(VERSION 3.23)
project(Final_Project)

set(CMAKE_CXX_STANDARD 14)

include_directories(checker)
include_directories(LS)

add_executable(checker
        checker/checker.cpp
        checker/checker.h)

add_executable(LS
        LS/LS.cpp
        LS/LS.h
        LS/LSMain.cpp
        LS/readInput.cpp)
