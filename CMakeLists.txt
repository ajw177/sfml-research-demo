cmake_minimum_required(VERSION 3.21)

project(SFMLResearchDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SFML 3
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

# Find Dear ImGui integration for SFML
find_package(ImGui-SFML CONFIG REQUIRED)

# Create executable
add_executable(sfml-research-demo
    src/main.cpp
    src/GameObject.cpp
    src/Scene.cpp
    src/Physics.cpp
)

# Project headers
target_include_directories(sfml-research-demo PRIVATE include)

# Link libraries
target_link_libraries(sfml-research-demo PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGui-SFML::ImGui-SFML
)