#pragma once
#include <string>
#include <vector>
#include "Maze.h"

class MazeBrowser
{
public:
    static std::vector<std::string> getMazeList(const std::string& directoryPath);

    static bool SaveMaze(const Maze& maze, const std::string& filename);

    static bool LoadMaze(Maze& maze, const std::string& filename);
};