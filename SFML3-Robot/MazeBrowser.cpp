#include "MazeBrowser.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

std::vector<std::string> MazeBrowser::getMazeList(const std::string& directoryPath)
{
    std::vector<std::string> mazes;

    try {
        if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
            std::cerr << "Erreur: Le dossier " << directoryPath << " n'existe pas." << std::endl;
            return mazes;
        }

        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                mazes.push_back(entry.path().filename().string());
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }

    return mazes;
}

bool MazeBrowser::SaveMaze(const Maze& maze, const std::string& filename)
{
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << maze.width << " " << maze.height << "\n";

    for (int y = 0; y < maze.height; ++y) {
        for (int x = 0; x < maze.width; ++x) {
            int typeInt = 0;
            CellType t = maze.grid[y][x]->getType();

            if (t == CellType::WALL) typeInt = 1;
            else if (t == CellType::START) typeInt = 2;
            else if (t == CellType::END) typeInt = 3;

            file << typeInt << " ";
        }
        file << "\n";
    }

    file.close();
    return true;
}

bool MazeBrowser::LoadMaze(Maze& maze, const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    int w, h;
    file >> w >> h;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int typeInt;
            file >> typeInt;

            CellType type = CellType::EMPTY;
            if (typeInt == 1) type = CellType::WALL;
            else if (typeInt == 2) type = CellType::START;
            else if (typeInt == 3) type = CellType::END;

            maze.setCell(x, y, type);

            if (type == CellType::START) maze.startPos = { x, y };
            if (type == CellType::END) maze.endPos = { x, y };
        }
    }

    file.close();
    return true;
}