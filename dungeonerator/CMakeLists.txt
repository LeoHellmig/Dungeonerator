cmake_minimum_required(VERSION 3.29)
project(dungeonerator)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/
        INTERFACE generationUtils)

#if(MSVC)
#    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX)
#else()
#    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wpedantic)
#endif()

add_compile_definitions(-DLOGGING)