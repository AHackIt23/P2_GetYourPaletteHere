cmake_minimum_requirement(VERSION 3.10)
project(P2_GetYourPaletteHere VERSION 1.0.0 LANGUAGES CXX)

#require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#define project sources
set(projectSources
    src/main.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_impl_opengl3.cpp
    src/imgui/imgui_impl_sdl2.cpp
    src/imgui/imgui_impl_sdlrenderer2.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
)

#define executable
add_executable(P2_GetYourPaletteHere ${projectSources})

#find and link sdl2
find_package(SDL2 REQUIRED)
target_link_libraries(P2_GetYourPaletteHere PRIVATE SDL2::SDL2 SDL2::SDL2main)

#include directories
target_include_directories(P2_GetYourPaletteHere PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui
    ${SDL2_INCLUDE_DIRS}
)
