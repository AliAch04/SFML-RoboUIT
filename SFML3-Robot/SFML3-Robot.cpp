/*
 * Maze Robot Simulation (A* IMPROVED - Single File)
 * - Compatible with SFML 2.5+ and SFML 3.0
 * - Safer A* with gScore + cameFrom maps
 * - Visualize final path and explored cells
 * - Smooth robot animation (interpolated movement)
 *
 * REQUIREMENTS:
 * 1. SFML 2.5+ installed and configured.
 * 2. Link against: sfml-graphics, sfml-window, sfml-system
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
#include <sstream>
#include <fstream>
#include <random>

 // Simple JSON writer for saving mazes
class SimpleJSON {
public:
    static std::string stringify(const std::vector<std::string>& maze, const std::string& name, int width, int height) {
        std::stringstream json;
        json << "{\n";
        json << "  \"name\": \"" << name << "\",\n";
        json << "  \"width\": " << width << ",\n";
        json << "  \"height\": " << height << ",\n";
        json << "  \"layout\": [\n";

        for (size_t i = 0; i < maze.size(); ++i) {
            json << "    \"" << maze[i] << "\"";
            if (i < maze.size() - 1) json << ",";
            json << "\n";
        }

        json << "  ]\n";
        json << "}";
        return json.str();
    }
};

//// 1. DATA STRUCTURES ////

enum class CellType { EMPTY, WALL, START, END, SPECIAL };
enum class GameState { IDLE, SOLVING, COMPLETE, FAILED };
enum class RobotState { IDLE, CALCULATING, MOVING, COMPLETED, PAUSED };

enum class AppState { MAIN_MENU, GAME, OPTIONS };
enum class MenuButton { START, OPTIONS, EXIT, NONE };

// Helper function to convert numbers to string
template<typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    bool isHovered = false;

public:
    Button(const sf::Vector2f& size, const sf::Vector2f& position,
        const std::string& buttonText, sf::Font& font, unsigned int characterSize = 24)
    {
        shape.setSize(size);
        shape.setPosition(position);
        shape.setFillColor(sf::Color(70, 70, 70));
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color::White);

        text.setFont(font);
        text.setString(buttonText);
        text.setCharacterSize(characterSize);
        text.setFillColor(sf::Color::White);

        // Center text in button
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
    }

    void setHovered(bool hover) {
        isHovered = hover;
        shape.setFillColor(hover ? sf::Color(100, 100, 100) : sf::Color(70, 70, 70));
    }

    bool contains(const sf::Vector2f& point) const {
        return shape.getGlobalBounds().contains(point);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(shape);
        window.draw(text);
    }

    void setText(const std::string& newText, sf::Font& font) {
        text.setString(newText);
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
        sf::Vector2f pos = shape.getPosition();
        sf::Vector2f size = shape.getSize();
        text.setPosition(pos.x + size.x / 2.0f, pos.y + size.y / 2.0f);
    }

    void setPosition(const sf::Vector2f& position) {
        shape.setPosition(position);
        sf::Vector2f size = shape.getSize();
        text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
    }
};

class Slider {
private:
    sf::RectangleShape track;
    sf::CircleShape thumb;
    sf::Text label;
    sf::Text valueText;
    float minValue, maxValue, currentValue;
    bool dragging = false;

public:
    Slider(const sf::Vector2f& position, float width, float minVal, float maxVal, float initialVal,
        const std::string& sliderLabel, sf::Font& font)
        : minValue(minVal), maxValue(maxVal), currentValue(initialVal) {

        track.setSize(sf::Vector2f(width, 5));
        track.setPosition(position);
        track.setFillColor(sf::Color(150, 150, 150));

        thumb.setRadius(8);
        thumb.setFillColor(sf::Color::White);
        thumb.setOutlineThickness(1);
        thumb.setOutlineColor(sf::Color::Black);

        label.setFont(font);
        label.setString(sliderLabel);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::White);
        label.setPosition(position.x, position.y - 25);

        valueText.setFont(font);
        valueText.setCharacterSize(16);
        valueText.setFillColor(sf::Color::White);
        valueText.setPosition(position.x + width + 10, position.y - 5);

        updateThumbPosition();
        updateValueText();
    }

    void updateThumbPosition() {
        float ratio = (currentValue - minValue) / (maxValue - minValue);
        float x = track.getPosition().x + ratio * track.getSize().x;
        thumb.setPosition(x - thumb.getRadius(), track.getPosition().y - thumb.getRadius());
    }

    void updateValueText() {
        valueText.setString(toString(currentValue));
    }

    bool contains(const sf::Vector2f& point) const {
        return thumb.getGlobalBounds().contains(point);
    }

    void setValueFromMouse(const sf::Vector2f& point) {
        float relativeX = point.x - track.getPosition().x;
        float ratio = std::max(0.0f, std::min(1.0f, relativeX / track.getSize().x));
        currentValue = minValue + ratio * (maxValue - minValue);
        updateThumbPosition();
        updateValueText();
    }

    void setDragging(bool drag) { dragging = drag; }
    bool isDragging() const { return dragging; }
    float getValue() const { return currentValue; }
    void setValue(float value) {
        currentValue = std::max(minValue, std::min(maxValue, value));
        updateThumbPosition();
        updateValueText();
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(track);
        window.draw(thumb);
        window.draw(label);
        window.draw(valueText);
    }
};

class TextInput {
private:
    sf::RectangleShape box;
    sf::Text text;
    sf::Text label;
    std::string inputText;
    bool focused = false;

public:
    TextInput(const sf::Vector2f& position, float width, const std::string& labelText, sf::Font& font) {
        box.setSize(sf::Vector2f(width, 30));
        box.setPosition(position);
        box.setFillColor(sf::Color::White);
        box.setOutlineThickness(2);
        box.setOutlineColor(sf::Color(150, 150, 150));

        label.setFont(font);
        label.setString(labelText);
        label.setCharacterSize(18);
        label.setFillColor(sf::Color::White);
        label.setPosition(position.x, position.y - 25);

        text.setFont(font);
        text.setString("");
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::Black);
        text.setPosition(position.x + 5, position.y + 5);
    }

    bool contains(const sf::Vector2f& point) const {
        return box.getGlobalBounds().contains(point);
    }

    void setFocused(bool focus) {
        focused = focus;
        box.setOutlineColor(focus ? sf::Color::Blue : sf::Color(150, 150, 150));
    }

    bool isFocused() const { return focused; }

    void handleTextEntered(sf::Uint32 unicode) {
        if (!focused) return;

        if (unicode == 8) { // Backspace
            if (!inputText.empty()) {
                inputText.pop_back();
            }
        }
        else if (unicode == 13) { // Enter
            focused = false;
            box.setOutlineColor(sf::Color(150, 150, 150));
        }
        else if (unicode >= 32 && unicode < 128) { // Printable characters
            inputText += static_cast<char>(unicode);
        }

        text.setString(inputText);
    }

    std::string getText() const { return inputText; }
    void setText(const std::string& newText) {
        inputText = newText;
        text.setString(inputText);
    }

    void draw(sf::RenderWindow& window) const {
        window.draw(box);
        window.draw(text);
        window.draw(label);
    }
};

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point& other) const { return !(*this == other); }
};

struct PointHash {
    std::size_t operator()(const Point& p) const {
        std::size_t hx = std::hash<int>()(p.x);
        std::size_t hy = std::hash<int>()(p.y);
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

    std::vector<std::string> toStringVector() const {
        std::vector<std::string> result;
        for (int y = 0; y < height; ++y) {
            std::string row;
            for (int x = 0; x < width; ++x) {
                CellType t = grid[y][x]->getType();
                if (t == CellType::WALL) row += '#';
                else if (t == CellType::START) row += 'S';
                else if (t == CellType::END) row += 'E';
                else row += '.';
            }
            result.push_back(row);
        }
        return result;
    }

    void resize(int newWidth, int newHeight) {
        std::vector<std::vector<std::unique_ptr<Cell>>> newGrid;
        newGrid.resize(newHeight);

        for (int y = 0; y < newHeight; ++y) {
            newGrid[y].resize(newWidth);
            for (int x = 0; x < newWidth; ++x) {
                if (y < height && x < width) {
                    newGrid[y][x] = std::move(grid[y][x]);
                }
                else {
                    newGrid[y][x] = Cell::create(CellType::EMPTY, { x, y });
                }
            }
        }

        grid = std::move(newGrid);
        width = newWidth;
        height = newHeight;

        // Update start and end positions if they're out of bounds
        if (startPos.x >= width || startPos.y >= height) {
            startPos = { 0, 0 };
            setCell(0, 0, CellType::START);
        }
        if (endPos.x >= width || endPos.y >= height) {
            endPos = { width - 1, height - 1 };
            setCell(width - 1, height - 1, CellType::END);
        }
    }

    void generateSolvableMaze() {
        // Initialize with all walls
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                setCell(x, y, CellType::WALL);
            }
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 3);

        // Start from a random point
        int startX = 1;
        int startY = 1;
        setCell(startX, startY, CellType::EMPTY);

        // Use DFS-like algorithm to generate paths
        std::vector<Point> stack;
        stack.push_back({ startX, startY });

        Point directions[4] = { {0, -2}, {2, 0}, {0, 2}, {-2, 0} };

        while (!stack.empty()) {
            Point current = stack.back();

            // Get unvisited neighbors
            std::vector<Point> neighbors;
            for (Point dir : directions) {
                Point neighbor = { current.x + dir.x, current.y + dir.y };
                if (neighbor.x > 0 && neighbor.x < width - 1 &&
                    neighbor.y > 0 && neighbor.y < height - 1 &&
                    isWall(neighbor)) {
                    neighbors.push_back(neighbor);
                }
            }

            if (!neighbors.empty()) {
                // Choose random neighbor
                Point next = neighbors[dis(gen) % neighbors.size()];

                // Remove wall between current and next
                int wallX = current.x + (next.x - current.x) / 2;
                int wallY = current.y + (next.y - current.y) / 2;
                setCell(wallX, wallY, CellType::EMPTY);
                setCell(next.x, next.y, CellType::EMPTY);

                stack.push_back(next);
            }
            else {
                stack.pop_back();
            }
        }

        // Set start and end positions
        setCell(1, 1, CellType::START);
        setCell(width - 2, height - 2, CellType::END);
    }
};

//// 5. ROBOT (with smooth movement) ////

class Robot {
private:
    float fx = 0.0f, fy = 0.0f;
    Point gridPos{ 0,0 };
    Point targetPos{ 0,0 };
    float moveDuration = 0.3f;
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

    void setMoveDuration(float duration) { moveDuration = duration; }
    float getMoveDuration() const { return moveDuration; }

    void moveTo(Point next) {
        if (next == gridPos) return;
        targetPos = next;
        elapsed = 0.0f;
        moving = true;
        state = RobotState::MOVING;
        stepCount++;
    }

    void update(float dt) {
        if (state == RobotState::PAUSED) return;
        if (!moving) return;

        elapsed += dt;
        float t = moveDuration <= 0.0f ? 1.0f : (elapsed / moveDuration);
        if (t >= 1.0f) {
            fx = static_cast<float>(targetPos.x);
            fy = static_cast<float>(targetPos.y);
            gridPos = targetPos;
            moving = false;
            state = RobotState::IDLE;
        }
        else {
            float sx = static_cast<float>(gridPos.x);
            float sy = static_cast<float>(gridPos.y);
            fx = sx + (static_cast<float>(targetPos.x) - sx) * t;
            fy = sy + (static_cast<float>(targetPos.y) - sy) * t;
        }
    }

    void pause() {
        if (state == RobotState::MOVING || state == RobotState::IDLE) {
            state = RobotState::PAUSED;
        }
    }

    void resume() {
        if (state == RobotState::PAUSED) {
            state = moving ? RobotState::MOVING : RobotState::IDLE;
        }
    }

    bool isPaused() const { return state == RobotState::PAUSED; }

    sf::Vector2f getFloatPos(float cellSize) const {
        return { fx * cellSize, fy * cellSize };
    }

    int getSteps() const { return stepCount; }
    bool isMoving() const { return moving; }
};

//// 6. PATHFINDER (A*) ////

class PathFinder {
private:
    std::unique_ptr<IHeuristic> heuristic;
    std::unordered_set<Point, PointHash> explored;

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

    bool isSolvable(Maze* maze) {
        auto path = findPath(maze);
        return !path.empty();
    }
};

//// 7. GAME ENGINE ////

class GameEngine {
private:
    std::unique_ptr<Maze> currentMaze;
    std::unique_ptr<Robot> playerRobot;
    std::unique_ptr<PathFinder> pathFinder;
    GameState state = GameState::IDLE;
    AppState appState = AppState::MAIN_MENU;

    float CELL_SIZE = 40.0f;
    float MIN_CELL_SIZE = 20.0f;
    float MAX_CELL_SIZE = 80.0f;
    std::vector<Point> solutionPath;
    size_t pathIndex = 0;

    sf::Font font;
    std::vector<Button> menuButtons;
    std::vector<Button> optionButtons;
    std::vector<Button> gameButtons;
    std::vector<std::unique_ptr<Slider>> optionSliders;
    std::unique_ptr<TextInput> mazeNameInput;
    std::unique_ptr<TextInput> mazeWidthInput;
    std::unique_ptr<TextInput> mazeHeightInput;

    sf::Text titleText;
    sf::Text optionsTitleText;
    sf::Text gameTitleText;
    bool fontLoaded = false;

    // Maze position for centering
    sf::Vector2f mazeOffset;

    // Configurable parameters
    float robotSpeed = 0.3f;
    float cellSizeValue = 40.0f;
    bool showExploredCells = true;
    bool showPath = true;
    std::string currentMazeName = "My Maze";

    // Run/Pause state
    bool isRunning = false;

public:
    GameEngine() : playerRobot(std::make_unique<Robot>()),
        pathFinder(std::make_unique<PathFinder>()) {

        // Try to load font from common locations
        std::vector<std::string> fontPaths = {
            "arial.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/System/Library/Fonts/Helvetica.ttc"
        };

        for (const auto& path : fontPaths) {
            if (font.loadFromFile(path)) {
                fontLoaded = true;
                std::cout << "Font loaded from: " << path << std::endl;
                break;
            }
        }

        if (!fontLoaded) {
            std::cout << "Warning: Could not load font." << std::endl;
        }

        if (fontLoaded) {
            titleText.setFont(font);
            optionsTitleText.setFont(font);
            gameTitleText.setFont(font);
            setupMainMenu();
            setupOptionsMenu();
            setupGameUI();
        }
    }

    void setupMainMenu() {
        if (!fontLoaded) return;

        titleText.setString("MAZE ROBOT SIMULATION");
        titleText.setCharacterSize(48);
        titleText.setFillColor(sf::Color::White);
        titleText.setStyle(sf::Text::Bold);

        menuButtons.clear();
        menuButtons.emplace_back(sf::Vector2f(200, 50), sf::Vector2f(300, 250), "START", font);
        menuButtons.emplace_back(sf::Vector2f(200, 50), sf::Vector2f(300, 320), "OPTIONS", font);
        menuButtons.emplace_back(sf::Vector2f(200, 50), sf::Vector2f(300, 390), "EXIT", font);
    }

    void setupOptionsMenu() {
        if (!fontLoaded) return;

        optionsTitleText.setString("OPTIONS");
        optionsTitleText.setCharacterSize(48);
        optionsTitleText.setFillColor(sf::Color::White);
        optionsTitleText.setStyle(sf::Text::Bold);

        optionButtons.clear();
        optionButtons.emplace_back(sf::Vector2f(150, 40), sf::Vector2f(325, 450), "BACK", font, 20);

        optionSliders.clear();
        optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(250, 150), 300, 0.1f, 1.0f, robotSpeed, "Robot Speed", font));
        optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(250, 220), 300, 20.0f, 80.0f, cellSizeValue, "Cell Size", font));

        // Toggle buttons for boolean options
        optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, 290),
            showExploredCells ? "Explored: ON" : "Explored: OFF", font, 18);
        optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, 350),
            showPath ? "Path: ON" : "Path: OFF", font, 18);
    }

    void setupGameUI() {
        if (!fontLoaded) return;

        gameTitleText.setString("MAZE SIMULATION");
        gameTitleText.setCharacterSize(24);
        gameTitleText.setFillColor(sf::Color::White);
        gameTitleText.setStyle(sf::Text::Bold);

        // Control panel buttons
        gameButtons.clear();
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 100), "Zoom+", font, 16);
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 140), "Zoom-", font, 16);
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 180), "Generate", font, 16);
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 220), "Run", font, 16);
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 260), "Tester", font, 16);
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 300), "Sauver", font, 16);
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 340), "Resize", font, 16);
        gameButtons.emplace_back(sf::Vector2f(120, 30), sf::Vector2f(650, 380), "Back", font, 16);

        // Text inputs for maze configuration
        mazeNameInput = std::make_unique<TextInput>(sf::Vector2f(650, 420), 120, "Maze Name", font);
        mazeWidthInput = std::make_unique<TextInput>(sf::Vector2f(650, 470), 55, "Width", font);
        mazeHeightInput = std::make_unique<TextInput>(sf::Vector2f(720, 470), 55, "Height", font);

        mazeNameInput->setText(currentMazeName);
        mazeWidthInput->setText("15");
        mazeHeightInput->setText("15");
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
        isRunning = false;

        // Apply settings
        playerRobot->setMoveDuration(robotSpeed);
        CELL_SIZE = cellSizeValue;

        computePath();
        updateMazePosition();
    }

    void updateMazePosition() {
        if (!currentMaze) return;

        float mazeWidth = currentMaze->width * CELL_SIZE;
        float mazeHeight = currentMaze->height * CELL_SIZE;

        // Center the maze in the left part of the window (before control panel)
        mazeOffset.x = (600 - mazeWidth) / 2.0f;
        mazeOffset.y = (600 - mazeHeight) / 2.0f;

        // Ensure maze doesn't go behind control panel
        if (mazeOffset.x + mazeWidth > 600) {
            mazeOffset.x = 600 - mazeWidth - 10;
        }
        if (mazeOffset.x < 10) {
            mazeOffset.x = 10;
        }
        if (mazeOffset.y < 10) {
            mazeOffset.y = 10;
        }
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
            pathIndex = 0;
            if (!solutionPath.empty() && solutionPath[0] == currentMaze->startPos) pathIndex = 1;
            playerRobot->setPosition(currentMaze->startPos);
        }
    }

    void zoomIn() {
        CELL_SIZE = std::min(MAX_CELL_SIZE, CELL_SIZE + 5.0f);
        updateMazePosition();
    }

    void zoomOut() {
        CELL_SIZE = std::max(MIN_CELL_SIZE, CELL_SIZE - 5.0f);
        updateMazePosition();
    }

    void generateMaze() {
        if (!currentMaze) return;

        try {
            int width = std::stoi(mazeWidthInput->getText());
            int height = std::stoi(mazeHeightInput->getText());

            width = std::max(5, std::min(30, width));
            height = std::max(5, std::min(30, height));

            currentMaze = std::make_unique<Maze>(width, height);
            currentMaze->generateSolvableMaze();
            playerRobot->setPosition(currentMaze->startPos);
            state = GameState::IDLE;
            isRunning = false;
            computePath();
            updateMazePosition();

            std::cout << "Generated new maze: " << width << "x" << height << std::endl;
        }
        catch (...) {
            std::cout << "Invalid size input for maze generation!" << std::endl;
        }
    }

    void toggleRunPause() {
        if (!currentMaze) return;

        if (isRunning) {
            // Pause
            playerRobot->pause();
            isRunning = false;
            gameButtons[3].setText("Run", font);
        }
        else {
            // Run
            if (state == GameState::COMPLETE || state == GameState::FAILED) {
                // Reset if completed or failed
                playerRobot->setPosition(currentMaze->startPos);
                pathIndex = 1;
                state = GameState::SOLVING;
            }
            playerRobot->resume();
            isRunning = true;
            gameButtons[3].setText("Pause", font);
        }
    }

    void testMaze() {
        if (!currentMaze) return;
        bool solvable = pathFinder->isSolvable(currentMaze.get());
        std::cout << "Maze is " << (solvable ? "SOLVABLE" : "NOT SOLVABLE") << std::endl;
    }

    void saveMaze() {
        if (!currentMaze) return;

        std::string filename = currentMazeName + ".json";
        std::ofstream file(filename);
        if (file.is_open()) {
            auto mazeLayout = currentMaze->toStringVector();
            std::string json = SimpleJSON::stringify(mazeLayout, currentMazeName, currentMaze->width, currentMaze->height);
            file << json;
            file.close();
            std::cout << "Maze saved as: " << filename << std::endl;
        }
        else {
            std::cout << "Error saving maze!" << std::endl;
        }
    }

    void resizeMaze() {
        if (!currentMaze) return;

        try {
            int newWidth = std::stoi(mazeWidthInput->getText());
            int newHeight = std::stoi(mazeHeightInput->getText());

            newWidth = std::max(5, std::min(30, newWidth));
            newHeight = std::max(5, std::min(30, newHeight));

            currentMaze->resize(newWidth, newHeight);
            playerRobot->setPosition(currentMaze->startPos);
            state = GameState::IDLE;
            isRunning = false;
            computePath();
            updateMazePosition();

            std::cout << "Maze resized to: " << newWidth << "x" << newHeight << std::endl;
        }
        catch (...) {
            std::cout << "Invalid size input!" << std::endl;
        }
    }

    void run() {
        sf::RenderWindow window(sf::VideoMode(800, 600), "Robot A* Simulation", sf::Style::Titlebar | sf::Style::Close);
        window.setFramerateLimit(60);
        sf::Clock deltaClock;

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }

                if (appState == AppState::MAIN_MENU) {
                    handleMenuEvents(event, window);
                }
                else if (appState == AppState::OPTIONS) {
                    handleOptionsEvents(event, window);
                }
                else if (appState == AppState::GAME) {
                    handleGameEvents(event, window);
                }
            }

            float dt = deltaClock.restart().asSeconds();

            if (appState == AppState::GAME) {
                updateGame(dt);
            }

            window.clear(sf::Color(40, 40, 40));

            if (appState == AppState::MAIN_MENU) {
                drawMainMenu(window);
            }
            else if (appState == AppState::OPTIONS) {
                drawOptionsMenu(window);
            }
            else if (appState == AppState::GAME) {
                drawGame(window);
            }

            window.display();
        }
    }

private:
    void handleMenuEvents(sf::Event& event, sf::RenderWindow& window) {
        if (event.type == sf::Event::MouseMoved) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y));
            for (auto& button : menuButtons) {
                button.setHovered(button.contains(mousePos));
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
                static_cast<float>(event.mouseButton.y));

            if (menuButtons.size() > 0 && menuButtons[0].contains(mousePos)) {
                appState = AppState::GAME;
                loadLevel();
            }
            else if (menuButtons.size() > 1 && menuButtons[1].contains(mousePos)) {
                appState = AppState::OPTIONS;
            }
            else if (menuButtons.size() > 2 && menuButtons[2].contains(mousePos)) {
                window.close();
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            window.close();
        }
    }

    void handleOptionsEvents(sf::Event& event, sf::RenderWindow& window) {
        if (event.type == sf::Event::MouseMoved) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y));

            for (auto& button : optionButtons) {
                button.setHovered(button.contains(mousePos));
            }

            // Handle slider dragging
            for (auto& slider : optionSliders) {
                if (slider->isDragging()) {
                    slider->setValueFromMouse(mousePos);
                }
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
                static_cast<float>(event.mouseButton.y));

            // Check buttons
            if (optionButtons.size() > 0 && optionButtons[0].contains(mousePos)) {
                appState = AppState::MAIN_MENU;
            }
            else if (optionButtons.size() > 1 && optionButtons[1].contains(mousePos)) {
                showExploredCells = !showExploredCells;
                optionButtons[1].setText(showExploredCells ? "Explored: ON" : "Explored: OFF", font);
            }
            else if (optionButtons.size() > 2 && optionButtons[2].contains(mousePos)) {
                showPath = !showPath;
                optionButtons[2].setText(showPath ? "Path: ON" : "Path: OFF", font);
            }

            // Check sliders
            for (auto& slider : optionSliders) {
                if (slider->contains(mousePos)) {
                    slider->setDragging(true);
                    slider->setValueFromMouse(mousePos);

                    // Apply changes immediately
                    if (slider.get() == optionSliders[0].get()) {
                        robotSpeed = slider->getValue();
                        if (playerRobot) {
                            playerRobot->setMoveDuration(robotSpeed);
                        }
                    }
                    else if (slider.get() == optionSliders[1].get()) {
                        cellSizeValue = slider->getValue();
                        CELL_SIZE = cellSizeValue;
                        updateMazePosition();
                    }
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            for (auto& slider : optionSliders) {
                slider->setDragging(false);
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            appState = AppState::MAIN_MENU;
        }
    }

    void handleGameEvents(sf::Event& event, sf::RenderWindow& window) {
        if (event.type == sf::Event::MouseMoved) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x),
                static_cast<float>(event.mouseMove.y));

            for (auto& button : gameButtons) {
                button.setHovered(button.contains(mousePos));
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x),
                static_cast<float>(event.mouseButton.y));

            // Check control panel buttons
            if (gameButtons.size() > 0 && gameButtons[0].contains(mousePos)) {
                zoomIn();
            }
            else if (gameButtons.size() > 1 && gameButtons[1].contains(mousePos)) {
                zoomOut();
            }
            else if (gameButtons.size() > 2 && gameButtons[2].contains(mousePos)) {
                generateMaze();
            }
            else if (gameButtons.size() > 3 && gameButtons[3].contains(mousePos)) {
                toggleRunPause();
            }
            else if (gameButtons.size() > 4 && gameButtons[4].contains(mousePos)) {
                testMaze();
            }
            else if (gameButtons.size() > 5 && gameButtons[5].contains(mousePos)) {
                saveMaze();
            }
            else if (gameButtons.size() > 6 && gameButtons[6].contains(mousePos)) {
                resizeMaze();
            }
            else if (gameButtons.size() > 7 && gameButtons[7].contains(mousePos)) {
                appState = AppState::MAIN_MENU;
            }

            // Handle text input focus
            if (mazeNameInput->contains(mousePos)) {
                mazeNameInput->setFocused(true);
                mazeWidthInput->setFocused(false);
                mazeHeightInput->setFocused(false);
            }
            else if (mazeWidthInput->contains(mousePos)) {
                mazeNameInput->setFocused(false);
                mazeWidthInput->setFocused(true);
                mazeHeightInput->setFocused(false);
            }
            else if (mazeHeightInput->contains(mousePos)) {
                mazeNameInput->setFocused(false);
                mazeWidthInput->setFocused(false);
                mazeHeightInput->setFocused(true);
            }
            else {
                mazeNameInput->setFocused(false);
                mazeWidthInput->setFocused(false);
                mazeHeightInput->setFocused(false);
            }
        }

        if (event.type == sf::Event::TextEntered) {
            mazeNameInput->handleTextEntered(event.text.unicode);
            mazeWidthInput->handleTextEntered(event.text.unicode);
            mazeHeightInput->handleTextEntered(event.text.unicode);

            // Update maze name when text changes
            currentMazeName = mazeNameInput->getText();
            if (currentMazeName.empty()) {
                currentMazeName = "My Maze";
            }
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::R) {
                loadLevel();
            }
            if (event.key.code == sf::Keyboard::Escape) {
                appState = AppState::MAIN_MENU;
            }
        }
    }

    void updateGame(float dt) {
        if (isRunning && state == GameState::SOLVING && !playerRobot->isMoving() && pathIndex < solutionPath.size()) {
            playerRobot->moveTo(solutionPath[pathIndex]);
            pathIndex++;
        }

        playerRobot->update(dt);

        if (state == GameState::SOLVING && playerRobot->getPosition() == currentMaze->endPos) {
            state = GameState::COMPLETE;
            playerRobot->setState(RobotState::COMPLETED);
            isRunning = false;
            gameButtons[3].setText("Run", font);
            std::cout << "Target Reached! Steps: " << playerRobot->getSteps() << std::endl;
        }
    }

    void drawMainMenu(sf::RenderWindow& window) {
        if (!fontLoaded) {
            // Draw simple fallback
            sf::RectangleShape rect(sf::Vector2f(400, 100));
            rect.setPosition(200, 250);
            rect.setFillColor(sf::Color::Green);
            window.draw(rect);
            return;
        }

        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        titleText.setPosition(400.0f, 150.0f);
        window.draw(titleText);

        for (const auto& button : menuButtons) {
            button.draw(window);
        }
    }

    void drawOptionsMenu(sf::RenderWindow& window) {
        if (!fontLoaded) return;

        sf::FloatRect titleBounds = optionsTitleText.getLocalBounds();
        optionsTitleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
        optionsTitleText.setPosition(400.0f, 80.0f);
        window.draw(optionsTitleText);

        for (const auto& slider : optionSliders) {
            slider->draw(window);
        }

        for (const auto& button : optionButtons) {
            button.draw(window);
        }
    }

    void drawGame(sf::RenderWindow& window) {
        // Draw control panel background
        sf::RectangleShape panel(sf::Vector2f(200, 600));
        panel.setPosition(600, 0);
        panel.setFillColor(sf::Color(50, 50, 50));
        window.draw(panel);

        // Draw title
        gameTitleText.setPosition(610, 30);
        window.draw(gameTitleText);

        // Draw control panel buttons
        for (const auto& button : gameButtons) {
            button.draw(window);
        }

        // Draw text inputs
        mazeNameInput->draw(window);
        mazeWidthInput->draw(window);
        mazeHeightInput->draw(window);

        // Draw maze centered
        drawMaze(window);
        if (showPath) {
            drawPathOverlay(window);
        }
        if (showExploredCells) {
            drawExploredCells(window);
        }
        drawRobot(window);
    }

    void drawMaze(sf::RenderWindow& window) {
        if (!currentMaze) return;

        sf::RectangleShape cellShape(sf::Vector2f(CELL_SIZE - 2.0f, CELL_SIZE - 2.0f));
        for (int y = 0; y < currentMaze->height; ++y) {
            for (int x = 0; x < currentMaze->width; ++x) {
                CellType t = currentMaze->grid[y][x]->getType();
                cellShape.setPosition(x * CELL_SIZE + mazeOffset.x + 1.0f,
                    y * CELL_SIZE + mazeOffset.y + 1.0f);
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

    void drawExploredCells(sf::RenderWindow& window) {
        if (!currentMaze) return;

        sf::RectangleShape exploredShape(sf::Vector2f(CELL_SIZE - 6.0f, CELL_SIZE - 6.0f));
        exploredShape.setFillColor(sf::Color(180, 180, 180, 160));
        for (const Point& p : pathFinder->getExplored()) {
            CellType t = currentMaze->grid[p.y][p.x]->getType();
            if (t == CellType::WALL || t == CellType::START || t == CellType::END) continue;
            exploredShape.setPosition(p.x * CELL_SIZE + mazeOffset.x + 3.0f,
                p.y * CELL_SIZE + mazeOffset.y + 3.0f);
            window.draw(exploredShape);
        }
    }

    void drawPathOverlay(sf::RenderWindow& window) {
        if (!currentMaze || solutionPath.empty()) return;

        sf::RectangleShape pathShape(sf::Vector2f(CELL_SIZE - 8.0f, CELL_SIZE - 8.0f));
        pathShape.setFillColor(sf::Color(220, 220, 100, 200));
        for (const Point& p : solutionPath) {
            CellType t = currentMaze->grid[p.y][p.x]->getType();
            if (t == CellType::WALL) continue;
            pathShape.setPosition(p.x * CELL_SIZE + mazeOffset.x + 4.0f,
                p.y * CELL_SIZE + mazeOffset.y + 4.0f);
            window.draw(pathShape);
        }
    }

    void drawRobot(sf::RenderWindow& window) {
        if (!currentMaze) return;

        sf::Vector2f floatPos = playerRobot->getFloatPos(CELL_SIZE);
        float radius = CELL_SIZE / 3.0f;
        sf::CircleShape robotShape(radius);
        robotShape.setFillColor(sf::Color::Blue);
        robotShape.setOutlineThickness(2);
        robotShape.setOutlineColor(sf::Color::White);

        float centerX = floatPos.x + mazeOffset.x + (CELL_SIZE / 2.0f - robotShape.getRadius());
        float centerY = floatPos.y + mazeOffset.y + (CELL_SIZE / 2.0f - robotShape.getRadius());
        robotShape.setPosition(centerX, centerY);
        window.draw(robotShape);
    }
};

int main() {
    GameEngine engine;
    engine.run();
    return 0;
}