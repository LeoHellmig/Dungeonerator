cmake_minimum_required(VERSION 3.29)
project(grammars)

set(CMAKE_CXX_STANDARD 20)

set(HEADERS symbol_data.hpp symbol_registry.hpp grammar.hpp grammar_rule.hpp graph.hpp)
set(SOURCES symbol_data.cpp symbol_registry.cpp grammar.cpp grammar_rule.cpp graph.cpp)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic)
endif()