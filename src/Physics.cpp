#include "Physics.hpp"

#include <algorithm>

Physics::Physics() {
    gravity = 1000.f;
    terminalVelocity = 700.f;

    gravityEnabled = false;
    grounded = false;
}

void Physics::setGravityEnabled(bool enabled) {
    gravityEnabled = enabled;
    grounded = false;
}

bool Physics::isGravityEnabled() const {
    return gravityEnabled;
}

bool Physics::isGrounded() const {
    return grounded;
}

void Physics::reset() {
    grounded = false;
}

void Physics::update(
    sf::RectangleShape& player,
    sf::Vector2f& velocity,
    const std::vector<GameObject>& objects,
    float deltaTime
) {
    if (!gravityEnabled) {
        return;
    }

    // Apply gravity to vertical velocity
    velocity.y += gravity * deltaTime;

    // Limit maximum falling speed
    velocity.y = std::min(velocity.y, terminalVelocity);

    // Remember the player's previous position
    sf::Vector2f previousPosition = player.getPosition();

    // Move player vertically
    player.move({0.f, velocity.y * deltaTime});

    grounded = false;

    // Check collisions with scene objects
    for (const auto& object : objects) {

        sf::FloatRect playerBounds = player.getGlobalBounds();
        sf::FloatRect objectBounds = object.getBounds();

        if (!playerBounds.findIntersection(objectBounds)) {
            continue;
        }

        // Calculate previous and current bottom positions
        float previousBottom =
            previousPosition.y + player.getSize().y;

        float objectTop = objectBounds.position.y;

        // Landing on top of an object
        if (velocity.y >= 0.f &&
            previousBottom <= objectTop + 1.f) {

            player.setPosition({
                player.getPosition().x,
                objectTop - player.getSize().y
            });

            velocity.y = 0.f;
            grounded = true;
        }

        // Hitting the underside of an object
        else if (velocity.y < 0.f) {

            float objectBottom =
                objectBounds.position.y + objectBounds.size.y;

            player.setPosition({
                player.getPosition().x,
                objectBottom
            });

            velocity.y = 0.f;
        }
    }

    // Ground collision at bottom of window
    float groundY = 600.f - player.getSize().y;

    if (player.getPosition().y >= groundY) {

        player.setPosition({
            player.getPosition().x,
            groundY
        });

        velocity.y = 0.f;
        grounded = true;
    }

    // Prevent leaving the top of the window
    if (player.getPosition().y < 0.f) {

        player.setPosition({
            player.getPosition().x,
            0.f
        });

        if (velocity.y < 0.f) {
            velocity.y = 0.f;
        }
    }
}