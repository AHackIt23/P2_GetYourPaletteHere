cmake_minimum_required(VERSION 3.10)
project(P2_GetYourPaletteHere VERSION 1.0.0 LANGUAGES CXX)

# Require C++20 for <filesystem> and modern C++ features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#this is so that if anyone wants to run the code and they dont have like the sdl2 stuff then they an still run it as its now in an external folder
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2")
    set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2")
    message(STATUS "SDL2 found in palette project: ${SDL2_PATH}")

    set(SDL2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2")
    if (WIN32)
        set(SDL2_LIBRARY "${SDL2_PATH}/lib/x64/SDL2.lib")
        set(SDL2MAIN_LIBRARY "${SDL2_PATH}/lib/x64/SDL2main.lib")
    else()
        set(SDL2_LIBRARY "SDL2")
        set(SDL2MAIN_LIBRARY "SDL2main")
    endif()
else()

    find_package(SDL2 REQUIRED)
    set(SDL2_INCLUDE_DIR ${SDL2_INCLUDE_DIRS})
    set(SDL2_LIBRARY ${SDL2_LIBRARIES})
    set(SDL2MAIN_LIBRARY "")
    message(STATUS "SDL2 found using find_package")
endif()

#define project sources
set(projectSources
    src/main.cpp
    src/Color.cpp
    src/MedianCut.cpp
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

target_link_libraries(P2_GetYourPaletteHere PRIVATE
    ${SDL2MAIN_LIBRARY}
    ${SDL2_LIBRARY}
    GL
)


#include directories
target_include_directories(P2_GetYourPaletteHere PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/backends
    ${SDL2_INCLUDE_DIR}
)

if(WIN32 AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2/lib/x64/SDL2.dll)
    add_custom_command(TARGET P2_GetYourPaletteHere POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2/lib/x64/SDL2.dll
            $<TARGET_FILE_DIR:P2_GetYourPaletteHere>)
endif()


