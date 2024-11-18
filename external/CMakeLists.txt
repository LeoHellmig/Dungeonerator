cmake_minimum_required(VERSION 3.29)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(SDL)


target_compile_options(SDL3-static PRIVATE "-Wno-deprecated-declarations")

file(GLOB imgui_sources CONFIGURE_DEPENDS "imgui/*.cpp")
file(GLOB imgui_headers CONFIGURE_DEPENDS "imgui/*.h")

set(imgui_files ${imgui_sources} ${imgui_headers}
        imgui/misc/cpp/imgui_stdlib.cpp
        imgui/misc/freetype/imgui_freetype.cpp
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_sdlrenderer3.h
        imgui/backends/imgui_impl_sdlrenderer3.cpp
)

add_library(ImGui ${imgui_files} ${implot_files})
target_compile_definitions(ImGui PRIVATE SDL_ENABLE_OLD_NAMES)

target_link_libraries(ImGui
        PRIVATE SDL3::SDL3-static)

target_include_directories(ImGui
        PRIVATE freetype/include
        PUBLIC imgui
        PUBLIC imgui/backends
)


add_library(external INTERFACE)

target_link_libraries(external INTERFACE
        ImGui
        SDL3::SDL3-static
)

target_include_directories(external
        INTERFACE SDL/include
        INTERFACE glm
)