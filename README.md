# SFML Research Demo

CSCE 490 – Capstone Research Milestone
Video Link: https://youtu.be/SlZZXaf1XgI

## Project Overview

This project is a small interactive 2D application developed in C++ using SFML 3. It was created as part of my research for our capstone project, Real Engine, a custom 2D game engine written in C++.

The purpose of this project was to become familiar with SFML, experiment with its rendering and input systems, and understand how to manage external libraries using CMake and vcpkg.

## Technologies Used

### C++
The main programming language used for the application. C++ gives developers control over performance and memory management, making it a common choice for game engines.

### SFML 3
Simple and Fast Multimedia Library (SFML) is a multimedia library that provides functionality for graphics, window management, input handling, audio, and networking.

For this project, I used SFML to create a window, draw objects, detect keyboard input, and check collisions between rectangles.

**Advantages:**
- Relatively easy to learn compared to lower-level graphics APIs.
- Provides built-in window management and input handling.
- Supports multiple operating systems.
- Useful for developing 2D applications and games.

**Disadvantages:**
- Does not provide a complete game engine.
- More advanced systems, such as physics and entity management, must be implemented separately.
- Less control over low-level rendering than APIs such as OpenGL or Vulkan.

### CMake
CMake was used to configure and build the C++ application.

It allows the project to define its source files, compiler requirements, and external dependencies without relying on IDE-specific project files.

### vcpkg
vcpkg is a C++ package manager developed by Microsoft.

I used vcpkg to install SFML 3 and its dependencies and integrate them with CMake.

This simplified the setup process because I did not have to manually download, compile, and link each library.

## Application Features

The application demonstrates several basic concepts relevant to 2D game engine development.

- **Rendering:** Draws a player and three obstacles.
- **Keyboard Input:** Uses WASD or arrow keys to move the player.
- **Collision Detection:** Detects overlapping rectangles using SFML's bounding box functionality.
- **Collision Response:** Prevents the player from moving through obstacles.
- **Window Boundaries:** Keeps the player inside the application window.
- **Visual Feedback:** Changes the player's color when a collision occurs.
- **Reset:** Returns the player to its starting position.

## Controls

| Key | Action |
|-----|--------|
| W / Up Arrow | Move up |
| A / Left Arrow | Move left |
| S / Down Arrow | Move down |
| D / Right Arrow | Move right |
| R | Reset player position |
| Escape | Close application |

## Building and Running

### Requirements

- C++17-compatible compiler
- CMake 3.21 or newer
- vcpkg
- SFML 3 (installed automatically through vcpkg)

### Build Instructions

Clone the repository:

```bash
git clone https://github.com/ajw177/sfml-research-demo.git
cd sfml-research-demo
```

Configure the project using CMake and vcpkg:

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

Build the application:

```bash
cmake --build build
```

Run the application:

```bash
./build/sfml-research-demo
```

These commands were tested on macOS with Apple Silicon. The executable location may differ on other platforms.

## Research Findings

One of the biggest things I learned was how different tools work together in a C++ project.

Before this research, I had limited experience setting up external C++ libraries. Using SFML, CMake, and vcpkg helped me better understand the development process.

SFML handles many of the lower-level tasks involved in creating a graphical application, such as opening windows, rendering shapes, and reading keyboard input. This allows developers to focus on implementing their own application logic.

I also learned how collision detection can be implemented using rectangular bounding boxes. SFML provides the functionality to detect intersections, but the application must decide how to respond to them.

CMake and vcpkg were especially useful because they made dependency management and compilation more organized.

## Relevance to Real Engine

Our capstone project, Real Engine, is a custom 2D game engine written in C++.

This research helped me understand several systems that will be important during development, including:

- Rendering and drawing game objects.
- Handling keyboard input.
- Updating object positions.
- Detecting collisions.
- Organizing a C++ project using CMake.
- Managing external libraries using vcpkg.

SFML could provide useful functionality for window management, input, audio, and rendering while allowing our team to implement custom engine systems.

## Future Improvements

Possible improvements include:

- Frame-rate-independent movement using delta time.
- Improved collision response that allows sliding along obstacles.
- Additional game objects and components.
- Basic physics and animation.
- Integration with a larger game engine architecture.
