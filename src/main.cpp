#include <SFML/Graphics.hpp>

#include "GameObject.hpp"
#include "Scene.hpp"

#include <algorithm>
#include <numeric>
#include <vector>

// Check whether two rectangles overlap
bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

int main() {
    // Create the application window
    sf::RenderWindow window(
        sf::VideoMode({800, 600}),
        "Real Engine Research - Scene Saving"
    );

    window.setFramerateLimit(60);

    // Create the player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    const float speed = 5.f;

    // Create our scene manager
    Scene scene;

    // Store our game objects
    std::vector<GameObject> objects;

    // Red horizontal obstacle - Layer 1
    objects.emplace_back(
        sf::Vector2f{200.f, 150.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Red,
        1
    );

    // Blue vertical obstacle - Layer 2
    objects.emplace_back(
        sf::Vector2f{450.f, 250.f},
        sf::Vector2f{40.f, 200.f},
        sf::Color::Blue,
        2
    );

    // Magenta horizontal obstacle - Layer 3
    objects.emplace_back(
        sf::Vector2f{100.f, 450.f},
        sf::Vector2f{200.f, 40.f},
        sf::Color::Magenta,
        3
    );

    // Cyan rectangle - Layer 0
    objects.emplace_back(
        sf::Vector2f{300.f, 300.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Cyan,
        0
    );

    // Yellow rectangle - Layer 4
    objects.emplace_back(
        sf::Vector2f{350.f, 350.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Yellow,
        4
    );

    // Keep track of the yellow object
    const std::size_t selectedObject = 4;

    // Store drawing order separately
    std::vector<std::size_t> renderOrder(objects.size());

    // Main application loop
    while (window.isOpen()) {

        // Handle events
        while (const auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                // Close application
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                // Reset player
                if (key->code == sf::Keyboard::Key::R) {
                    player.setPosition({50.f, 50.f});
                }

                // Lower yellow object's Z-layer
                if (key->code == sf::Keyboard::Key::Q) {
                    int currentLayer =
                        objects[selectedObject].getLayer();

                    objects[selectedObject].setLayer(
                        currentLayer - 1
                    );
                }

                // Raise yellow object's Z-layer
                if (key->code == sf::Keyboard::Key::E) {
                    int currentLayer =
                        objects[selectedObject].getLayer();

                    objects[selectedObject].setLayer(
                        currentLayer + 1
                    );
                }

                // Save the scene using F5
                if (key->code == sf::Keyboard::Key::F5) {
                    scene.save(objects, "scenes/example.scene");
                }
            }
        }

        // Save player's previous position
        sf::Vector2f oldPosition = player.getPosition();
        sf::Vector2f position = oldPosition;

        // Keyboard movement
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

        // Keep player inside window
        position.x = std::clamp(position.x, 0.f, 760.f);
        position.y = std::clamp(position.y, 0.f, 560.f);

        player.setPosition(position);

        // Collision detection
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

        // Prepare drawing order
        std::iota(
            renderOrder.begin(),
            renderOrder.end(),
            0
        );

        // Sort objects by Z-layer
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