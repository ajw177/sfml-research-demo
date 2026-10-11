#include "Scene.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

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

bool Scene::load(std::vector<GameObject>& objects,
                 const std::string& filename) const {

    // Open the scene file
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Failed to open scene file: "
                  << filename << std::endl;
        return false;
    }

    // Store loaded objects temporarily
    std::vector<GameObject> loadedObjects;

    std::string line;
    int lineNumber = 0;

    // Read one game object per line
    while (std::getline(file, line)) {
        ++lineNumber;

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        std::istringstream input(line);

        float x, y;
        float width, height;
        int red, green, blue, alpha;
        int layer;

        // Read the nine values describing an object
        if (!(input >> x >> y
                    >> width >> height
                    >> red >> green >> blue >> alpha
                    >> layer)) {

            std::cerr << "Invalid scene data on line "
                      << lineNumber << std::endl;

            return false;
        }

        // Check that the line contains no extra values
        std::string extra;
        if (input >> extra) {
            std::cerr << "Unexpected data on line "
                      << lineNumber << std::endl;
            return false;
        }

        // Validate color components
        if (red < 0 || red > 255 ||
            green < 0 || green > 255 ||
            blue < 0 || blue > 255 ||
            alpha < 0 || alpha > 255) {

            std::cerr << "Invalid color on line "
                      << lineNumber << std::endl;

            return false;
        }

        // Validate object dimensions
        if (width <= 0 || height <= 0) {
            std::cerr << "Invalid object size on line "
                      << lineNumber << std::endl;

            return false;
        }

        // Reconstruct the game object
        loadedObjects.emplace_back(
            sf::Vector2f{x, y},
            sf::Vector2f{width, height},
            sf::Color(
                static_cast<std::uint8_t>(red),
                static_cast<std::uint8_t>(green),
                static_cast<std::uint8_t>(blue),
                static_cast<std::uint8_t>(alpha)
            ),
            layer
        );
    }

    // Check whether reading failed unexpectedly
    if (file.bad()) {
        std::cerr << "Error reading scene file: "
                  << filename << std::endl;
        return false;
    }

    // Replace the current scene only after loading succeeds
    objects = std::move(loadedObjects);

    std::cout << "Scene loaded successfully: "
              << filename << std::endl;

    std::cout << "Objects loaded: "
              << objects.size() << std::endl;

    return true;
}