#include "GameObject.hpp"

GameObject::GameObject(sf::Vector2f position, sf::Vector2f size,
                       sf::Color color, int layer) {
    shape.setPosition(position);
    shape.setSize(size);
    shape.setFillColor(color);
    zLayer = layer;
}

void GameObject::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

void GameObject::setPosition(sf::Vector2f position) {
    shape.setPosition(position);
}

sf::Vector2f GameObject::getPosition() const {
    return shape.getPosition();
}

void GameObject::setLayer(int layer) {
    zLayer = layer;
}

int GameObject::getLayer() const {
    return zLayer;
}