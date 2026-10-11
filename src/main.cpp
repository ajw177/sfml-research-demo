#include <SFML/Graphics.hpp>

int main() {
    // Create the game window
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML Research Demo");
    window.setFramerateLimit(60);

    // Create the player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({380.f, 280.f});

    // Movement speed
    const float speed = 5.f;

    // Main game loop
    while (window.isOpen()) {

        // Handle window events
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }
        }

        // Get current player position
        sf::Vector2f position = player.getPosition();

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

        // Update player position
        player.setPosition(position);

        // Draw everything
        window.clear(sf::Color(30, 30, 40));
        window.draw(player);
        window.display();
    }

    return 0;
}