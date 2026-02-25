#pragma once
#include "Cell.h"
#include "Enums.h"
#include "Point.h"
#include <vector>
#include <memory>
#include <string>

class Maze {
public:
    int width = 0, height = 0;
    Point startPos{ 0,0 }, endPos{ 0,0 };
    std::vector<std::vector<std::unique_ptr<Cell>>> grid;

    Maze() = default;
    Maze(int w, int h);

    bool isValid(Point p) const;
    bool isWall(Point p) const;
    void setCell(int x, int y, CellType type);
    void loadFromMap(const std::vector<std::string>& layout);
    void resize(int newWidth, int newHeight);
    void generateSolvableMaze();

    std::vector<std::string> toStringVector() const;

private:
    void initializeGrid();
};