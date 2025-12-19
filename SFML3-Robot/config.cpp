#include "Config.h"
#include <fstream>
#include <sstream>

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            if (key == "robotSpeed") robotSpeed = std::stof(value);
            else if (key == "cellSize") cellSize = std::stof(value);
            else if (key == "showExploredCells") showExploredCells = (value == "1");
            else if (key == "showPath") showPath = (value == "1");
        }
    }
    return true;
}

void Config::save(const std::string& filename) const {
    std::ofstream file(filename);
    file << "robotSpeed=" << robotSpeed << "\n";
    file << "cellSize=" << cellSize << "\n";
    file << "showExploredCells=" << (showExploredCells ? 1 : 0) << "\n";
    file << "showPath=" << (showPath ? 1 : 0) << "\n";
}