cmake_minimum_required(VERSION 3.26)
project(ctpy)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include)

add_executable(ctpytest
    doctest.cpp
    src/function.cpp
    src/integration.cpp
    src/lexer.cpp
    src/parser.cpp
)
target_include_directories(ctpytest PRIVATE include/ctpy)
find_package(doctest CONFIG REQUIRED)
target_link_libraries(ctpytest PRIVATE doctest::doctest ${PROJECT_NAME})

add_executable(example1cpp examples/example1cpp.cpp)
target_compile_options(example1cpp PRIVATE "/FA")

add_executable(example1python examples/example1python.cpp)
target_link_libraries(example1python PRIVATE ${PROJECT_NAME})
target_compile_options(example1python PRIVATE "/FA")
