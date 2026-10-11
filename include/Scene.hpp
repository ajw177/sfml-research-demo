#pragma once

#include "GameObject.hpp"

#include <string>
#include <vector>

class Scene {
public:
    bool save(const std::vector<GameObject>& objects,
              const std::string& filename) const;
};