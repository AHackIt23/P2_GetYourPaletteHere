cmake_minimum_requirement(VERSION 3.10)
project(P2_GetYourPaletteHere)

#Include directories
include_directories(include include.imgui include/stb)

#include source files 
file(global projectSources src/imgui/*.cpp)

#create executable
add_executable(app src/main.cpp ${projectSources})

#find and link sdl2 and opengl
find_package(SDL2 REQUIRED)
target_link_libraries(app PRIVATE SDL2::SDL2 SDL2::SDL2main ${OPENGL_LIBRARIES})
