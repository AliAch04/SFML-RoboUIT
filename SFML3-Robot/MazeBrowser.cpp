#include "MazeBrowser.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include "includes/json.hpp"  

using json = nlohmann::json;
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

bool MazeBrowser::SaveMaze(const Maze& maze, const std::string& filename) {
    try {
        // Créer un objet JSON
        json j;

        // Ajouter les métadonnées
        j["name"] = fs::path(filename).stem().string();
        j["width"] = maze.width;
        j["height"] = maze.height;

        // Ajouter les positions
        j["start"]["x"] = maze.startPos.x;
        j["start"]["y"] = maze.startPos.y;
        j["end"]["x"] = maze.endPos.x;
        j["end"]["y"] = maze.endPos.y;

        // Créer la grille
        json grid = json::array();
        for (int y = 0; y < maze.height; ++y) {
            json row = json::array();
            for (int x = 0; x < maze.width; ++x) {
                CellType t = maze.grid[y][x]->getType();
                int typeInt = 0;
                if (t == CellType::WALL) typeInt = 1;
                else if (t == CellType::START) typeInt = 2;
                else if (t == CellType::END) typeInt = 3;
                else if (t == CellType::SPECIAL) typeInt = 4;
                row.push_back(typeInt);
            }
            grid.push_back(row);
        }
        j["grid"] = grid;

        // Écrire dans le fichier
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[MazeBrowser] Erreur: Impossible d'ouvrir " << filename << std::endl;
            return false;
        }

        file << j.dump(2);  // Pretty print avec indentation de 2 espaces
        file.close();

        std::cout << "[MazeBrowser] Labyrinthe sauvegardé (JSON): " << filename << std::endl;
        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "[MazeBrowser] Exception: " << e.what() << std::endl;
        return false;
    }
}

bool MazeBrowser::LoadMaze(Maze& maze, const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "[MazeBrowser] Impossible d'ouvrir: " << filename << std::endl;
            return false;
        }

        // Parser le JSON
        json j;
        file >> j;
        file.close();

        // Lire les dimensions
        int width = j["width"];
        int height = j["height"];

        std::cout << "[MazeBrowser] Chargement: " << width << "x" << height << std::endl;

        // Redimensionner le labyrinthe
        maze = Maze(width, height);

        // Lire les positions
        maze.startPos = { j["start"]["x"], j["start"]["y"] };
        maze.endPos = { j["end"]["x"], j["end"]["y"] };

        // Lire la grille
        json grid = j["grid"];
        for (int y = 0; y < height; ++y) {
            json row = grid[y];
            for (int x = 0; x < width; ++x) {
                int typeInt = row[x];
                CellType type = CellType::EMPTY;

                if (typeInt == 1) type = CellType::WALL;
                else if (typeInt == 2) type = CellType::START;
                else if (typeInt == 3) type = CellType::END;
                else if (typeInt == 4) type = CellType::SPECIAL;

                maze.grid[y][x]->setType(type);
            }
        }

        std::cout << "[MazeBrowser] Chargé avec succès!" << std::endl;
        return true;

    }
    catch (const json::exception& e) {
        std::cerr << "[MazeBrowser] Erreur JSON: " << e.what() << std::endl;
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "[MazeBrowser] Exception: " << e.what() << std::endl;
        return false;
    }
}