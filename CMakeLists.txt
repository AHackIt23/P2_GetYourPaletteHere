cmake_minimum_required(VERSION 3.5...3.30)
project(P2_GetYourPaletteHere VERSION 1.0.0 LANGUAGES C CXX)

# Require C++20 for <filesystem> and modern C++ features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find necessary external packages
find_package(SDL2 REQUIRED)

# Suppress CMP0072 warning by explicitly using GLVND
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()
find_package(OpenGL REQUIRED)

set (MAIN_SRC src/main.cpp)
# Define the source files for our executable
set(IMGUI_FILES
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
    external/imgui/backends/imgui_impl_sdlrenderer2.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
)

#combine source files
set(SRC_FILES ${MAIN_SRC} ${IMGUI_FILES})

# Create the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link external libraries (SDL2 and OpenGL)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL2::SDL2
    SDL2::SDL2main
    OpenGL::OpenGL
)

# Specify all header search paths (stb, imgui, and SDL)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${SDL2_INCLUDE_DIRS}
)
