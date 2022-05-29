cmake_minimum_required (VERSION 3.10.2)
SET(CMAKE_CXX_COMPILER g++)
project (threadsafe_stack_test)
add_compile_options(-Wall -Wextra -Ofast)
include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/main.cpp)
add_executable (${PROJECT_NAME} "${SOURCES}")
target_link_libraries (${PROJECT_NAME} -lpthread)

