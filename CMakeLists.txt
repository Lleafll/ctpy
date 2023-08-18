cmake_minimum_required(VERSION 3.26)
project(ctpy)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME}
    doctest.cpp
    src/function.cpp
    src/lexer.cpp
)
target_include_directories(${PROJECT_NAME}
    PRIVATE include/ctpy
)

find_package(doctest CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE doctest::doctest)
