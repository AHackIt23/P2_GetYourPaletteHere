cmake_minimum_required(VERSION 3.5)
project(P2_GetYourPaletteHere VERSION 1.0.0 LANGUAGES C CXX)

# Require C++20 for <filesystem> and modern C++ features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find necessary external packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

# Define the source files for our executable
set(SRC_FILES
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

# Create the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link external libraries (SDL2 and OpenGL)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL::GL
)

# Specify all header search paths (stb, imgui, and SDL)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stb
    ${SDL2_INCLUDE_DIRS}
)
