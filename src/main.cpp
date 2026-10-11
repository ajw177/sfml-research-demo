#include <SFML/Graphics.hpp>

#include "GameObject.hpp"
#include "Scene.hpp"

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
        "Real Engine Research - Interactive Scene Editor"
    );

    window.setFramerateLimit(60);

    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    const float speed = 5.f;

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

    std::vector<std::size_t> renderOrder;

    int selectedObject = -1;
    bool dragging = false;

    sf::Vector2f dragOffset{0.f, 0.f};

    // Used to give newly created objects different colors
    int objectCounter = 0;

    while (window.isOpen()) {

        while (const auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                if (key->code == sf::Keyboard::Key::R) {
                    player.setPosition({50.f, 50.f});
                }

                // Lower selected object's layer
                if (key->code == sf::Keyboard::Key::Q &&
                    selectedObject >= 0) {

                    GameObject& object = objects[selectedObject];
                    object.setLayer(object.getLayer() - 1);
                }

                // Raise selected object's layer
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

                    objects.emplace_back(
                        mousePosition,
                        sf::Vector2f{80.f, 80.f},
                        color,
                        5
                    );

                    selectedObject =
                        static_cast<int>(objects.size()) - 1;

                    objects[selectedObject].setSelected(true);

                    std::cout << "Created new object. Total: "
                              << objects.size() << std::endl;
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

        // Player movement
        sf::Vector2f oldPosition = player.getPosition();
        sf::Vector2f position = oldPosition;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            position.y -= speed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            position.y += speed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            position.x -= speed;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            position.x += speed;
        }

        position.x = std::clamp(position.x, 0.f, 760.f);
        position.y = std::clamp(position.y, 0.f, 560.f);

        player.setPosition(position);

        bool collided = false;

        for (const auto& object : objects) {
            if (checkCollision(
                    player.getGlobalBounds(),
                    object.getBounds())) {

                collided = true;
                break;
            }
        }

        if (collided) {
            player.setPosition(oldPosition);
            player.setFillColor(sf::Color::Yellow);
        } else {
            player.setFillColor(sf::Color::Green);
        }

        // Sort rendering order
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

        window.clear(sf::Color(30, 30, 40));

        for (std::size_t index : renderOrder) {
            objects[index].draw(window);
        }

        window.draw(player);
        window.display();
    }

    return 0;
}