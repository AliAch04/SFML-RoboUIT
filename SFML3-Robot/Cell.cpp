#include "Cell.h"
#include <memory>

std::unique_ptr<Cell> Cell::create(CellType type, Point pos) {
    switch (type) {
    case CellType::WALL: return std::make_unique<WallCell>(pos);
    case CellType::START: return std::make_unique<StartCell>(pos);
    case CellType::END: return std::make_unique<EndCell>(pos);
    default: return std::make_unique<EmptyCell>(pos);
    }
}