#pragma once
#include <functional>
#include <cstddef>

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point& other) const { return !(*this == other); }
};

struct PointHash {
    std::size_t operator()(const Point& p) const {
        // Use a simpler hash combination 
        return static_cast<std::size_t>(p.x) * 31 + static_cast<std::size_t>(p.y);
    }
};

