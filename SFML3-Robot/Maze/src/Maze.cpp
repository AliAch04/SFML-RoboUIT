#include "Maze.h"
#include "MazeGenerator.h"
#include <algorithm>
#include <string>

Maze::Maze(int w, int h) : width(w), height(h) {
    initializeGrid();
}

void Maze::initializeGrid() {
    grid.resize(height);
    for (int y = 0; y < height; ++y) {
        grid[y].resize(width);
        for (int x = 0; x < width; ++x) {
            grid[y][x] = Cell::create(CellType::EMPTY, { x,y });
        }
    }
}

bool Maze::isValid(Point p) const {
    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
}

bool Maze::isWall(Point p) const {
    if (!isValid(p)) return true;
    return grid[p.y][p.x]->getType() == CellType::WALL;
}

void Maze::setCell(int x, int y, CellType type) {
    if (isValid({ x, y })) {
        grid[y][x] = Cell::create(type, { x, y });
        if (type == CellType::START) startPos = { x, y };
        if (type == CellType::END) endPos = { x, y };
    }
}

void Maze::loadFromMap(const std::vector<std::string>& layout) {
    height = static_cast<int>(layout.size());
    if (height == 0) return;
    width = static_cast<int>(layout[0].size());

    grid.clear();
    grid.resize(height);

    for (int y = 0; y < height; ++y) {
        grid[y].resize(width);
        for (int x = 0; x < width; ++x) {
            char c = layout[y][x];
            CellType t = CellType::EMPTY;
            if (c == '#') t = CellType::WALL;
            else if (c == 'S') t = CellType::START;
            else if (c == 'E') t = CellType::END;
            setCell(x, y, t);
        }
    }
}

void Maze::resize(int newWidth, int newHeight) {
    std::vector<std::vector<std::unique_ptr<Cell>>> newGrid;
    newGrid.resize(newHeight);

    for (int y = 0; y < newHeight; ++y) {
        newGrid[y].resize(newWidth);
        for (int x = 0; x < newWidth; ++x) {
            if (y < height && x < width) {
                // Copy existing cell
                newGrid[y][x] = std::move(grid[y][x]);
            }
            else {
                // NEW CELLS ARE WALLS, NOT EMPTY
                newGrid[y][x] = Cell::create(CellType::WALL, { x, y });
            }
        }
    }

    grid = std::move(newGrid);
    width = newWidth;
    height = newHeight;

    // Update start position if out of bounds
    if (startPos.x >= width || startPos.y >= height) {
        // Find a valid start position (non-wall cell)
        bool found = false;
        for (int y = 0; y < height && !found; ++y) {
            for (int x = 0; x < width && !found; ++x) {
                if (grid[y][x]->getType() != CellType::WALL) {
                    startPos = { x, y };
                    grid[y][x]->setType(CellType::START);
                    found = true;
                }
            }
        }
        if (!found) {
            // All cells are walls, force a non-wall
            startPos = { 0, 0 };
            grid[0][0]->setType(CellType::START);
        }
    }

    // Update end position if out of bounds
    if (endPos.x >= width || endPos.y >= height) {
        // Find a valid end position (non-wall cell, not start)
        bool found = false;
        for (int y = height - 1; y >= 0 && !found; --y) {
            for (int x = width - 1; x >= 0 && !found; --x) {
                Point p = { x, y };
                if (p != startPos && grid[y][x]->getType() != CellType::WALL) {
                    endPos = p;
                    grid[y][x]->setType(CellType::END);
                    found = true;
                }
            }
        }
        if (!found) {
            // Force a non-wall
            if (grid[0][1]->getType() != CellType::WALL) {
                endPos = { 1, 0 };
            }
            else {
                endPos = { 1, 1 };
            }
            grid[endPos.y][endPos.x]->setType(CellType::END);
        }
    }
}

void Maze::generateSolvableMaze() {
    MazeGenerator::generateSolvableMaze(this);
}

std::vector<std::string> Maze::toStringVector() const {
    std::vector<std::string> result;
    for (int y = 0; y < height; ++y) {
        std::string row;
        for (int x = 0; x < width; ++x) {
            CellType t = grid[y][x]->getType();
            if (t == CellType::WALL) row += '#';
            else if (t == CellType::START) row += 'S';
            else if (t == CellType::END) row += 'E';
            else row += '.';
        }
        result.push_back(row);
    }
    return result;
}