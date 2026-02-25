#pragma once
#include <string>

struct Config {
    float robotSpeed = 0.5f;
    float cellSize = 40.0f;
    bool showExploredCells = true;
    bool showPath = true;
    float messageDisplayTime = 3.0f;

    // new texture paths
    std::string robotTexturePath = "assets/textures/robot.png";
    std::string wallTexturePath = "assets/textures/wall.png";
    std::string floorTexturePath = "assets/textures/floor.png";
    std::string obstacleTexturePath = "assets/textures/obstacle.png";

    bool load(const std::string& filename);
    void save(const std::string& filename) const;

    // Sound settings
    float musicVolume = 70.0f;
    float sfxVolume = 100.0f;
    bool musicMuted = false;
    bool sfxMuted = false;
};
