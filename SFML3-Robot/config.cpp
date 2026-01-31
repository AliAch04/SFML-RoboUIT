#include "Config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>

bool Config::load(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string key, value;

        if (std::getline(ss, key, '=') && std::getline(ss, value))
        {
            // Existing settings
            if (key == "robotSpeed")
                robotSpeed = std::stof(value);
            else if (key == "cellSize")
                cellSize = std::stof(value);
            else if (key == "showExploredCells")
                showExploredCells = (value == "1");
            else if (key == "showPath")
                showPath = (value == "1");

            // Texture paths
            else if (key == "robotTexturePath")
                robotTexturePath = value;
            else if (key == "wallTexturePath")
                wallTexturePath = value;
            else if (key == "floorTexturePath")
                floorTexturePath = value;
            else if (key == "obstacleTexturePath")
                obstacleTexturePath = value;

            // NEW: Sound settings
            else if (key == "musicVolume")
                musicVolume = std::stof(value);
            else if (key == "sfxVolume")
                sfxVolume = std::stof(value);
            else if (key == "musicMuted")
                musicMuted = (value == "1");
            else if (key == "sfxMuted")
                sfxMuted = (value == "1");

        }
    }

    return true;
}

// Config.cpp



void Config::save(const std::string& filename) const
{
    // 1) Open with trunc so we overwrite cleanly
    std::ofstream file(filename, std::ios::out | std::ios::trunc);

    // 2) HARD CHECK (otherwise it fails silently)
    if (!file.is_open())
    {
        char buf[256];
        strerror_s(buf, sizeof(buf), errno);
        std::cout << buf << "\n";
        return;
    }

    // Existing settings
    file << "robotSpeed=" << robotSpeed << "\n";
    file << "cellSize=" << cellSize << "\n";
    file << "showExploredCells=" << (showExploredCells ? 1 : 0) << "\n";
    file << "showPath=" << (showPath ? 1 : 0) << "\n";

    // Texture paths
    file << "robotTexturePath=" << robotTexturePath << "\n";
    file << "wallTexturePath=" << wallTexturePath << "\n";
    file << "floorTexturePath=" << floorTexturePath << "\n";
    file << "obstacleTexturePath=" << obstacleTexturePath << "\n";

    // Sound settings
    file << "musicVolume=" << musicVolume << "\n";
    file << "sfxVolume=" << sfxVolume << "\n";
    file << "musicMuted=" << (musicMuted ? 1 : 0) << "\n";
    file << "sfxMuted=" << (sfxMuted ? 1 : 0) << "\n";

    file.flush();
    file.close();

    // 3) CONFIRMATION LOG
    std::cout << "[CONFIG] SAVE OK -> '" << filename << "'\n";
    std::cout << "[CONFIG] saved texture paths:\n"
        << "  robot=" << robotTexturePath << "\n"
        << "  wall =" << wallTexturePath << "\n"
        << "  floor=" << floorTexturePath << "\n"
        << "  obst =" << obstacleTexturePath << "\n";
}

