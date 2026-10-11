cmake_minimum_required(VERSION 3.21)

project(SFMLResearchDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(sfml-research-demo
    src/main.cpp
    src/GameObject.cpp
    src/Scene.cpp
)

target_include_directories(sfml-research-demo PRIVATE include)

target_link_libraries(sfml-research-demo PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)