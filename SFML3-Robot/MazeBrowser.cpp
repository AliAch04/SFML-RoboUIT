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
    if (!file.is_open()) {
        std::cerr << "[MazeBrowser] Impossible d'ouvrir le fichier: " << filename << std::endl;
        return false;
    }

    int w, h;
    file >> w >> h;

    // DEBUG: Vérifier les dimensions lues
    std::cout << "[MazeBrowser] Lecture des dimensions: " << w << "x" << h << std::endl;

    // Vérifier que les dimensions sont valides
    if (w <= 0 || h <= 0 || w > 100 || h > 100) {
        std::cerr << "[MazeBrowser] Dimensions invalides: " << w << "x" << h << std::endl;
        return false;
    }

    // IMPORTANT: Redimensionner le labyrinthe avec les nouvelles dimensions
    maze = Maze(w, h);  // Créer un nouveau labyrinthe avec les bonnes dimensions

    // Réinitialiser les positions de départ/arrivée
    maze.startPos = { 0, 0 };
    maze.endPos = { w - 1, h - 1 };

    // Lire la grille
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int typeInt;
            if (!(file >> typeInt)) {
                std::cerr << "[MazeBrowser] Erreur de lecture à la position (" << x << "," << y << ")" << std::endl;
                return false;
            }

            CellType type = CellType::EMPTY;
            if (typeInt == 1) type = CellType::WALL;
            else if (typeInt == 2) type = CellType::START;
            else if (typeInt == 3) type = CellType::END;
            else if (typeInt == 4) type = CellType::SPECIAL;

            // Utiliser la méthode setCell du Maze pour mettre à jour le type
            if (maze.grid[y][x]) {
                maze.grid[y][x]->setType(type);
            }

            // Mettre à jour les positions de départ et d'arrivée
            if (type == CellType::START) {
                maze.startPos = { x, y };
                std::cout << "[MazeBrowser] Position de départ trouvée: (" << x << "," << y << ")" << std::endl;
            }
            if (type == CellType::END) {
                maze.endPos = { x, y };
                std::cout << "[MazeBrowser] Position d'arrivée trouvée: (" << x << "," << y << ")" << std::endl;
            }
        }
    }

    file.close();

    // DEBUG: Confirmer les dimensions finales
    std::cout << "[MazeBrowser] Labyrinthe chargé: " << filename
        << " (" << maze.width << "x" << maze.height << ")" << std::endl;
    std::cout << "[MazeBrowser] Start: (" << maze.startPos.x << "," << maze.startPos.y
        << "), End: (" << maze.endPos.x << "," << maze.endPos.y << ")" << std::endl;

    return true;
}