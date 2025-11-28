#pragma once
#include "Point.h"

class IHeuristic {
public:
    virtual float calculate(Point current, Point goal) = 0;
    virtual ~IHeuristic() = default;
};

class ManhattanHeuristic : public IHeuristic {
public:
    float calculate(Point current, Point goal) override;
};