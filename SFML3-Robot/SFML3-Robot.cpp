/*
 * Maze Robot Simulation (A* IMPROVED - Single File)
 * - Option B: fixes + improvements
 * - Safer A* (no raw Node*), gScore + cameFrom maps
 * - Visualize final path and explored cells
 * - Smooth robot animation (interpolated movement)
 * - Minor code cleanups (better PointHash)
 *
 * REQUIREMENTS:
 * 1. SFML installed and configured.
 * 2. Link against: sfml-graphics, sfml-window, sfml-system
 *
 * Build (example, MSVC): link sfml-graphics.lib sfml-window.lib sfml-system.lib
 */

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <memory>
#include <algorithm>


 //// 1. DATA STRUCTURES ////

enum class CellType { EMPTY, WALL, START, END, SPECIAL };
enum class GameState { IDLE, SOLVING, COMPLETE, FAILED };
enum class RobotState { IDLE, CALCULATING, MOVING, COMPLETED };



struct Point {
    int x, y;

    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point& other) const { return !(*this == other); }
};

// Hash for Point to use in unordered_set/map (improved combine)
struct PointHash {
    std::size_t operator()(const Point& p) const {
        std::size_t hx = std::hash<int>()(p.x);
        std::size_t hy = std::hash<int>()(p.y);
        // boost's combine
        return hx ^ (hy + 0x9e3779b97f4a7c15ULL + (hx << 6) + (hx >> 2));
    }
};

//// 2. HEURISTICS ////

class IHeuristic {
public:
    virtual float calculate(Point current, Point goal) = 0;
    virtual ~IHeuristic() = default;
};

class ManhattanHeuristic : public IHeuristic {
public:
    float calculate(Point current, Point goal) override {
        return static_cast<float>(std::abs(current.x - goal.x) + std::abs(current.y - goal.y));
    }
};

//// 3. GRID ELEMENTS ////


class Cell {
protected:
    Point position;
    CellType type;
public:
    Cell(Point pos, CellType t) : position(pos), type(t) {}
    virtual ~Cell() = default;
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

std::unique_ptr<Cell> Cell::create(CellType type, Point pos) {
    switch (type) {
    case CellType::WALL: return std::make_unique<WallCell>(pos);
    case CellType::START: return std::make_unique<StartCell>(pos);
    case CellType::END: return std::make_unique<EndCell>(pos);
    default: return std::make_unique<EmptyCell>(pos);
    }
}

//// 4. MAZE ////

class Maze {
public:
    int width = 0, height = 0;
    Point startPos{ 0,0 }, endPos{ 0,0 };
    std::vector<std::vector<std::unique_ptr<Cell>>> grid;

    Maze() = default;
    Maze(int w, int h) : width(w), height(h) {
        grid.resize(height);
        for (int y = 0; y < height; ++y) {
            grid[y].resize(width);
            for (int x = 0; x < width; ++x) {
                grid[y][x] = Cell::create(CellType::EMPTY, { x,y });
            }
        }
    }

    bool isValid(Point p) const {
        return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
    }

    bool isWall(Point p) const {
        if (!isValid(p)) return true;
        return grid[p.y][p.x]->getType() == CellType::WALL;
    }

    void setCell(int x, int y, CellType type) {
        if (isValid({ x, y })) {
            grid[y][x] = Cell::create(type, { x, y });
            if (type == CellType::START) startPos = { x, y };
            if (type == CellType::END) endPos = { x, y };
        }
    }

    void loadFromMap(const std::vector<std::string>& layout) {
        height = static_cast<int>(layout.size());
        if (height == 0) return;
        width = static_cast<int>(layout[0].size());

        grid.clear();
        grid.resize(height);

        for (int y = 0; y < height; ++y) {
            grid[y].resize(width);
            for (int x = 0; x < width; ++x) {
                char c = layout[y][x];
                CellType t = CellType::EMPTY;
                if (c == '#') t = CellType::WALL;
                else if (c == 'S') t = CellType::START;
                else if (c == 'E') t = CellType::END;
                setCell(x, y, t);
            }
        }
    }
};

//// 5. ROBOT (with smooth movement) ////


class Robot {
private:
    // actual floating position for smooth interpolation
    float fx = 0.0f, fy = 0.0f;
    Point gridPos{ 0,0 };

    // animation state
    Point targetPos{ 0,0 };
    float moveDuration = 0.3f; // seconds
    float elapsed = 0.0f;
    bool moving = false;

    RobotState state = RobotState::IDLE;
    int stepCount = 0;
public:
    Robot() = default;
    void setPosition(Point p) {
        gridPos = p;
        fx = static_cast<float>(p.x);
        fy = static_cast<float>(p.y);
        targetPos = p;
        moving = false;
    }
    Point getPosition() const { return gridPos; }
    void setState(RobotState s) { state = s; }
    RobotState getState() const { return state; }

    // Command robot to move to next grid cell (starts interpolation)
    void moveTo(Point next) {
        if (next == gridPos) return;
        targetPos = next;
        elapsed = 0.0f;
        moving = true;
        state = RobotState::MOVING;
        stepCount++;
    }

    void update(float dt) {
        if (!moving) return;
        elapsed += dt;
        float t = moveDuration <= 0.0f ? 1.0f : (elapsed / moveDuration);
        if (t >= 1.0f) {
            // finish move
            fx = static_cast<float>(targetPos.x);
            fy = static_cast<float>(targetPos.y);
            gridPos = targetPos;
            moving = false;
            state = RobotState::IDLE;
        }
        else {
            // linear interpolate
            float sx = static_cast<float>(gridPos.x);
            float sy = static_cast<float>(gridPos.y);
            fx = sx + (static_cast<float>(targetPos.x) - sx) * t;
            fy = sy + (static_cast<float>(targetPos.y) - sy) * t;
        }
    }

    // get floating pos for rendering
    sf::Vector2f getFloatPos(float cellSize) const {
        return { fx * cellSize, fy * cellSize };
    }

    int getSteps() const { return stepCount; }
    bool isMoving() const { return moving; }
};

//// 6. PATHFINDER (A* with maps, exposes explored set)  ////

class PathFinder {
private:
    std::unique_ptr<IHeuristic> heuristic;
    std::unordered_set<Point, PointHash> explored; // closed set for visualization

public:
    PathFinder() {
        heuristic = std::make_unique<ManhattanHeuristic>();
    }

    void clearExplored() { explored.clear(); }
    const std::unordered_set<Point, PointHash>& getExplored() const { return explored; }

    std::vector<Point> findPath(Maze* maze) {
        explored.clear();
        if (!maze) return {};
        if (!maze->isValid(maze->startPos) || !maze->isValid(maze->endPos)) return {};
        if (maze->startPos == maze->endPos) return { maze->startPos };

        struct PQNode { float f; Point pos; };
        struct PQComp { bool operator()(PQNode const& a, PQNode const& b) const { return a.f > b.f; } }; // min-heap

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

            // If this node is stale (its f doesn't align with gScore+heuristic), skip
            auto git = gScore.find(current);
            if (git == gScore.end()) continue; // no known g
            float currentG = git->second;
            float expectedF = currentG + heuristic->calculate(current, maze->endPos);
            if (top.f > expectedF + 1e-6f) continue; // stale entry

            // mark explored
            explored.insert(current);

            if (current == maze->endPos) {
                // reconstruct
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

        // no path
        return {};
    }
};

//// 7. GAME ENGINE (SFML) ////

class GameEngine {
private:
    std::unique_ptr<Maze> currentMaze;
    std::unique_ptr<Robot> playerRobot;
    std::unique_ptr<PathFinder> pathFinder;
    GameState state = GameState::IDLE;

    const float CELL_SIZE = 60.0f;
    std::vector<Point> solutionPath;
    size_t pathIndex = 0;

public:
    GameEngine() {
        playerRobot = std::make_unique<Robot>();
        pathFinder = std::make_unique<PathFinder>();
        state = GameState::IDLE;
    }

    void loadLevel() {
        std::vector<std::string> levelMap = {
            "##########",
            "#S...#...#",
            "###.####.#",
            "#...#....#",
            "#.###.##.#",
            "#.#....#.#",
            "#.####.#.#",
            "#......#E#",
            "##########"
        };

        currentMaze = std::make_unique<Maze>(10, 9);
        currentMaze->loadFromMap(levelMap);
        playerRobot->setPosition(currentMaze->startPos);
        state = GameState::IDLE;

        // compute path immediately
        computePath();
    }

    void computePath() {
        if (!currentMaze) return;
        pathFinder->clearExplored();
        solutionPath = pathFinder->findPath(currentMaze.get());
        if (solutionPath.empty()) {
            std::cout << "No path found!" << std::endl;
            state = GameState::FAILED;
        }
        else {
            state = GameState::SOLVING;
            // if first node is start, skip so robot doesn't "move to same"
            pathIndex = 0;
            if (!solutionPath.empty() && solutionPath[0] == currentMaze->startPos) pathIndex = 1;
            // ensure robot is on start
            playerRobot->setPosition(currentMaze->startPos);
        }
    }

    void run() {
        if (!currentMaze) return;
        auto windowWidth = static_cast<unsigned int>(currentMaze->width * CELL_SIZE);
        auto windowHeight = static_cast<unsigned int>(currentMaze->height * CELL_SIZE);
        sf::RenderWindow window(sf::VideoMode({ windowWidth, windowHeight }), "Robot A* Improved");
        window.setFramerateLimit(60);

        sf::Clock deltaClock;

        while (window.isOpen()) {
            while (std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();
                if (event->is<sf::Event::KeyPressed>()) {
                    if (event->getIf<sf::Event::KeyPressed>()->scancode == sf::Keyboard::Scancode::R) {
                        // restart level
                        loadLevel();
                    }
                }
            }

            float dt = deltaClock.restart().asSeconds();

            // If solving and robot idle, command next move
            if (state == GameState::SOLVING && !playerRobot->isMoving() && pathIndex < solutionPath.size()) {
                playerRobot->moveTo(solutionPath[pathIndex]);
                pathIndex++;
            }

            // Update robot (smooth interpolation)
            playerRobot->update(dt);

            // If robot reached end cell in grid coordinates
            if (state == GameState::SOLVING && playerRobot->getPosition() == currentMaze->endPos) {
                state = GameState::COMPLETE;
                playerRobot->setState(RobotState::COMPLETED);
                std::cout << "Target Reached using A*! Steps: " << playerRobot->getSteps() << std::endl;
            }

            window.clear(sf::Color(40, 40, 40));
            drawMaze(window);
            drawPathOverlay(window);
            drawRobot(window);
            window.display();
        }
    }

    void drawMaze(sf::RenderWindow& window) {
        sf::RectangleShape cellShape(sf::Vector2f(CELL_SIZE - 2.0f, CELL_SIZE - 2.0f));
        for (int y = 0; y < currentMaze->height; ++y) {
            for (int x = 0; x < currentMaze->width; ++x) {
                CellType t = currentMaze->grid[y][x]->getType();
                cellShape.setPosition(sf::Vector2f(x * CELL_SIZE + 1.0f, y * CELL_SIZE + 1.0f));
                switch (t) {
                case CellType::WALL: cellShape.setFillColor(sf::Color::Black); break;
                case CellType::START: cellShape.setFillColor(sf::Color(100, 220, 100)); break;
                case CellType::END: cellShape.setFillColor(sf::Color(220, 100, 100)); break;
                default: cellShape.setFillColor(sf::Color(200, 200, 200)); break;
                }
                window.draw(cellShape);
            }
        }
    }

    void drawPathOverlay(sf::RenderWindow& window) {
        // draw explored cells lightly
        sf::RectangleShape exploredShape(sf::Vector2f(CELL_SIZE - 6.0f, CELL_SIZE - 6.0f));
        exploredShape.setFillColor(sf::Color(180, 180, 180, 160));
        for (const Point& p : pathFinder->getExplored()) {
            // don't overwrite walls/start/end clarity
            CellType t = currentMaze->grid[p.y][p.x]->getType();
            if (t == CellType::WALL || t == CellType::START || t == CellType::END) continue;
            exploredShape.setPosition(sf::Vector2f(p.x * CELL_SIZE + 3.0f, p.y * CELL_SIZE + 3.0f));
            window.draw(exploredShape);
        }

        // draw final path if present
        if (!solutionPath.empty()) {
            sf::RectangleShape pathShape(sf::Vector2f(CELL_SIZE - 8.0f, CELL_SIZE - 8.0f));
            pathShape.setFillColor(sf::Color(220, 220, 100, 200));
            for (const Point& p : solutionPath) {
                CellType t = currentMaze->grid[p.y][p.x]->getType();
                if (t == CellType::WALL) continue;
                pathShape.setPosition(sf::Vector2f(p.x * CELL_SIZE + 4.0f, p.y * CELL_SIZE + 4.0f));
                window.draw(pathShape);
            }
        }
    }

    void drawRobot(sf::RenderWindow& window) {
        // robot rendered as circle centered in cell using floating pos
        sf::Vector2f floatPos = playerRobot->getFloatPos(CELL_SIZE);
        float radius = CELL_SIZE / 3.0f;
        sf::CircleShape robotShape(radius);
        robotShape.setFillColor(sf::Color::Blue);
        robotShape.setOutlineThickness(2);
        robotShape.setOutlineColor(sf::Color::White);

        // center within cell
        float centerX = floatPos.x + (CELL_SIZE / 2.0f - robotShape.getRadius());
        float centerY = floatPos.y + (CELL_SIZE / 2.0f - robotShape.getRadius());
        robotShape.setPosition(sf::Vector2f(centerX, centerY));
        window.draw(robotShape);
    }
};

int main() {
    GameEngine engine;
    engine.loadLevel();
    engine.run();
    return 0;
}