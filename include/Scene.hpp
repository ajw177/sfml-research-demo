#pragma once

#include "GameObject.hpp"

#include <string>
#include <vector>

class Scene {
public:
    // Save game objects to a file
    bool save(const std::vector<GameObject>& objects,
              const std::string& filename) const;

    // Load game objects from a file
    bool load(std::vector<GameObject>& objects,
              const std::string& filename) const;
};