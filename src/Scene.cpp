#include "Scene.hpp"

#include <fstream>
#include <iostream>

bool Scene::save(const std::vector<GameObject>& objects,
                 const std::string& filename) const {

    // Open the file for writing
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open scene file: "
                  << filename << std::endl;

        return false;
    }

    // Save each game object
    for (const auto& object : objects) {

        sf::Vector2f position = object.getPosition();
        sf::Vector2f size = object.getSize();
        sf::Color color = object.getColor();

        file << position.x << " "
             << position.y << " "
             << size.x << " "
             << size.y << " "
             << static_cast<int>(color.r) << " "
             << static_cast<int>(color.g) << " "
             << static_cast<int>(color.b) << " "
             << static_cast<int>(color.a) << " "
             << object.getLayer()
             << "\n";
    }

    file.close();

    if (file.fail()) {
        std::cerr << "Failed to write scene file: "
                  << filename << std::endl;

        return false;
    }

    std::cout << "Scene saved successfully: "
              << filename << std::endl;

    return true;
}