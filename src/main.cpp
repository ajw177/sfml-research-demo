#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>

// Check if two rectangles overlap
bool checkCollision(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.findIntersection(b).has_value();
}

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML Collision Demo");
    window.setFramerateLimit(60);

    // Create player
    sf::RectangleShape player({40.f, 40.f});
    player.setFillColor(sf::Color::Green);
    player.setPosition({50.f, 50.f});

    // Create obstacles
    std::vector<sf::RectangleShape> obstacles;

    sf::RectangleShape obstacle1({200.f, 40.f});
    obstacle1.setPosition({200.f, 150.f});
    obstacle1.setFillColor(sf::Color::Red);
    obstacles.push_back(obstacle1);

    sf::RectangleShape obstacle2({40.f, 200.f});
    obstacle2.setPosition({450.f, 250.f});
    obstacle2.setFillColor(sf::Color::Red);
    obstacles.push_back(obstacle2);

    sf::RectangleShape obstacle3({200.f, 40.f});
    obstacle3.setPosition({100.f, 450.f});
    obstacle3.setFillColor(sf::Color::Red);
    obstacles.push_back(obstacle3);

    const float speed = 5.f;

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }

                // Reset player position
                if (key->code == sf::Keyboard::Key::R) {
                    player.setPosition({50.f, 50.f});
                }
            }
        }

        // Calculate proposed movement
        sf::Vector2f position = player.getPosition();

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

        // Keep player inside window boundaries
        position.x = std::clamp(position.x, 0.f, 760.f);
        position.y = std::clamp(position.y, 0.f, 560.f);

        // Check collision before moving
        sf::Vector2f oldPosition = player.getPosition();
        player.setPosition(position);

        bool collided = false;

        for (const auto& obstacle : obstacles) {
            if (checkCollision(player.getGlobalBounds(), obstacle.getGlobalBounds())) {
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

        // Render everything
        window.clear(sf::Color(30, 30, 40));

        for (const auto& obstacle : obstacles) {
            window.draw(obstacle);
        }

        window.draw(player);
        window.display();
    }

    return 0;
}