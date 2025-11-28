#pragma once
#include "Maze.h"
#include "Heuristics.h"
#include "Point.h"
#include <vector>
#include <unordered_set>
#include <queue>
#include <unordered_map>

class PathFinder {
private:
    std::unique_ptr<IHeuristic> heuristic;
    std::unordered_set<Point, PointHash> explored;

public:
    PathFinder();
    void clearExplored() { explored.clear(); }
    const std::unordered_set<Point, PointHash>& getExplored() const { return explored; }

    std::vector<Point> findPath(Maze* maze);
    bool isSolvable(Maze* maze);
};