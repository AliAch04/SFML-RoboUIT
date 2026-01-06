#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>

// Tes includes
#include "Logger.h"
#include "Config.h"
#include "Maze.h"
#include "Robot.h"
#include "Enums.h"
#include "AStar.h"
#include "Button.h"
#include "Slider.h"
#include "TextInput.h"
#include "Constants.h"
#include "EditorToolbar.h"
#include "MazeEditor.h"
#include "LearningRobot.h"


class GameEngine
{
private:
    // --- CŒUR DU JEU ---
    std::unique_ptr<Maze> currentMaze;
    // std::unique_ptr<Robot> playerRobot;
    std::unique_ptr<LearningRobot> playerRobot;
    std::unique_ptr<PathFinder> pathFinder;

    // NOUVEAU : Pointeur intelligent vers l'éditeur (Important !)
    std::unique_ptr<MazeEditor> mazeEditor;

    // --- ÉTATS ---
    GameState state = GameState::IDLE;
    AppState appState = AppState::MAIN_MENU;
    bool isRunning = false;

    // --- MODE ÉDITION ---
    EditorToolbar editorToolbar;
    EditorTool currentTool = EditorTool::WALL; // Une seule déclaration

    // --- VARIABLES SAUVEGARDÉES (Pour toggle edit mode) ---
    Point savedRobotPos;
    RobotState savedRobotState;

    // --- CONFIGURATION & PARAMÈTRES ---
    Config config;
    float robotSpeed = Constants::DEFAULT_ROBOT_SPEED;
    float cellSizeValue = Constants::DEFAULT_CELL_SIZE;
    bool showExploredCells = true;
    bool showPath = true;

    // --- NOUVEAU : Option pour garder la position du robot ---
    bool preserveRobotState = false;
    // --------------------------------------------------------

    std::string currentMazeName = "My Maze";
    float CELL_SIZE = Constants::DEFAULT_CELL_SIZE;

    // --- INTERFACE GRAPHIQUE (SFML) ---
    sf::Font font;
    bool fontLoaded = false;

    sf::Text titleText;
    sf::Text optionsTitleText;
    sf::Text gameTitleText;
    // --- WORLDVIEW AND UIVIEW  ---




    sf::View worldView;
    sf::View uiView;
    // --- drag navigation  ---


    


    bool isPanning = false;
    sf::Vector2f lastMousePos;
    float zoomLevel = 1.0f;


    // Conteneurs UI
    std::vector<Button> menuButtons;
    std::vector<Button> optionButtons;
    std::vector<Button> gameButtons;
    std::vector<std::unique_ptr<Slider>> optionSliders;

    std::unique_ptr<TextInput> mazeNameInput;
    std::unique_ptr<TextInput> mazeWidthInput;
    std::unique_ptr<TextInput> mazeHeightInput;

    // Positionnement pour centrer le labyrinthe
    sf::Vector2f mazeOffset;

    // Algorithme Pathfinding
    std::vector<Point> solutionPath;
    size_t pathIndex = 0;

    // robot graphics
    sf::Texture robotTexture;
    sf::Sprite robotSprite;
    
    // Wall & obstacle textures and floors
    sf::Texture wallTexture;
    sf::Sprite wallSprite;

    sf::Texture obstacleTexture;
    sf::Sprite obstacleSprite;

    sf::Texture floorTexture;
    sf::Sprite  floorSprite;
    bool floorLoaded = true; // assume true if load succeeded

    // Q-leaning robot
    sf::Text learningScoreText;
    sf::Text successRateText;
    sf::Text explorationRateText;
    sf::RectangleShape learningPanel;

    
    void updateLearningUI();
    void drawLearningInfo(sf::RenderWindow& window);
    void saveLearningModel();
    void loadLearningModel();

public:
    GameEngine();
    void run();

    // Méthode pour passer du mode Jeu au mode Édition
    void toggleEditMode();

    // Setter pour changer d'outil
    void setTool(EditorTool tool);

private:
    // Initialisation
    void setupMainMenu();
    void setupOptionsMenu();
    void setupGameUI();

    // Logique interne
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

    // Gestion des événements
    void handleMenuEvents(sf::Event& event, sf::RenderWindow& window);
    void handleOptionsEvents(sf::Event& event, sf::RenderWindow& window);
    void handleGameEvents(sf::Event& event, sf::RenderWindow& window);

    // Update & Draw
    void updateGame(float dt);
    void drawMainMenu(sf::RenderWindow& window);
    void drawOptionsMenu(sf::RenderWindow& window);
    void drawGame(sf::RenderWindow& window);

    // Dessin des composants du jeu
    void drawMaze(sf::RenderWindow& window);
    void drawExploredCells(sf::RenderWindow& window);
    void drawPathOverlay(sf::RenderWindow& window);
    void drawRobot(sf::RenderWindow& window);
};