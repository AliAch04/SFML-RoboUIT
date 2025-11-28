#include "MazeGenerator.h"
#include "Maze.h"
#include "Point.h"
#include <vector>
#include <random>

void MazeGenerator::generateSolvableMaze(Maze* maze) {
    initializeWithWalls(maze);
    carvePaths(maze);

    // Set start and end positions
    maze->setCell(1, 1, CellType::START);
    maze->setCell(maze->width - 2, maze->height - 2, CellType::END);
}

void MazeGenerator::initializeWithWalls(Maze* maze) {
    for (int y = 0; y < maze->height; ++y) {
        for (int x = 0; x < maze->width; ++x) {
            maze->setCell(x, y, CellType::WALL);
        }
    }
}

void MazeGenerator::carvePaths(Maze* maze) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 3);

    // Start from a random point
    int startX = 1;
    int startY = 1;
    maze->setCell(startX, startY, CellType::EMPTY);

    // Use DFS-like algorithm to generate paths
    std::vector<Point> stack;
    stack.push_back({ startX, startY });

    Point directions[4] = { {0, -2}, {2, 0}, {0, 2}, {-2, 0} };

    while (!stack.empty()) {
        Point current = stack.back();

        // Get unvisited neighbors
        std::vector<Point> neighbors;
        for (Point dir : directions) {
            Point neighbor = { current.x + dir.x, current.y + dir.y };
            if (neighbor.x > 0 && neighbor.x < maze->width - 1 &&
                neighbor.y > 0 && neighbor.y < maze->height - 1 &&
                maze->isWall(neighbor)) {
                neighbors.push_back(neighbor);
            }
        }

        if (!neighbors.empty()) {
            // Choose random neighbor
            Point next = neighbors[dis(gen) % neighbors.size()];

            // Remove wall between current and next
            int wallX = current.x + (next.x - current.x) / 2;
            int wallY = current.y + (next.y - current.y) / 2;
            maze->setCell(wallX, wallY, CellType::EMPTY);
            maze->setCell(next.x, next.y, CellType::EMPTY);

            stack.push_back(next);
        }
        else {
            stack.pop_back();
        }
    }
}