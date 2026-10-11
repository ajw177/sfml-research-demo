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

bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

float snapValue(float value, float gridSize) {
    return std::round(value / gridSize) * gridSize;
}

sf::Vector2f snapPosition(sf::Vector2f position, float gridSize) {
    return {
        snapValue(position.x, gridSize),
        snapValue(position.y, gridSize)
    };
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode({800, 600}),
        "Real Engine Research - Scene Editor"
    );

    window.setFramerateLimit(60);

    bool playMode = false;

    const float gridSize = 40.f;
    bool showGrid = true;
    bool snapToGrid = false;

    sf::VertexArray grid(sf::PrimitiveType::Lines);

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

    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    const sf::Vector2f playerStart{50.f, 50.f};

    const float moveSpeed = 250.f;
    const float jumpStrength = -500.f;

    sf::Vector2f velocity{0.f, 0.f};

    Physics physics;
    sf::Clock clock;
    Scene scene;

    std::vector<GameObject> objects;

    objects.emplace_back(
        sf::Vector2f{200.f, 150.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Red, 1
    );

    objects.emplace_back(
        sf::Vector2f{450.f, 250.f},
        sf::Vector2f{40.f, 200.f},
        sf::Color::Blue, 2
    );

    objects.emplace_back(
        sf::Vector2f{100.f, 450.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Magenta, 3
    );

    objects.emplace_back(
        sf::Vector2f{300.f, 300.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Cyan, 0
    );

    objects.emplace_back(
        sf::Vector2f{350.f, 350.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Yellow, 4
    );

    std::vector<GameObject> sceneBackup;
    std::vector<std::size_t> renderOrder;

    int selectedObject = -1;
    bool dragging = false;

    sf::Vector2f dragOffset{0.f, 0.f};

    int objectCounter = 0;

    auto updateRenderOrder = [&]() {
        renderOrder.resize(objects.size());

        std::iota(renderOrder.begin(), renderOrder.end(), 0);

        std::stable_sort(
            renderOrder.begin(),
            renderOrder.end(),
            [&objects](std::size_t a, std::size_t b) {
                return objects[a].getLayer() <
                       objects[b].getLayer();
            }
        );
    };

    auto updateWindowTitle = [&]() {
        std::string title = "Real Engine | ";

        title += playMode ? "PLAY" : "EDIT";

        if (!playMode && selectedObject >= 0 &&
            selectedObject < static_cast<int>(objects.size())) {

            const GameObject& object = objects[selectedObject];

            sf::Vector2f position = object.getPosition();
            sf::Vector2f size = object.getSize();

            title += " | Object " + std::to_string(selectedObject);
            title += " | Pos: (" +
                     std::to_string(static_cast<int>(position.x)) +
                     ", " +
                     std::to_string(static_cast<int>(position.y)) + ")";

            title += " | Size: (" +
                     std::to_string(static_cast<int>(size.x)) +
                     ", " +
                     std::to_string(static_cast<int>(size.y)) + ")";

            title += " | Layer: " +
                     std::to_string(object.getLayer());
        }

        title += " | Snap: ";
        title += snapToGrid ? "ON" : "OFF";

        return title;
    };

    auto resetPlayer = [&]() {
        player.setPosition(playerStart);
        velocity = {0.f, 0.f};
        physics.reset();
    };

    auto toggleMode = [&]() {
        if (!playMode) {
            sceneBackup = objects;

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
            objects = sceneBackup;

            selectedObject = -1;
            dragging = false;

            resetPlayer();
            physics.setGravityEnabled(false);

            playMode = false;

            std::cout << "Returned to EDIT mode" << std::endl;
        }

        updateRenderOrder();
    };

    updateRenderOrder();

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        deltaTime = std::min(deltaTime, 0.033f);

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                if (key->code == sf::Keyboard::Key::Tab) {
                    toggleMode();
                    continue;
                }

                if (playMode) {
                    if (key->code == sf::Keyboard::Key::R) {
                        resetPlayer();
                    }

                    if (key->code == sf::Keyboard::Key::Space &&
                        physics.isGrounded()) {
                        velocity.y = jumpStrength;
                    }

                    continue;
                }

                // Edit mode: grid visibility
                if (key->code == sf::Keyboard::Key::H) {
                    showGrid = !showGrid;
                }

                // Edit mode: grid snapping
                if (key->code == sf::Keyboard::Key::T) {
                    snapToGrid = !snapToGrid;
                }

                // Edit mode: selected object controls
                if (selectedObject >= 0 &&
                    selectedObject < static_cast<int>(objects.size())) {

                    GameObject& object = objects[selectedObject];
                    sf::Vector2f size = object.getSize();

                    // Change Z-layer
                    if (key->code == sf::Keyboard::Key::Q) {
                        object.setLayer(object.getLayer() - 1);
                    }

                    if (key->code == sf::Keyboard::Key::E) {
                        object.setLayer(object.getLayer() + 1);
                    }

                    // Decrease width
                    if (key->code == sf::Keyboard::Key::LBracket) {
                        size.x = std::max(20.f, size.x - 10.f);
                        object.setSize(size);
                    }

                    // Increase width
                    if (key->code == sf::Keyboard::Key::RBracket) {
                        size.x += 10.f;
                        object.setSize(size);
                    }

                    // Decrease height
                    if (key->code == sf::Keyboard::Key::Hyphen) {
                        size.y = std::max(20.f, size.y - 10.f);
                        object.setSize(size);
                    }

                    // Increase height
                    if (key->code == sf::Keyboard::Key::Equal) {
                        size.y += 10.f;
                        object.setSize(size);
                    }
                }

                // Create object
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
                }

                // Delete object
                if ((key->code == sf::Keyboard::Key::Delete ||
                     key->code == sf::Keyboard::Key::Backspace) &&
                    selectedObject >= 0) {

                    objects.erase(objects.begin() + selectedObject);

                    selectedObject = -1;
                    dragging = false;
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

                updateRenderOrder();
            }

            if (playMode) {
                continue;
            }

            // Mouse selection
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

            // Mouse release
            if (const auto* mouse =
                    event->getIf<sf::Event::MouseButtonReleased>()) {

                if (mouse->button == sf::Mouse::Button::Left) {
                    dragging = false;
                }
            }

            // Mouse dragging
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

        // Run physics only in Play Mode
        if (playMode) {
            velocity.x = 0.f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                velocity.x -= moveSpeed;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                velocity.x += moveSpeed;
            }

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

            physics.update(player, velocity, objects, deltaTime);

            sf::Vector2f position = player.getPosition();
            position.x = std::clamp(position.x, 0.f, 760.f);
            player.setPosition(position);
        }

        updateRenderOrder();
        window.setTitle(updateWindowTitle());

        window.clear(sf::Color(30, 30, 40));

        if (!playMode && showGrid) {
            window.draw(grid);
        }

        for (std::size_t index : renderOrder) {
            objects[index].draw(window);
        }

        if (playMode) {
            window.draw(player);
        }

        window.display();
    }

    return 0;
}