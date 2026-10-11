#include <SFML/Graphics.hpp>
#include "GameObject.hpp"

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
        "Real Engine Research - Z-Layer Rendering"
    );

    window.setFramerateLimit(60);

    // Create the player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    const float speed = 5.f;

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

    // Cyan background rectangle - Layer 0
    objects.emplace_back(
        sf::Vector2f{300.f, 300.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Cyan,
        0
    );

    // Yellow rectangle - Layer 4
    // This is the object whose layer we will change
    objects.emplace_back(
        sf::Vector2f{350.f, 350.f},
        sf::Vector2f{140.f, 140.f},
        sf::Color::Yellow,
        4
    );

    // Keep track of the yellow object using its index
    const std::size_t selectedObject = 4;

    // Store drawing order separately from the objects
    std::vector<std::size_t> renderOrder(objects.size());

    // Main game loop
    while (window.isOpen()) {

        // Handle window events
        while (const auto event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key =
                    event->getIf<sf::Event::KeyPressed>()) {

                // Exit the application
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                // Reset player position
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
            }
        }

        // Save the player's current position
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

        // Prevent the player from leaving the window
        position.x = std::clamp(position.x, 0.f, 760.f);
        position.y = std::clamp(position.y, 0.f, 560.f);

        // Temporarily move the player
        player.setPosition(position);

        // Check for collisions with game objects
        bool collided = false;

        for (const auto& object : objects) {
            if (checkCollision(
                    player.getGlobalBounds(),
                    object.getBounds())) {

                collided = true;
                break;
            }
        }

        // Respond to collisions
        if (collided) {
            player.setPosition(oldPosition);
            player.setFillColor(sf::Color::Yellow);
        } else {
            player.setFillColor(sf::Color::Green);
        }

        // Create an index list: 0, 1, 2, 3, 4
        std::iota(
            renderOrder.begin(),
            renderOrder.end(),
            0
        );

        // Sort indices by each object's Z-layer
        std::stable_sort(
            renderOrder.begin(),
            renderOrder.end(),
            [&objects](std::size_t a, std::size_t b) {
                return objects[a].getLayer() <
                       objects[b].getLayer();
            }
        );

        // Clear the window
        window.clear(sf::Color(30, 30, 40));

        // Draw objects from lowest layer to highest
        for (std::size_t index : renderOrder) {
            objects[index].draw(window);
        }

        // Draw player on top
        window.draw(player);

        // Display the completed frame
        window.display();
    }

    return 0;
}