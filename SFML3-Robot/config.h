#pragma once
#include <string>

struct Config {
    float robotSpeed = 0.5f;
    float cellSize = 40.0f;
    bool showExploredCells = true;
    bool showPath = true;

    bool load(const std::string& filename);
    void save(const std::string& filename) const;
};
