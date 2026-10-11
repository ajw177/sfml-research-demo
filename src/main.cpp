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

    // Grid settings
    const float gridSize = 40.f;
    bool showGrid = true;
    bool snapToGrid = false;

    // Create grid lines (SFML 3 compatible)
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

    // Create the player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    // Movement and physics settings
    const float moveSpeed = 250.f;
    const float jumpStrength = -500.f;

    sf::Vector2f velocity{0.f, 0.f};

    Physics physics;
    sf::Clock clock;

    // Scene manager
    Scene scene;

    // Store game objects
    std::vector<GameObject> objects;

    // Red horizontal obstacle
    objects.emplace_back(
        sf::Vector2f{200.f, 150.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Red,
        1
    );

    // Blue vertical obstacle
    objects.emplace_back(
        sf::Vector2f{450.f, 250.f},
        sf::Vector2f{40.f, 200.f},
        sf::Color::Blue,
        2
    );

    // Magenta horizontal obstacle
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

    // Rendering order
    std::vector<std::size_t> renderOrder;

    // Selection and dragging
    int selectedObject = -1;
    bool dragging = false;

    sf::Vector2f dragOffset{0.f, 0.f};

    // Counter for new object colors
    int objectCounter = 0;

    // Update window title
    auto updateWindowTitle = [&]() {
        std::string title = "Real Engine Research | Grid: ";

        title += showGrid ? "ON" : "OFF";

        title += " | Snap: ";
        title += snapToGrid ? "ON" : "OFF";

        title += " | Gravity: ";
        title += physics.isGravityEnabled() ? "ON" : "OFF";

        return title;
    };

    window.setTitle(updateWindowTitle());

    // Main application loop
    while (window.isOpen()) {

        // Calculate delta time
        float deltaTime = clock.restart().asSeconds();

        // Limit unusually large time steps
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

                // Exit application
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                // Reset player
                if (key->code == sf::Keyboard::Key::R) {
                    player.setPosition({50.f, 50.f});
                    velocity = {0.f, 0.f};
                    physics.reset();
                }

                // Toggle grid visibility
                if (key->code == sf::Keyboard::Key::H) {
                    showGrid = !showGrid;

                    std::cout << "Grid visibility: "
                              << (showGrid ? "ON" : "OFF")
                              << std::endl;
                }

                // Toggle grid snapping
                if (key->code == sf::Keyboard::Key::T) {
                    snapToGrid = !snapToGrid;

                    std::cout << "Grid snapping: "
                              << (snapToGrid ? "ON" : "OFF")
                              << std::endl;
                }

                // Toggle gravity
                if (key->code == sf::Keyboard::Key::G) {
                    bool enabled = !physics.isGravityEnabled();

                    physics.setGravityEnabled(enabled);
                    velocity = {0.f, 0.f};

                    std::cout << "Gravity: "
                              << (enabled ? "ON" : "OFF")
                              << std::endl;
                }

                // Jump when grounded
                if (key->code == sf::Keyboard::Key::Space &&
                    physics.isGravityEnabled() &&
                    physics.isGrounded()) {

                    velocity.y = jumpStrength;
                }

                // Lower selected object's Z-layer
                if (key->code == sf::Keyboard::Key::Q &&
                    selectedObject >= 0) {

                    GameObject& object = objects[selectedObject];

                    object.setLayer(object.getLayer() - 1);
                }

                // Raise selected object's Z-layer
                if (key->code == sf::Keyboard::Key::E &&
                    selectedObject >= 0) {

                    GameObject& object = objects[selectedObject];

                    object.setLayer(object.getLayer() + 1);
                }

                // Create a new object
                if (key->code == sf::Keyboard::Key::N) {

                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(
                            sf::Mouse::getPosition(window)
                        );

                    // Snap new objects to the grid
                    if (snapToGrid) {
                        mousePosition =
                            snapPosition(mousePosition, gridSize);
                    }

                    // Available object colors
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

                    // Deselect previous object
                    for (auto& object : objects) {
                        object.setSelected(false);
                    }

                    // Create new rectangle
                    objects.emplace_back(
                        mousePosition,
                        sf::Vector2f{80.f, 80.f},
                        color,
                        5
                    );

                    // Select newly created object
                    selectedObject =
                        static_cast<int>(objects.size()) - 1;

                    objects[selectedObject].setSelected(true);

                    std::cout << "Created object at: "
                              << mousePosition.x << ", "
                              << mousePosition.y << std::endl;
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

                    std::cout << "Object deleted. Total: "
                              << objects.size() << std::endl;
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
                    }
                }
            }

            // Mouse selection
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseButtonPressed>()) {

                if (mouse->button == sf::Mouse::Button::Left) {

                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(mouse->position);

                    // Clear previous selection
                    for (auto& object : objects) {
                        object.setSelected(false);
                    }

                    selectedObject = -1;
                    dragging = false;

                    // Check objects from front to back
                    for (auto it = renderOrder.rbegin();
                         it != renderOrder.rend(); ++it) {

                        std::size_t index = *it;

                        if (objects[index].getBounds()
                                .contains(mousePosition)) {

                            selectedObject =
                                static_cast<int>(index);

                            objects[index].setSelected(true);

                            // Preserve cursor offset while dragging
                            dragOffset =
                                mousePosition -
                                objects[index].getPosition();

                            dragging = true;
                            break;
                        }
                    }
                }
            }

            // Stop dragging when mouse is released
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseButtonReleased>()) {

                if (mouse->button == sf::Mouse::Button::Left) {
                    dragging = false;
                }
            }

            // Move selected object with mouse
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseMoved>()) {

                if (dragging && selectedObject >= 0) {

                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(mouse->position);

                    sf::Vector2f newPosition =
                        mousePosition - dragOffset;

                    // Snap dragged objects when enabled
                    if (snapToGrid) {
                        newPosition =
                            snapPosition(newPosition, gridSize);
                    }

                    objects[selectedObject].setPosition(
                        newPosition
                    );
                }
            }
        }

        // Horizontal player movement
        velocity.x = 0.f;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {

            velocity.x -= moveSpeed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {

            velocity.x += moveSpeed;
        }

        // Free vertical movement when gravity is disabled
        if (!physics.isGravityEnabled()) {

            velocity.y = 0.f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {

                velocity.y -= moveSpeed;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {

                velocity.y += moveSpeed;
            }
        }

        // Move player horizontally
        sf::Vector2f oldPosition = player.getPosition();

        player.move({velocity.x * deltaTime, 0.f});

        // Check horizontal collisions
        bool horizontalCollision = false;

        for (const auto& object : objects) {

            if (checkCollision(
                    player.getGlobalBounds(),
                    object.getBounds())) {

                horizontalCollision = true;
                break;
            }
        }

        // Undo movement if collision occurred
        if (horizontalCollision) {
            player.setPosition(oldPosition);
        }

        // Apply vertical physics
        if (physics.isGravityEnabled()) {

            physics.update(
                player,
                velocity,
                objects,
                deltaTime
            );

        } else {

            // Free vertical movement without gravity
            sf::Vector2f beforeVertical =
                player.getPosition();

            player.move({0.f, velocity.y * deltaTime});

            for (const auto& object : objects) {

                if (checkCollision(
                        player.getGlobalBounds(),
                        object.getBounds())) {

                    player.setPosition(beforeVertical);
                    break;
                }
            }
        }

        // Keep player inside window
        sf::Vector2f position = player.getPosition();

        position.x = std::clamp(position.x, 0.f, 760.f);

        if (!physics.isGravityEnabled()) {
            position.y = std::clamp(position.y, 0.f, 560.f);
        }

        player.setPosition(position);

        // Prepare Z-layer rendering order
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

        // Update window title
        window.setTitle(updateWindowTitle());

        // Clear window
        window.clear(sf::Color(30, 30, 40));

        // Draw grid behind objects
        if (showGrid) {
            window.draw(grid);
        }

        // Draw objects in Z-layer order
        for (std::size_t index : renderOrder) {
            objects[index].draw(window);
        }

        // Draw player
        window.draw(player);

        // Display frame
        window.display();
    }

    return 0;
}