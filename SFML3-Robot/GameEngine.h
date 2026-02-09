#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>

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
#include "TrainingVisualizer.h"  
#include "PerformanceDashboard.h"
#include "TextureManager.h"
#include "ControlPanelWidget.h"
#include "SoundManager.h"
#include "MazeBrowserWindow.h"

class GameEngine
{
private:
    void setupTexturesUI();          // La déclaration de la fonction

    // --- SYSTEME DE FOND (VIDEO & STATIC) - AJOUTÉ ---
    std::vector<sf::Texture> videoFrames; // Sequence d'images pour le menu
    sf::Sprite videoSprite;               // Sprite pour afficher la video
    int currentFrame = 0;                 // Frame actuelle
    sf::Clock videoClock;                 // Timer pour l'animation
    float timePerFrame = 0.04f;           // Vitesse (0.04s = 25 FPS)

    sf::Texture bgStaticTexture;          // Texture pour Jeu/Options
    sf::Sprite bgStaticSprite;            // Sprite pour Jeu/Options
    // ------------------------------------------------

    Button texturePrevButton;        // Les boutons
    Button textureNextButton;
    Button textureSelectButton;
    std::vector<sf::Texture> robotTextures;
    int currentTextureIndex;

    // Anciennes variables (on peut les garder ou les ignorer, j'utilise les nouvelles au-dessus)
    sf::Sprite m_bgSprite;
    sf::Texture m_bgTexture;

    // --- CŒUR DU JEU ---
    std::unique_ptr<Maze> currentMaze;
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

    ControlPanelWidget optionsTexturePanel;

    // --- VARIABLES SAUVEGARDÉES (Pour toggle edit mode) ---
    Point savedRobotPos;
    RobotState savedRobotState;

    // --- CONFIGURATION & PARAMÈTRES ---
    Config config;
    float robotSpeed = Constants::DEFAULT_ROBOT_SPEED;
    float cellSizeValue = Constants::DEFAULT_CELL_SIZE;
    bool showExploredCells = true;
    bool showPath = true;

    // Option pour garder la position du robot 
    bool preserveRobotState = false;

    void persistTextureConfig();

    std::string currentMazeName = "My Maze";
    float CELL_SIZE = Constants::DEFAULT_CELL_SIZE;

    // --- INTERFACE GRAPHIQUE (SFML) ---
    sf::Font font;
    bool fontLoaded = false;

    sf::Text titleText;
    sf::Text optionsTitleText;
    sf::Text gameTitleText;

    sf::View worldView;
    sf::View uiView;

    SoundManager soundManager;

    // Sound UI elements
    std::unique_ptr<Slider> musicVolumeSlider;
    std::unique_ptr<Slider> sfxVolumeSlider;
    Button musicMuteButton;
    Button sfxMuteButton;
    Button musicTestButton;
    Button sfxTestButton;
    Button backgroundMusicControlButton;
    sf::Text musicStatusText;

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

    // texute manager
    TextureManager textureManager;

    void applyTexturesFromManager();
    // control panel widget
    ControlPanelWidget controlPanel;

    // Enum pour identifier les onglets
    enum class OptionsTab { SETTINGS, TEXTURES, SOUND, MY_MAZES };

    // Assurez-vous que l'onglet par défaut est bien SETTINGS
    OptionsTab currentOptionTab = OptionsTab::SETTINGS;

    // Liste des boutons pour les onglets
    std::vector<Button> optionTabButtons;

    void updateDashboards();

    std::unique_ptr<TrainingVisualizer> trainingVisualizer;

    // Add toggle button
    Button dashboardToggleButton;
    bool dashboardVisible = true;

    // Timer pour les mises à jour
    sf::Clock updateClock;
    float updateInterval;

    // Système de navigateur de labyrinthes
    MazeBrowserWindow mazeBrowserWindow;
    bool mazeBrowserVisible = false;

    // Messages temporaires
    sf::Text saveMessage;
    sf::Text errorMessage;
    sf::Clock messageTimer;
    bool showMessage = false;
    bool isErrorMessage = false;

    // Double-clic pour le navigateur de labyrinthes
    sf::Clock doubleClickClock;
    sf::Vector2f lastClickPosition;
    float doubleClickThreshold = 0.3f; // 300ms

    // Pour les titres des sections
    std::vector<sf::Text> sectionTitles;

    // Helper pour créer un titre
    void createSectionTitle(const std::string& title, float x, float y);

    sf::RectangleShape controlPanelBackground;

public:
    GameEngine();
    void run();

    // Méthode pour passer du mode Jeu au mode Édition
    void toggleEditMode();

    // Setter pour changer d'outil
    void setTool(EditorTool tool);

    void showTemporaryMessage(const std::string& message, bool isError);

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

    void setupSoundUI();
    void updateMusicStatusText();
};