#include <SFML/Graphics.hpp>

#include "GameObject.hpp"
#include "Scene.hpp"
#include "Physics.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// Check whether two rectangles overlap
bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

// Round a coordinate to the nearest grid line
float snapValue(float value, float gridSize) {
    return std::round(value / gridSize) * gridSize;
}

// Snap a position to the grid
sf::Vector2f snapPosition(sf::Vector2f position, float gridSize) {
    return {
        snapValue(position.x, gridSize),
        snapValue(position.y, gridSize)
    };
}

int main() {
    // Create the application window
    sf::RenderWindow window(
        sf::VideoMode({800, 600}),
        "Real Engine Research - Scene Editor"
    );

    window.setFramerateLimit(60);

    // Editor mode
    bool playMode = false;

    // Grid settings
    const float gridSize = 40.f;
    bool showGrid = true;
    bool snapToGrid = false;

    // Create grid lines
    sf::VertexArray grid(sf::PrimitiveType::Lines);

    // Vertical grid lines
    for (float x = 0.f; x <= 800.f; x += gridSize) {
        sf::Vertex top;
        top.position = {x, 0.f};
        top.color = sf::Color(65, 65, 75);

        sf::Vertex bottom;
        bottom.position = {x, 600.f};
        bottom.color = sf::Color(65, 65, 75);

        grid.append(top);
        grid.append(bottom);
    }

    // Horizontal grid lines
    for (float y = 0.f; y <= 600.f; y += gridSize) {
        sf::Vertex left;
        left.position = {0.f, y};
        left.color = sf::Color(65, 65, 75);

        sf::Vertex right;
        right.position = {800.f, y};
        right.color = sf::Color(65, 65, 75);

        grid.append(left);
        grid.append(right);
    }

    // Create player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    const sf::Vector2f playerStart{50.f, 50.f};

    // Physics settings
    const float moveSpeed = 250.f;
    const float jumpStrength = -500.f;

    sf::Vector2f velocity{0.f, 0.f};

    Physics physics;
    sf::Clock clock;

    // Scene manager
    Scene scene;

    // Store game objects
    std::vector<GameObject> objects;

    // Red platform
    objects.emplace_back(
        sf::Vector2f{200.f, 150.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Red,
        1
    );

    // Blue platform
    objects.emplace_back(
        sf::Vector2f{450.f, 250.f},
        sf::Vector2f{40.f, 200.f},
        sf::Color::Blue,
        2
    );

    // Magenta platform
    objects.emplace_back(
        sf::Vector2f{100.f, 450.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Magenta,
        3
    );

    // Cyan rectangle
    objects.emplace_back(
        sf::Vector2f{300.f, 300.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Cyan,
        0
    );

    // Yellow rectangle
    objects.emplace_back(
        sf::Vector2f{350.f, 350.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Yellow,
        4
    );

    // Backup of scene before entering Play Mode
    std::vector<GameObject> sceneBackup;

    // Rendering order
    std::vector<std::size_t> renderOrder;

    // Object selection
    int selectedObject = -1;
    bool dragging = false;

    sf::Vector2f dragOffset{0.f, 0.f};

    int objectCounter = 0;

    // Update the window title
    auto updateWindowTitle = [&]() {
        std::string title = "Real Engine | Mode: ";

        title += playMode ? "PLAY" : "EDIT";

        title += " | Grid: ";
        title += (showGrid && !playMode) ? "ON" : "OFF";

        title += " | Snap: ";
        title += snapToGrid ? "ON" : "OFF";

        title += " | Gravity: ";
        title += physics.isGravityEnabled() ? "ON" : "OFF";

        return title;
    };

    // Update the order in which objects are drawn
    auto updateRenderOrder = [&]() {
        renderOrder.resize(objects.size());

        std::iota(
            renderOrder.begin(),
            renderOrder.end(),
            0
        );

        std::stable_sort(
            renderOrder.begin(),
            renderOrder.end(),
            [&objects](std::size_t a, std::size_t b) {
                return objects[a].getLayer() <
                       objects[b].getLayer();
            }
        );
    };

    // Reset player and physics state
    auto resetPlayer = [&]() {
        player.setPosition(playerStart);
        velocity = {0.f, 0.f};
        physics.reset();
    };

    // Switch between Edit and Play modes
    auto toggleMode = [&]() {
        if (!playMode) {
            // Enter Play Mode
            sceneBackup = objects;

            // Remove editor selection
            for (auto& object : objects) {
                object.setSelected(false);
            }

            selectedObject = -1;
            dragging = false;

            resetPlayer();

            physics.setGravityEnabled(true);
            playMode = true;

            std::cout << "Entered PLAY mode" << std::endl;
        } else {
            // Return to Edit Mode
            objects = sceneBackup;

            selectedObject = -1;
            dragging = false;

            resetPlayer();

            physics.setGravityEnabled(false);
            playMode = false;

            std::cout << "Returned to EDIT mode" << std::endl;
            std::cout << "Scene restored" << std::endl;
        }

        updateRenderOrder();
        window.setTitle(updateWindowTitle());
    };

    updateRenderOrder();
    window.setTitle(updateWindowTitle());

    // Main application loop
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        deltaTime = std::min(deltaTime, 0.033f);

        // Handle events
        while (const auto event = window.pollEvent()) {

            // Close window
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Keyboard input
            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                // Exit
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                // Switch between Edit and Play modes
                if (key->code == sf::Keyboard::Key::Tab) {
                    toggleMode();
                    continue;
                }

                // Play Mode controls
                if (playMode) {

                    // Reset player
                    if (key->code == sf::Keyboard::Key::R) {
                        resetPlayer();
                    }

                    // Jump
                    if (key->code == sf::Keyboard::Key::Space &&
                        physics.isGrounded()) {

                        velocity.y = jumpStrength;
                    }

                    // Skip editor-only keyboard controls
                    continue;
                }

                // EDIT MODE CONTROLS

                // Toggle grid
                if (key->code == sf::Keyboard::Key::H) {
                    showGrid = !showGrid;

                    std::cout << "Grid: "
                              << (showGrid ? "ON" : "OFF")
                              << std::endl;
                }

                // Toggle snapping
                if (key->code == sf::Keyboard::Key::T) {
                    snapToGrid = !snapToGrid;

                    std::cout << "Snapping: "
                              << (snapToGrid ? "ON" : "OFF")
                              << std::endl;
                }

                // Lower selected object's layer
                if (key->code == sf::Keyboard::Key::Q &&
                    selectedObject >= 0) {

                    GameObject& object = objects[selectedObject];
                    object.setLayer(object.getLayer() - 1);

                    updateRenderOrder();
                }

                // Raise selected object's layer
                if (key->code == sf::Keyboard::Key::E &&
                    selectedObject >= 0) {

                    GameObject& object = objects[selectedObject];
                    object.setLayer(object.getLayer() + 1);

                    updateRenderOrder();
                }

                // Create a new object
                if (key->code == sf::Keyboard::Key::N) {

                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(
                            sf::Mouse::getPosition(window)
                        );

                    if (snapToGrid) {
                        mousePosition =
                            snapPosition(mousePosition, gridSize);
                    }

                    sf::Color colors[] = {
                        sf::Color::Green,
                        sf::Color::Blue,
                        sf::Color::Magenta,
                        sf::Color::Cyan,
                        sf::Color::Yellow,
                        sf::Color::Red
                    };

                    sf::Color color = colors[objectCounter % 6];
                    objectCounter++;

                    for (auto& object : objects) {
                        object.setSelected(false);
                    }

                    objects.emplace_back(
                        mousePosition,
                        sf::Vector2f{80.f, 80.f},
                        color,
                        5
                    );

                    selectedObject =
                        static_cast<int>(objects.size()) - 1;

                    objects[selectedObject].setSelected(true);

                    updateRenderOrder();

                    std::cout << "Object created" << std::endl;
                }

                // Delete selected object
                if ((key->code == sf::Keyboard::Key::Delete ||
                     key->code == sf::Keyboard::Key::Backspace) &&
                    selectedObject >= 0) {

                    objects.erase(
                        objects.begin() + selectedObject
                    );

                    selectedObject = -1;
                    dragging = false;

                    updateRenderOrder();

                    std::cout << "Object deleted" << std::endl;
                }

                // Save scene
                if (key->code == sf::Keyboard::Key::F5) {
                    scene.save(objects, "scenes/example.scene");
                }

                // Load scene
                if (key->code == sf::Keyboard::Key::F9) {
                    if (scene.load(objects, "scenes/example.scene")) {
                        selectedObject = -1;
                        dragging = false;

                        updateRenderOrder();
                    }
                }
            }

            // Mouse input is only active in Edit Mode
            if (playMode) {
                continue;
            }

            // Select objects
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseButtonPressed>()) {

                if (mouse->button == sf::Mouse::Button::Left) {

                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(mouse->position);

                    for (auto& object : objects) {
                        object.setSelected(false);
                    }

                    selectedObject = -1;
                    dragging = false;

                    // Select topmost object
                    for (auto it = renderOrder.rbegin();
                         it != renderOrder.rend(); ++it) {

                        std::size_t index = *it;

                        if (objects[index].getBounds()
                                .contains(mousePosition)) {

                            selectedObject =
                                static_cast<int>(index);

                            objects[index].setSelected(true);

                            dragOffset =
                                mousePosition -
                                objects[index].getPosition();

                            dragging = true;
                            break;
                        }
                    }
                }
            }

            // Stop dragging
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseButtonReleased>()) {

                if (mouse->button == sf::Mouse::Button::Left) {
                    dragging = false;
                }
            }

            // Drag selected object
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseMoved>()) {

                if (dragging && selectedObject >= 0) {

                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(mouse->position);

                    sf::Vector2f newPosition =
                        mousePosition - dragOffset;

                    if (snapToGrid) {
                        newPosition =
                            snapPosition(newPosition, gridSize);
                    }

                    objects[selectedObject].setPosition(newPosition);
                }
            }
        }

        // Only run player movement and physics in Play Mode
        if (playMode) {

            velocity.x = 0.f;

            // Move left
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {

                velocity.x -= moveSpeed;
            }

            // Move right
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {

                velocity.x += moveSpeed;
            }

            // Horizontal movement
            sf::Vector2f oldPosition = player.getPosition();

            player.move({velocity.x * deltaTime, 0.f});

            bool horizontalCollision = false;

            for (const auto& object : objects) {
                if (checkCollision(
                        player.getGlobalBounds(),
                        object.getBounds())) {

                    horizontalCollision = true;
                    break;
                }
            }

            if (horizontalCollision) {
                player.setPosition(oldPosition);
            }

            // Gravity and vertical collisions
            physics.update(
                player,
                velocity,
                objects,
                deltaTime
            );

            // Keep player inside window horizontally
            sf::Vector2f position = player.getPosition();

            position.x = std::clamp(position.x, 0.f, 760.f);

            player.setPosition(position);
        }

        // Update rendering order
        updateRenderOrder();

        // Update title
        window.setTitle(updateWindowTitle());

        // Clear screen
        window.clear(sf::Color(30, 30, 40));

        // Grid is visible only in Edit Mode
        if (!playMode && showGrid) {
            window.draw(grid);
        }

        // Draw scene objects
        for (std::size_t index : renderOrder) {
            objects[index].draw(window);
        }

        // Draw player only in Play Mode
        if (playMode) {
            window.draw(player);
        }

        window.display();
    }

    return 0;
}