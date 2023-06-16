cmake_minimum_required(VERSION 3.25)
project(cppplayground)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS  OFF)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fexperimental-library -stdlib=libc++")

add_executable(cppplayground main.cpp, src/SubjectiveLogic.h)
