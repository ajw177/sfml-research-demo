#include <SFML/Graphics.hpp>

#include "GameObject.hpp"
#include "Scene.hpp"
#include "Physics.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode({800, 600}),
        "Real Engine Research - Physics Demo"
    );

    window.setFramerateLimit(60);

    // Player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    // Movement and physics
    const float moveSpeed = 250.f;
    const float jumpStrength = -500.f;

    sf::Vector2f velocity{0.f, 0.f};

    Physics physics;
    sf::Clock clock;

    // Scene manager
    Scene scene;

    // Game objects
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

    std::vector<std::size_t> renderOrder;

    int selectedObject = -1;
    bool dragging = false;

    sf::Vector2f dragOffset{0.f, 0.f};

    int objectCounter = 0;

    while (window.isOpen()) {

        // Delta time
        float deltaTime = clock.restart().asSeconds();

        // Prevent unusually large physics steps
        deltaTime = std::min(deltaTime, 0.033f);

        // Events
        while (const auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                // Reset player
                if (key->code == sf::Keyboard::Key::R) {
                    player.setPosition({50.f, 50.f});
                    velocity = {0.f, 0.f};
                    physics.reset();
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

                // Jump
                if (key->code == sf::Keyboard::Key::Space &&
                    physics.isGravityEnabled() &&
                    physics.isGrounded()) {

                    velocity.y = jumpStrength;
                }

                // Change Z-layer
                if (key->code == sf::Keyboard::Key::Q &&
                    selectedObject >= 0) {

                    GameObject& object = objects[selectedObject];
                    object.setLayer(object.getLayer() - 1);
                }

                if (key->code == sf::Keyboard::Key::E &&
                    selectedObject >= 0) {

                    GameObject& object = objects[selectedObject];
                    object.setLayer(object.getLayer() + 1);
                }

                // Create object
                if (key->code == sf::Keyboard::Key::N) {

                    sf::Vector2f mousePosition =
                        window.mapPixelToCoords(
                            sf::Mouse::getPosition(window)
                        );

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

                    objects.erase(
                        objects.begin() + selectedObject
                    );

                    selectedObject = -1;
                    dragging = false;
                }

                // Save
                if (key->code == sf::Keyboard::Key::F5) {
                    scene.save(objects, "scenes/example.scene");
                }

                // Load
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

                            selectedObject = static_cast<int>(index);

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

                    objects[selectedObject].setPosition(
                        mousePosition - dragOffset
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

        // Move horizontally
        sf::Vector2f oldPosition = player.getPosition();

        player.move({velocity.x * deltaTime, 0.f});

        // Horizontal collision detection
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

        // Apply gravity and vertical collisions
        if (physics.isGravityEnabled()) {
            physics.update(player, velocity, objects, deltaTime);
        } else {
            // Free movement without gravity
            sf::Vector2f beforeVertical = player.getPosition();

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

        // Keep player inside window horizontally
        sf::Vector2f position = player.getPosition();

        position.x = std::clamp(position.x, 0.f, 760.f);

        if (!physics.isGravityEnabled()) {
            position.y = std::clamp(position.y, 0.f, 560.f);
        }

        player.setPosition(position);

        // Rendering order
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

        // Render
        window.clear(sf::Color(30, 30, 40));

        for (std::size_t index : renderOrder) {
            objects[index].draw(window);
        }

        window.draw(player);
        window.display();
    }

    return 0;
}