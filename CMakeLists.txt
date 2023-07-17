cmake_minimum_required(VERSION 3.25)
project(cppplayground)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fexperimental-library -stdlib=libc++")

add_executable(cppplayground src/main.cpp)
add_executable(diagonalTraversal src/DiagonalTraversal.cpp)



include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/fb11778f43d5844ca9e2016bae75b22d26ea451f.zip
)

FetchContent_MakeAvailable(googletest)
enable_testing()
add_executable(
      tests
      test/binomialOpinionTest.cpp
      test/multinomialOpinionTest.cpp
        src/subjective_logic/Operations.h)
target_link_libraries(
        tests
        GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(tests)
