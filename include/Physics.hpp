#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "GameObject.hpp"

class Physics {
public:
    Physics();

    void update(
        sf::RectangleShape& player,
        sf::Vector2f& velocity,
        const std::vector<GameObject>& objects,
        float deltaTime
    );

    void setGravityEnabled(bool enabled);
    bool isGravityEnabled() const;

    bool isGrounded() const;

    void reset();

private:
    float gravity;
    float terminalVelocity;

    bool gravityEnabled;
    bool grounded;
};