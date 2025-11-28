#pragma once
#include "Maze.h"
#include "Robot.h"
#include "Enums.h"
#include "AStar.h"
#include "Button.h"
#include "Slider.h"
#include "TextInput.h"
#include "Constants.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class GameEngine {
private:
    std::unique_ptr<Maze> currentMaze;
    std::unique_ptr<Robot> playerRobot;
    std::unique_ptr<PathFinder> pathFinder;
    GameState state = GameState::IDLE;
    AppState appState = AppState::MAIN_MENU;

    float CELL_SIZE = Constants::DEFAULT_CELL_SIZE;
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
    float robotSpeed = Constants::DEFAULT_ROBOT_SPEED;
    float cellSizeValue = Constants::DEFAULT_CELL_SIZE;
    bool showExploredCells = true;
    bool showPath = true;
    std::string currentMazeName = "My Maze";

    // Run/Pause state
    bool isRunning = false;

public:
    GameEngine();
    void run();

private:
    void setupMainMenu();
    void setupOptionsMenu();
    void setupGameUI();
    void loadLevel();
    void updateMazePosition();
    void computePath();
    void zoomIn();
    void zoomOut();
    void generateMaze();
    void toggleRunPause();
    void testMaze();
    void saveMaze();
    void resizeMaze();

    void handleMenuEvents(sf::Event& event, sf::RenderWindow& window);
    void handleOptionsEvents(sf::Event& event, sf::RenderWindow& window);
    void handleGameEvents(sf::Event& event, sf::RenderWindow& window);
    void updateGame(float dt);

    void drawMainMenu(sf::RenderWindow& window);
    void drawOptionsMenu(sf::RenderWindow& window);
    void drawGame(sf::RenderWindow& window);
    void drawMaze(sf::RenderWindow& window);
    void drawExploredCells(sf::RenderWindow& window);
    void drawPathOverlay(sf::RenderWindow& window);
    void drawRobot(sf::RenderWindow& window);
};