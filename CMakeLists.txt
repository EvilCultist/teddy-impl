cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(teddyimpl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW
find_package(glfw3 3.4 QUIET)
if (NOT glfw3_FOUND)
    FetchContent_Declare(
            glfw3
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
    )
    FetchContent_GetProperties(glfw3)
    if (NOT glfw3_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(glfw3)
        add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})
    endif()
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# GLAD
FetchContent_Declare(
    glad
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip
)

FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(glad)

    add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
    glad_add_library(glad REPRODUCIBLE EXCLUDE_FROM_ALL LOADER API gl:core=4.6)
endif()
set_target_properties(glad PROPERTIES FOLDER "Dependencies")

# GLM
find_package(glm 1.0.1 QUIET)
if (NOT glm_FOUND)
    FetchContent_Declare(
            glm
            DOWNLOAD_EXTRACT_TIMESTAMP OFF
            URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
    )
    FetchContent_GetProperties(glm)
    if (NOT glm_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(glm)
        add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
    endif()
endif()
# set_target_properties(glm PROPERTIES FOLDER "Dependencies")

# ImGui
find_package(imgui REQUIRED)

add_subdirectory(src)
