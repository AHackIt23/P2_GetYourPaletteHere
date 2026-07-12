cmake_minimum_required(VERSION 3.10)
project(P2_GetYourPaletteHere VERSION 1.0.0 LANGUAGES CXX)

# Require C++20 for <filesystem> and modern C++ features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

#this is so that if anyone wants to run the code and they dont have like the sdl2 stuff then they an still run it as its now in an external folder
if(WIN32 AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2)
    set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2")
    message(STATUS "Windows detected: SDL2 found in palette project: ${SDL2_PATH}")

    set(SDL2_INCLUDE_DIR "${SDL2_PATH}" "${SDL2_PATH}/include")
    set(SDL2_LIBRARIES "${SDL2_PATH}/lib/x64/SDL2main.lib"
        "${SDL2_PATH}/lib/x64/SDL2.lib")

else()
    find_package(SDL2 CONFIG QUIET)
    if (SDL2_FOUND)
        set(SDL2_INCLUDE_DIR ${SDL2_INCLUDE_DIRS})
        set(SDL2_LIBRARIES SDL2::SDL2main SDL2::SDL2)
        message(STATUS "Linux fallback: SDL2 found using find_package")
    else()
        #fallback if native CMake modules not indexed
        find_package(PkgCongig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sd12)
        set (SDL2_INCLUDE_DIR ${SDL2_INCLUDE_DIRS})
        set(SDL2_LIBRARIES ${SDL2_LIBRARIES})
        message(STATUS "Linux: SDL2 found through PkgConfig fallback")
    endif()
endif()

#define project sources
set(projectSources
    src/main.cpp
    src/Color.cpp
    src/MedianCut.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
)

#define executable
add_executable(P2_GetYourPaletteHere ${projectSources})

#find and link sdl2
#find_package(SDL2 REQUIRED)
#target_link_libraries(P2_GetYourPaletteHere PRIVATE SDL2::SDL2 SDL2::SDL2main)

#link sdl2 - isa's version

target_link_libraries(P2_GetYourPaletteHere PRIVATE
    ${SDL2_LIBRARIES}
    OpenGL::GL
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
    ${OPENGL_INCLUDE_DIR}
)

if(WIN32 AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/SDL2)
    add_custom_command(TARGET P2_GetYourPaletteHere POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${SDL2_PATH}/lib/x64/SDL2.dll
            $<TARGET_FILE_DIR:P2_GetYourPaletteHere>)
endif()


