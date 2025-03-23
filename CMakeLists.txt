cmake_minimum_required(VERSION 3.22)
project(LowLevelRenderer VERSION 0.1.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# macOS specific settings
set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15)

# Find required packages
find_package(OpenGL REQUIRED)

# Configure GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLM is header-only
add_subdirectory(external/glm)

# STB is header-only, include it directly
include_directories(external/stb)

# Add include directories
include_directories(include)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/core/*.cpp"
    "src/graphics/*.cpp"
    "src/scene/*.cpp"
    "src/app/*.cpp"
)

# Main executable
add_executable(LLR src/main.cpp ${SOURCES})

# Link libraries
target_link_libraries(LLR PRIVATE
    OpenGL::GL
    glfw
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreFoundation"
)

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
