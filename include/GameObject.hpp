#pragma once

#include <SFML/Graphics.hpp>

class GameObject {
public:
    GameObject(sf::Vector2f position, sf::Vector2f size,
               sf::Color color, int layer);

    void draw(sf::RenderWindow& window) const;

    void setPosition(sf::Vector2f position);
    sf::Vector2f getPosition() const;

    sf::Vector2f getSize() const;

    void setColor(sf::Color color);
    sf::Color getColor() const;

    sf::FloatRect getBounds() const;

    void setLayer(int layer);
    int getLayer() const;

private:
    sf::RectangleShape shape;
    int zLayer;
};