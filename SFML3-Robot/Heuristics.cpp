#include "Heuristics.h"
#include <cmath>

float ManhattanHeuristic::calculate(Point current, Point goal) {
    return static_cast<float>(std::abs(current.x - goal.x) + std::abs(current.y - goal.y));
}