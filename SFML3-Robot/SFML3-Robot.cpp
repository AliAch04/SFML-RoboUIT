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

 //// 1. DATA STRUCTURES ////

enum class CellType { EMPTY, WALL, START, END, SPECIAL };
enum class GameState { IDLE, SOLVING, COMPLETE, FAILED };
enum class RobotState { IDLE, CALCULATING, MOVING, COMPLETED };

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
};

//// 7. GAME ENGINE ////

class GameEngine {
private:
    std::unique_ptr<Maze> currentMaze;
    std::unique_ptr<Robot> playerRobot;
    std::unique_ptr<PathFinder> pathFinder;
    GameState state = GameState::IDLE;
    AppState appState = AppState::MAIN_MENU;

    float CELL_SIZE = 60.0f;
    std::vector<Point> solutionPath;
    size_t pathIndex = 0;

    sf::Font font;
    std::vector<Button> menuButtons;
    std::vector<Button> optionButtons;
    std::vector<std::unique_ptr<Slider>> optionSliders;
    sf::Text titleText;
    sf::Text optionsTitleText;
    bool fontLoaded = false;

    // Configurable parameters
    float robotSpeed = 0.3f;
    float cellSizeValue = 60.0f;
    bool showExploredCells = true;
    bool showPath = true;

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
            setupMainMenu();
            setupOptionsMenu();
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
        optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(250, 220), 300, 40.0f, 100.0f, cellSizeValue, "Cell Size", font));

        // Toggle buttons for boolean options
        optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, 290),
            showExploredCells ? "Explored: ON" : "Explored: OFF", font, 18);
        optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, 350),
            showPath ? "Path: ON" : "Path: OFF", font, 18);
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

        // Apply settings
        playerRobot->setMoveDuration(robotSpeed);
        CELL_SIZE = cellSizeValue;

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
            pathIndex = 0;
            if (!solutionPath.empty() && solutionPath[0] == currentMaze->startPos) pathIndex = 1;
            playerRobot->setPosition(currentMaze->startPos);
        }
    }

    void run() {
        sf::RenderWindow window(sf::VideoMode(800, 600), "Robot A* Simulation");
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
                else {
                    handleGameEvents(event);
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
            else {
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

    void handleGameEvents(sf::Event& event) {
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
        if (state == GameState::SOLVING && !playerRobot->isMoving() && pathIndex < solutionPath.size()) {
            playerRobot->moveTo(solutionPath[pathIndex]);
            pathIndex++;
        }

        playerRobot->update(dt);

        if (state == GameState::SOLVING && playerRobot->getPosition() == currentMaze->endPos) {
            state = GameState::COMPLETE;
            playerRobot->setState(RobotState::COMPLETED);
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
        sf::RectangleShape cellShape(sf::Vector2f(CELL_SIZE - 2.0f, CELL_SIZE - 2.0f));
        for (int y = 0; y < currentMaze->height; ++y) {
            for (int x = 0; x < currentMaze->width; ++x) {
                CellType t = currentMaze->grid[y][x]->getType();
                cellShape.setPosition(x * CELL_SIZE + 1.0f, y * CELL_SIZE + 1.0f);
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
        sf::RectangleShape exploredShape(sf::Vector2f(CELL_SIZE - 6.0f, CELL_SIZE - 6.0f));
        exploredShape.setFillColor(sf::Color(180, 180, 180, 160));
        for (const Point& p : pathFinder->getExplored()) {
            CellType t = currentMaze->grid[p.y][p.x]->getType();
            if (t == CellType::WALL || t == CellType::START || t == CellType::END) continue;
            exploredShape.setPosition(p.x * CELL_SIZE + 3.0f, p.y * CELL_SIZE + 3.0f);
            window.draw(exploredShape);
        }
    }

    void drawPathOverlay(sf::RenderWindow& window) {
        if (!solutionPath.empty()) {
            sf::RectangleShape pathShape(sf::Vector2f(CELL_SIZE - 8.0f, CELL_SIZE - 8.0f));
            pathShape.setFillColor(sf::Color(220, 220, 100, 200));
            for (const Point& p : solutionPath) {
                CellType t = currentMaze->grid[p.y][p.x]->getType();
                if (t == CellType::WALL) continue;
                pathShape.setPosition(p.x * CELL_SIZE + 4.0f, p.y * CELL_SIZE + 4.0f);
                window.draw(pathShape);
            }
        }
    }

    void drawRobot(sf::RenderWindow& window) {
        sf::Vector2f floatPos = playerRobot->getFloatPos(CELL_SIZE);
        float radius = CELL_SIZE / 3.0f;
        sf::CircleShape robotShape(radius);
        robotShape.setFillColor(sf::Color::Blue);
        robotShape.setOutlineThickness(2);
        robotShape.setOutlineColor(sf::Color::White);

        float centerX = floatPos.x + (CELL_SIZE / 2.0f - robotShape.getRadius());
        float centerY = floatPos.y + (CELL_SIZE / 2.0f - robotShape.getRadius());
        robotShape.setPosition(centerX, centerY);
        window.draw(robotShape);
    }
};

int main() {
    GameEngine engine;
    engine.run();
    return 0;
}