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

void GameObject::setSize(sf::Vector2f size) {
    shape.setSize(size);
}

sf::Vector2f GameObject::getSize() const {
    return shape.getSize();
}

void GameObject::setColor(sf::Color color) {
    shape.setFillColor(color);
}

sf::Color GameObject::getColor() const {
    return shape.getFillColor();
}

sf::FloatRect GameObject::getBounds() const {
    return shape.getGlobalBounds();
}

void GameObject::setLayer(int layer) {
    zLayer = layer;
}

int GameObject::getLayer() const {
    return zLayer;
}

void GameObject::setSelected(bool value) {
    selected = value;

    if (selected) {
        shape.setOutlineThickness(3.f);
        shape.setOutlineColor(sf::Color::White);
    } else {
        shape.setOutlineThickness(0.f);
    }
}

bool GameObject::isSelected() const {
    return selected;
}