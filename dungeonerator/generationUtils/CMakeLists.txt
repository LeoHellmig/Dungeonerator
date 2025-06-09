cmake_minimum_required(VERSION 3.29)

project(generationUtils)

set(HEADERS delaunator.hpp PoissonGenerator.hpp)

add_library(${PROJECT_NAME} ${HEADERS} )

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

#target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic)
