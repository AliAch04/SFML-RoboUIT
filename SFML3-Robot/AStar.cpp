#include "AStar.h"
#include <algorithm>
#include <functional>  // For std::greater in priority_queue
#include <memory>      // For std::unique_ptr

PathFinder::PathFinder() {
    heuristic = std::make_unique<ManhattanHeuristic>();
}

std::vector<Point> PathFinder::findPath(Maze* maze) {
    explored.clear();
    if (!maze) return {};
    if (!maze->isValid(maze->startPos) || !maze->isValid(maze->endPos)) return {};
    if (maze->startPos == maze->endPos) return { maze->startPos };

    struct PQNode { float f; Point pos; };
    struct PQComp { bool operator()(PQNode const& a, PQNode const& b) const { return a.f > b.f; } };

    std::priority_queue<PQNode, std::vector<PQNode>, PQComp> open;
    std::unordered_map<Point, float, PointHash> gScore;
    std::unordered_map<Point, Point, PointHash> cameFrom;

    gScore.reserve(1024);
    cameFrom.reserve(1024);

    float h0 = heuristic->calculate(maze->startPos, maze->endPos);
    gScore[maze->startPos] = 0.0f;
    open.push({ h0, maze->startPos });

    Point directions[4] = { {0,1},{0,-1},{1,0},{-1,0} };

    while (!open.empty()) {
        PQNode top = open.top(); open.pop();
        Point current = top.pos;

        auto git = gScore.find(current);
        if (git == gScore.end()) continue;
        float currentG = git->second;
        float expectedF = currentG + heuristic->calculate(current, maze->endPos);
        if (top.f > expectedF + 1e-6f) continue;

        explored.insert(current);

        if (current == maze->endPos) {
            std::vector<Point> path;
            Point p = current;
            path.push_back(p);
            while (cameFrom.find(p) != cameFrom.end()) {
                p = cameFrom[p];
                path.push_back(p);
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (Point dir : directions) {
            Point neighbor = { current.x + dir.x, current.y + dir.y };
            if (!maze->isValid(neighbor) || maze->isWall(neighbor)) continue;

            float tentativeG = currentG + 1.0f;
            auto ngIt = gScore.find(neighbor);
            if (ngIt == gScore.end() || tentativeG + 1e-6f < ngIt->second) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentativeG;
                float f = tentativeG + heuristic->calculate(neighbor, maze->endPos);
                open.push({ f, neighbor });
            }
        }
    }

    return {};
}

bool PathFinder::isSolvable(Maze* maze) {
    auto path = findPath(maze);
    return !path.empty();
}