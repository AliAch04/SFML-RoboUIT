#pragma once
#include "Enums.h"
#include "Point.h"
#include <memory>

class Cell {
protected:
    Point position;
    CellType type;
public:
    Cell(Point pos, CellType t) : position(pos), type(t) {}
    virtual ~Cell() = default;  // Ensure virtual destructor
    virtual bool isWalkable() const { return type != CellType::WALL; }
    CellType getType() const { return type; }
    static std::unique_ptr<Cell> create(CellType type, Point pos);
};

class WallCell : public Cell {
public:
    WallCell(Point pos) : Cell(pos, CellType::WALL) {}
    bool isWalkable() const override { return false; }
};

class EmptyCell : public Cell {
public:
    EmptyCell(Point pos) : Cell(pos, CellType::EMPTY) {}
};

class StartCell : public Cell {
public:
    StartCell(Point pos) : Cell(pos, CellType::START) {}
};

class EndCell : public Cell {
public:
    EndCell(Point pos) : Cell(pos, CellType::END) {}
};