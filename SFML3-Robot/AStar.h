#pragma once
#include "Maze.h"
#include "Heuristics.h"
#include "Point.h"
#include <vector>
#include <unordered_set>
#include <queue>
#include <unordered_map>

class PathFinder {
protected:
    std::unique_ptr<IHeuristic> heuristic;
    std::unordered_set<Point, PointHash> explored;

public:
    PathFinder();
    void clearExplored() { explored.clear(); }
    const std::unordered_set<Point, PointHash>& getExplored() const { return explored; }

    virtual std::vector<Point> findPath(Maze* maze) = 0;
    bool isSolvable(Maze* maze);
};