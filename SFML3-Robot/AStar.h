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
    virtual ~PathFinder() = default;  

    void clearExplored() { explored.clear(); }
    const std::unordered_set<Point, PointHash>& getExplored() const { return explored; }

    // Méthode virtuelle pure
    virtual std::vector<Point> findPath(Maze* maze) = 0;

    bool isSolvable(Maze* maze);
};

class AStar : public PathFinder {
public:
    AStar();

    // Implémentation concrète de findPath
    std::vector<Point> findPath(Maze* maze) override;
};