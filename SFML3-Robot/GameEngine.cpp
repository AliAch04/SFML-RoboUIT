#include "GameEngine.h"
#include "SimpleJSON.h"
#include "Logger.h"
#include "Config.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm> // Pour std::max, std::min, std::find
#include <memory>

// -------------------------------------------------------------------------
// CONSTRUCTEUR
// -------------------------------------------------------------------------
GameEngine::GameEngine() :
    //playerRobot(std::make_unique<Robot>()),
    playerRobot(std::make_unique<LearningRobot>()),
    pathFinder(std::make_unique<AStar>()),
    savedRobotPos({ 0, 0 }),
    savedRobotState(RobotState::IDLE),
    updateInterval(0.1f)  ,

    musicMuteButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Mute", font, 14),
    sfxMuteButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Mute", font, 14),
    musicTestButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Test", font, 14),
    sfxTestButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Test", font, 14)
{
    Logger::info("GameEngine initialized");
    
    // Load robot texture
    if (!robotTexture.loadFromFile("assets/textures/robot.png")) {
        std::cout << "Failed to load robot texture!" << std::endl;
    }
    robotSprite.setTexture(robotTexture);
    robotTexture.setSmooth(true);

    // Load wall texture
    if (!wallTexture.loadFromFile("assets/textures/wall.png")) {
        std::cout << "Failed to load wall texture!" << std::endl;
    }
    else {
        wallTexture.setSmooth(true);
        wallSprite.setTexture(wallTexture);
    }

    // Load obstacle texture
    if (!obstacleTexture.loadFromFile("assets/textures/obstacle.png")) {
        std::cout << "Failed to load obstacle texture!" << std::endl;
    }
    else {
        obstacleTexture.setSmooth(true);
        obstacleSprite.setTexture(obstacleTexture);
    }
    
    // load floor texture
    if (!floorTexture.loadFromFile("assets/textures/floor.png")) {
        std::cout << "Failed to load floor texture!" << std::endl;
    }
    else {
        floorTexture.setSmooth(true);
        floorSprite.setTexture(floorTexture);
        std::cout << "Floor loaded: "
            << floorTexture.getSize().x << "x"
            << floorTexture.getSize().y << std::endl;
    }

    // config system link
    config.load("config.txt");

    robotSpeed = config.robotSpeed;
    cellSizeValue = config.cellSize;
    showExploredCells = config.showExploredCells;
    showPath = config.showPath;

    // Initialize sound manager with config values
    std::cout << "\n=== INITIALIZING AUDIO SYSTEM ===" << std::endl;
    soundManager.initialize(config.musicVolume, config.sfxVolume);

    // Apply mute states
    if (config.musicMuted) {
        std::cout << "Music was muted in config" << std::endl;
        soundManager.muteMusic(true);
    }
    if (config.sfxMuted) {
        std::cout << "SFX was muted in config" << std::endl;
        soundManager.muteSFX(true);
    }

    // Check if background music is playing
    if (soundManager.isBackgroundMusicPlaying()) {
        std::cout << "✓ Background music is playing" << std::endl;
    }
    else {
        std::cout << "✗ Background music is NOT playing" << std::endl;
        // Try to start it manually
        soundManager.startBackgroundMusic();
    }

    std::cout << "Music Volume: " << soundManager.getMusicVolume() << std::endl;
    std::cout << "SFX Volume: " << soundManager.getSFXVolume() << std::endl;
    std::cout << "=== AUDIO INITIALIZATION COMPLETE ===\n" << std::endl;

    // -------------------- TEXTURE MANAGER INIT (STEP 3) --------------------
    textureManager.setDefaults(
        "assets/textures/robot.png",
        "assets/textures/wall.png",
        "assets/textures/floor.png",
        "assets/textures/obstacle.png"
    );

    // take persisted paths from config (STEP 1 keys)
    textureManager.get(TextureManager::Id::Robot).currentPath = config.robotTexturePath;
    textureManager.get(TextureManager::Id::Wall).currentPath = config.wallTexturePath;
    textureManager.get(TextureManager::Id::Floor).currentPath = config.floorTexturePath;
    textureManager.get(TextureManager::Id::Obstacle).currentPath = config.obstacleTexturePath;

    // load them all
    textureManager.loadAll();

    // Bind loaded textures to your existing textures/sprites.
    // If a texture fails to load, fallback to default.
    if (!textureManager.get(TextureManager::Id::Robot).loaded)    textureManager.reset(TextureManager::Id::Robot);
    if (!textureManager.get(TextureManager::Id::Wall).loaded)     textureManager.reset(TextureManager::Id::Wall);
    if (!textureManager.get(TextureManager::Id::Floor).loaded)    textureManager.reset(TextureManager::Id::Floor);
    if (!textureManager.get(TextureManager::Id::Obstacle).loaded) textureManager.reset(TextureManager::Id::Obstacle);

    // Copy into your current sf::Texture members (minimal change)
    robotTexture = textureManager.get(TextureManager::Id::Robot).texture;
    wallTexture = textureManager.get(TextureManager::Id::Wall).texture;
    floorTexture = textureManager.get(TextureManager::Id::Floor).texture;
    obstacleTexture = textureManager.get(TextureManager::Id::Obstacle).texture;

    // Rebind sprites
    robotSprite.setTexture(robotTexture, true);
    wallSprite.setTexture(wallTexture, true);
    floorSprite.setTexture(floorTexture, true);
    obstacleSprite.setTexture(obstacleTexture, true);

    // your existing bool
    floorLoaded = (floorTexture.getSize().x > 0 && floorTexture.getSize().y > 0);
    // ----------------------------------------------------------------------


    // Initialisation par défaut
    preserveRobotState = false;

    // Chargement des polices
    std::vector<std::string> fontPaths = {
        "arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc" };

    for (const auto& path : fontPaths)
    {
        if (font.loadFromFile(path))
        {
            fontLoaded = true;
            std::cout << "Font loaded from: " << path << std::endl;
            break;
        }
    }

    if (!fontLoaded)
    {
        std::cout << "Warning: Could not load font." << std::endl;
    }

    if (fontLoaded) {
        titleText.setFont(font);
        optionsTitleText.setFont(font);
        gameTitleText.setFont(font);
        setupMainMenu();
        setupOptionsMenu();
        setupGameUI();
        controlPanel.init(font);
        controlPanel.setTextureManager(&textureManager);
        tabControlsBtn = std::make_unique<Button>(sf::Vector2f(90, 28), sf::Vector2f(610, 10), "Controls", font, 14);
        tabTexturesBtn = std::make_unique<Button>(sf::Vector2f(90, 28), sf::Vector2f(710, 10), "Textures", font, 14);



        // AJOUTER L'INITIALISATION DES DASHBOARDS
        /*trainingVisualizer = std::make_unique<TrainingVisualizer>(window, font);
        trainingVisualizer->setPosition(sf::Vector2f(620.0f, 350.0f));
        trainingVisualizer->setSize(350.0f, 200.0f);

        performanceDashboard = std::make_unique<PerformanceDashboard>(window, font);
        performanceDashboard->setPosition(sf::Vector2f(620.0f, 80.0f));
        performanceDashboard->setSize(350.0f, 250.0f);*/

        // Lier les algorithmes au dashboard
        auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());
        if (learningRobot) {
            // Le dashboard sera mis à jour via updateDashboards()
        }
    }
}

void GameEngine::updateDashboards() {
    auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());
    if (!learningRobot) return;

    // Mettre à jour le Training Visualizer
    if (trainingVisualizer) {
        double loss = 0.0;  // Placeholder - à remplacer par vraie valeur
        double reward = learningRobot->getTotalReward();
        double successRate = learningRobot->getSuccessRate();
        int trainingSteps = learningRobot->getTotalTrials();

        trainingVisualizer->update(loss, reward, successRate, trainingSteps);
    }

    // Mettre à jour le Performance Dashboard
    if (performanceDashboard) {
        double optimality = learningRobot->getEvolutionaryOptimality();
        double convergence = learningRobot->getEvolutionaryConvergence();
        double adaptability = learningRobot->getEvolutionaryAdaptability();

        performanceDashboard->addPerformanceData(optimality, convergence, adaptability);
        performanceDashboard->setGenerationInfo(
            learningRobot->getCurrentGeneration(),
            learningRobot->getMaxGenerations(),
            learningRobot->getCurrentStrategy()
        );
    }
}

void GameEngine::setupMainMenu()
{
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

void GameEngine::setupOptionsMenu()
{
    if (!fontLoaded) return;

    // --- 1. TITRE ---
    optionsTitleText.setString("OPTIONS");
    optionsTitleText.setCharacterSize(48);
    optionsTitleText.setFillColor(sf::Color::White);
    optionsTitleText.setStyle(sf::Text::Bold);

    // --- 2. CRÉATION DES 4 ONGLETS ---
    optionTabButtons.clear();

    // On réduit un peu la largeur pour faire tenir 4 boutons
    float tabW = 140.0f;
    float tabH = 40.0f;
    float gap = 10.0f;
    // Centrage approximatif : (800 - (4*140 + 3*10)) / 2 = ~105
    float startX = 100.0f;
    float tabY = 150.0f;

    // Onglet 0: SETTINGS (Paramètres généraux déplacés ici)
    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX, tabY), "SETTINGS", font, 16);

    // Onglet 1: TEXTURES (Sera pour le choix des textures)
    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX + tabW + gap, tabY), "TEXTURES", font, 16);

    // Onglet 2: SOUND
    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX + (tabW + gap) * 2, tabY), "SOUND", font, 16);

    // Onglet 3: MY MAZES
    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX + (tabW + gap) * 3, tabY), "MY MAZES", font, 16);


    // --- 3. CONTENU (Sliders & Boutons) ---
    optionButtons.clear();
    optionSliders.clear();

    // Bouton BACK (Index 0) - Toujours visible en bas
    optionButtons.emplace_back(sf::Vector2f(150, 40), sf::Vector2f(325, 550), "BACK", font, 20);

    // -- CONTENU POUR L'ONGLET "SETTINGS" --

    // Sliders
    optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(250, 250), 300, 0.05f, 0.5f, robotSpeed, "Robot Speed", font));
    optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(250, 320), 300, 10.0f, 60.0f, CELL_SIZE, "Cell Size", font));

    // Boutons Toggle (Index 1, 2, 3)
    float startY_Toggles = 390.0f;
    float gapToggle = 50.0f;

    // Index 1 : Explored
    optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, startY_Toggles),
        showExploredCells ? "Explored: ON" : "Explored: OFF", font, 18);

    // Index 2 : Path
    optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, startY_Toggles + gapToggle),
        showPath ? "Path: ON" : "Path: OFF", font, 18);

    // Index 3 : Keep Pos
    optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, startY_Toggles + gapToggle * 2),
        preserveRobotState ? "Keep Pos: ON" : "Keep Pos: OFF", font, 18);

    // Add sound UI elements for the SOUND tab
    // These will be created conditionally when SOUND tab is active
    // We'll initialize them with nullptr and create when needed
}


void GameEngine::applyTexturesFromManager()
{
    robotTexture = textureManager.get(TextureManager::Id::Robot).texture;
    wallTexture = textureManager.get(TextureManager::Id::Wall).texture;
    floorTexture = textureManager.get(TextureManager::Id::Floor).texture;
    obstacleTexture = textureManager.get(TextureManager::Id::Obstacle).texture;

    robotSprite.setTexture(robotTexture, true);
    wallSprite.setTexture(wallTexture, true);
    floorSprite.setTexture(floorTexture, true);
    obstacleSprite.setTexture(obstacleTexture, true);

    floorLoaded = (floorTexture.getSize().x > 0 && floorTexture.getSize().y > 0);
}


void GameEngine::setupGameUI()
{
    if (!fontLoaded) return;

    gameTitleText.setString("MAZE SIMULATION");
    gameTitleText.setPosition(630, 30);

    gameButtons.clear();
    float centerX = 640.0f;
    float btnW = 120.0f;
    float btnH = 30.0f;
    float gap = 10.0f;

    // CAMERA
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, 80), "Zoom +", font, 16);
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, 80 + btnH + gap), "Zoom -", font, 16);

    // SIMULATION
    float startY_Sim = 180.0f;
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, startY_Sim), "Generate", font, 16);
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, startY_Sim + (btnH + gap) * 1), "Run", font, 16);
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, startY_Sim + (btnH + gap) * 2), "Tester", font, 16);

    // FICHIER
    float startY_File = 320.0f;
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, startY_File), "Sauver", font, 16);
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, startY_File + (btnH + gap) * 1), "Resize", font, 16);
    gameButtons.emplace_back(sf::Vector2f(btnW, btnH), sf::Vector2f(centerX, startY_File + (btnH + gap) * 2), "Menu", font, 16);

    // INPUTS
    mazeNameInput = std::make_unique<TextInput>(sf::Vector2f(centerX, 450), 120, "Maze Name", font);
    mazeWidthInput = std::make_unique<TextInput>(sf::Vector2f(centerX, 500), 55, "Width", font);
    mazeHeightInput = std::make_unique<TextInput>(sf::Vector2f(centerX + 65, 500), 55, "Height", font);

    // EDITEUR (Index 8)
    gameButtons.emplace_back(sf::Vector2f(btnW, 40), sf::Vector2f(centerX, 550), "Edit Mode", font, 18);

    // UNDO / REDO (Index 9 et 10)
    float undoRedoY = 505.0f;
    gameButtons.emplace_back(sf::Vector2f(55, 30), sf::Vector2f(centerX - 32, undoRedoY), "<<", font, 18);
    gameButtons.emplace_back(sf::Vector2f(55, 30), sf::Vector2f(centerX + 32, undoRedoY), ">>", font, 18);

    editorToolbar.init(font, centerX, 180.0f);

    // (Q-learing) Les textes pour l'apprentissage
    learningScoreText.setFont(font);
    learningScoreText.setCharacterSize(16);
    learningScoreText.setFillColor(sf::Color::Green);
    learningScoreText.setPosition(610, 520);

    successRateText.setFont(font);
    successRateText.setCharacterSize(16);
    successRateText.setFillColor(sf::Color::Cyan);
    successRateText.setPosition(610, 540);

    explorationRateText.setFont(font);
    explorationRateText.setCharacterSize(16);
    explorationRateText.setFillColor(sf::Color::Yellow);
    explorationRateText.setPosition(610, 560);

    // Panel d'apprentissage
    learningPanel.setSize(sf::Vector2f(180, 80));
    learningPanel.setPosition(600, 510);
    learningPanel.setFillColor(sf::Color(30, 30, 30, 200));
    learningPanel.setOutlineThickness(1);
    learningPanel.setOutlineColor(sf::Color::White);

    // Bouton pour l'apprentissage continu
    float startY_Learning = 600.0f;
    gameButtons.emplace_back(
        sf::Vector2f(btnW, btnH),
        sf::Vector2f(centerX, startY_Learning),
        "Auto-Learn",
        font,
        16
    );

    // AJOUTER un bouton pour basculer en mode autonome (index 12)
    float startY_AutonomousMode = 640.0f;
    gameButtons.emplace_back(
        sf::Vector2f(btnW, btnH),
        sf::Vector2f(centerX, startY_AutonomousMode),
        "Autonomous",
        font,
        16
    );
}



// Ajouter cette méthode:
void GameEngine::updateLearningUI() {
    if (!fontLoaded) return;

    std::string scoreStr = "Score Apprentissage: " +
        std::to_string(static_cast<int>(playerRobot->getLearningScore())) + "%";
    learningScoreText.setString(scoreStr);

    std::string successStr = "Taux Reussite: " +
        std::to_string(static_cast<int>(playerRobot->getSuccessRate())) + "%";
    successRateText.setString(successStr);

    // Note: L'exploration rate n'est pas directement accessible dans l'interface publique
    // Vous devrez peut-être ajouter un getter dans QLearning
}

void GameEngine::loadLevel()
{
    std::vector<std::string> levelMap = {
        "##########",
        "#S...#...#",
        "###.####.#",
        "#...#....#",
        "#.###.##.#",
        "#.#....#.#",
        "#.####.#.#",
        "#......#E#",
        "##########" };

    currentMaze = std::make_unique<Maze>(10, 9);

    playerRobot->setMaze(currentMaze.get());
    playerRobot->startNewTrial();

    currentMaze->loadFromMap(levelMap);

    mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
    mazeEditor->setTool(currentTool);

    playerRobot->setPosition(currentMaze->startPos);
    state = GameState::IDLE;
    isRunning = false;

    playerRobot->setMoveDuration(robotSpeed);
    CELL_SIZE = cellSizeValue;

    computePath();
    updateMazePosition();
}

void GameEngine::updateMazePosition()
{
    if (!currentMaze) return;

    float mazeW = currentMaze->width * CELL_SIZE;
    float mazeH = currentMaze->height * CELL_SIZE;

    // Center in the LEFT area (0..600)
    mazeOffset.x = (600.f - mazeW) / 2.f;
    mazeOffset.y = (600.f - mazeH) / 2.f;

    // Clamp so maze stays inside left area with padding
    const float leftPadding = 10.f;
    const float topPadding = 10.f;

    // maxX means: left edge can't go beyond padding
    float maxX = leftPadding;
    // minX means: right edge can't cross x=600-leftPadding
    float minX = 600.f - leftPadding - mazeW;

    // If maze is smaller than area, keep centered
    if (mazeW <= 600.f - 2.f * leftPadding) {
        mazeOffset.x = (600.f - mazeW) / 2.f;
    }
    else {
        mazeOffset.x = std::max(minX, std::min(maxX, mazeOffset.x));
    }

    // For Y clamp within window height
    float windowH = (float)Constants::WINDOW_HEIGHT;
    float maxY = topPadding;
    float minY = windowH - topPadding - mazeH;

    if (mazeH <= windowH - 2.f * topPadding) {
        mazeOffset.y = (windowH - mazeH) / 2.f;
    }
    else {
        mazeOffset.y = std::max(minY, std::min(maxY, mazeOffset.y));
    }
}

void GameEngine::zoomIn()
{
    CELL_SIZE = std::min(Constants::MAX_CELL_SIZE, CELL_SIZE + 5.0f);
    updateMazePosition();
    // Play sound
    soundManager.playSound("test_sfx");
}

void GameEngine::zoomOut()
{
    CELL_SIZE = std::max(Constants::MIN_CELL_SIZE, CELL_SIZE - 5.0f);
    updateMazePosition();
    // Play sound
    soundManager.playSound("test_sfx");
}

// --------------------------------------------------------------------------------
// COMPUTE PATH - VERSION CORRIGÉE ET INTELLIGENTE
// --------------------------------------------------------------------------------
void GameEngine::computePath()
{
    if (!currentMaze)
        return;

    Point originalStartPos = currentMaze->startPos;
    Point robotPos = playerRobot->getPosition();

    // 1. VERIFICATION MANUELLE
    bool isInsideMap = (robotPos.x >= 0 && robotPos.x < currentMaze->width &&
        robotPos.y >= 0 && robotPos.y < currentMaze->height);

    bool isRobotValid = false;

    if (isInsideMap) {
        if (currentMaze->grid[robotPos.y][robotPos.x]->getType() != CellType::WALL) {
            isRobotValid = true;
        }
    }

    // 2. LOGIQUE INTELLIGENTE
    if (isRobotValid && robotPos != currentMaze->endPos)
    {
        // On calcule depuis le robot
        currentMaze->startPos = robotPos;

        pathFinder->clearExplored();
        solutionPath = pathFinder->findPath(currentMaze.get());

        // On remet le vrai départ
        currentMaze->startPos = originalStartPos;

        if (solutionPath.empty())
        {
            std::cout << "Aucun chemin depuis le robot !" << std::endl;
            state = GameState::FAILED;
        }
        else
        {
            std::cout << "Chemin mis a jour depuis la position du robot." << std::endl;
            state = GameState::SOLVING;
            pathIndex = 1;
        }
    }
    else
    {
        // CAS DE SECOURS (Retour case départ)
        if (!isRobotValid && isInsideMap) {
            std::cout << "Robot ecrase par un mur ! Retour au depart." << std::endl;
        }

        playerRobot->setPosition(originalStartPos);
        pathFinder->clearExplored();
        solutionPath = pathFinder->findPath(currentMaze.get());

        if (solutionPath.empty())
        {
            std::cout << "Maze impossible." << std::endl;
            state = GameState::FAILED;
        }
        else
        {
            state = GameState::SOLVING;
            pathIndex = 1;
        }
    }
}

void GameEngine::generateMaze() {
    if (!currentMaze) return;

    try {
        int width = std::stoi(mazeWidthInput->getText());
        int height = std::stoi(mazeHeightInput->getText());
        width = std::max(5, std::min(30, width));
        height = std::max(5, std::min(30, height));

        currentMaze = std::make_unique<Maze>(width, height);
        currentMaze->generateSolvableMaze();

        mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
        mazeEditor->setTool(currentTool);

        playerRobot->setPosition(currentMaze->startPos);

        playerRobot->setMaze(currentMaze.get()); // Conversion

        state = GameState::IDLE;
        isRunning = false;

        computePath();
        updateMazePosition();
        std::cout << "Generated new maze: " << width << "x" << height << std::endl;
    }
    catch (...) {
        std::cout << "Invalid size input for maze generation!" << std::endl;
    }
    // Play sound
    soundManager.playSound("test_sfx");
}

void GameEngine::toggleRunPause() {
    if (!currentMaze) return;

    if (isRunning) {
        playerRobot->pause();
        isRunning = false;
        gameButtons[3].setText("Run", font);
        // Play pause sound
        soundManager.playSound("test_sfx");
    }
    else {
        if (state == GameState::COMPLETE || state == GameState::FAILED) {
            playerRobot->setPosition(currentMaze->startPos);
            pathIndex = 1;
            state = GameState::SOLVING;
        }

        // CORRECTION : Appeler startNewTrial() avant de reprendre
        playerRobot->startNewTrial();  // Nouveau pour l'apprentissage

        playerRobot->resume();
        isRunning = true;
        gameButtons[3].setText("Pause", font);
        // Play start sound
        soundManager.playSound("test_sfx");
    }
}

void GameEngine::testMaze()
{
    if (!currentMaze) return;
    bool solvable = pathFinder->isSolvable(currentMaze.get());
    std::cout << "Maze is " << (solvable ? "SOLVABLE" : "NOT SOLVABLE") << std::endl;
    // Play sound
    soundManager.playSound("test_sfx");
}

void GameEngine::saveMaze()
{
    if (!currentMaze) return;
    std::string filename = currentMazeName + ".json";
    std::ofstream file(filename);
    if (file.is_open())
    {
        auto mazeLayout = currentMaze->toStringVector();
        std::string json = SimpleJSON::stringify(mazeLayout, currentMazeName, currentMaze->width, currentMaze->height);
        file << json;
        file.close();
        std::cout << "Maze saved as: " << filename << std::endl;
    }
    else
    {
        std::cout << "Error saving maze!" << std::endl;
    }
    // Play sound
    soundManager.playSound("test_sfx");
}

void GameEngine::resizeMaze() {
    if (!currentMaze) return;
    try {
        int newWidth = std::stoi(mazeWidthInput->getText());
        int newHeight = std::stoi(mazeHeightInput->getText());
        newWidth = std::max(5, std::min(30, newWidth));
        newHeight = std::max(5, std::min(30, newHeight));

        currentMaze->resize(newWidth, newHeight);

        mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
        mazeEditor->setTool(currentTool);

        playerRobot->setPosition(currentMaze->startPos);

        playerRobot->setMaze(currentMaze.get());  // Conversion

        state = GameState::IDLE;
        isRunning = false;
        computePath();
        updateMazePosition();
        std::cout << "Maze resized to: " << newWidth << "x" << newHeight << std::endl;
    }
    catch (...) {
        std::cout << "Invalid size input!" << std::endl;
    }

    // Play sound
    soundManager.playSound("test_sfx");
}

void GameEngine::run()
{
    sf::RenderWindow window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
        "Robot A* Simulation", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    sf::Clock deltaClock;
    worldView = window.getDefaultView();
    uiView = window.getDefaultView();

    // Test audio initialization
    std::cout << "Audio System Status: " << soundManager.getStatus() << std::endl;

    // INITIALISER LES DASHBOARDS 
    if (fontLoaded) {
        trainingVisualizer = std::make_unique<TrainingVisualizer>(window, font);
        trainingVisualizer->setPosition(sf::Vector2f(620.0f, 350.0f));
        trainingVisualizer->setSize(350.0f, 200.0f);

        performanceDashboard = std::make_unique<PerformanceDashboard>(window, font);
        performanceDashboard->setPosition(sf::Vector2f(620.0f, 80.0f));
        performanceDashboard->setSize(350.0f, 250.0f);
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) window.close();

            if (appState == AppState::MAIN_MENU) handleMenuEvents(event, window);
            else if (appState == AppState::OPTIONS) handleOptionsEvents(event, window);
            else if (appState == AppState::GAME) handleGameEvents(event, window);
        }

        float dt = deltaClock.restart().asSeconds();

        if (appState == AppState::GAME) {
            updateGame(dt);

            // MAJ LES DASHBOARDS 
            if (updateClock.getElapsedTime().asSeconds() >= updateInterval) {
                updateDashboards();
                updateClock.restart();
            }
        }

        window.clear(sf::Color(40, 40, 40));

        if (appState == AppState::MAIN_MENU) drawMainMenu(window);
        else if (appState == AppState::OPTIONS) drawOptionsMenu(window);
        else if (appState == AppState::GAME) drawGame(window);

        window.display();
    }

    // Save sound settings when closing
    config.musicVolume = soundManager.getMusicVolume();
    config.sfxVolume = soundManager.getSFXVolume();
    config.musicMuted = soundManager.isMusicMuted();
    config.sfxMuted = soundManager.isSFXMuted();

    // Save texture paths
    config.robotTexturePath = textureManager.get(TextureManager::Id::Robot).currentPath;
    config.wallTexturePath = textureManager.get(TextureManager::Id::Wall).currentPath;
    config.floorTexturePath = textureManager.get(TextureManager::Id::Floor).currentPath;
    config.obstacleTexturePath = textureManager.get(TextureManager::Id::Obstacle).currentPath;

    // Save simulation settings
    config.robotSpeed = robotSpeed;
    config.cellSize = CELL_SIZE;
    config.showExploredCells = showExploredCells;
    config.showPath = showPath;

    // Save everything to file
    config.save("config.txt");
}

void GameEngine::handleMenuEvents(sf::Event& event, sf::RenderWindow& window)
{
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
        for (auto& button : menuButtons) button.setHovered(button.contains(mousePos));
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));
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
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) window.close();
}

void GameEngine::handleOptionsEvents(sf::Event& event, sf::RenderWindow& window)
{
    // --- SURVOL (HOVER) ---
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));

        // Onglets
        for (auto& btn : optionTabButtons) btn.setHovered(btn.contains(mousePos));

        // Bouton Back
        if (optionButtons.size() > 0) optionButtons[0].setHovered(optionButtons[0].contains(mousePos));

        // Contenu SETTINGS uniquement
        if (currentOptionTab == OptionsTab::SETTINGS) {
            for (size_t i = 1; i < optionButtons.size(); ++i) {
                optionButtons[i].setHovered(optionButtons[i].contains(mousePos));
            }
            for (auto& slider : optionSliders) {
                if (slider->isDragging()) slider->setValueFromMouse(mousePos);
            }
        }
    }

    // --- CLIC (MOUSE PRESSED) ---
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        // 1. Clic sur les ONGLETS
        for (size_t i = 0; i < optionTabButtons.size(); ++i) {
            if (optionTabButtons[i].contains(mousePos)) {
                if (i == 0) currentOptionTab = OptionsTab::SETTINGS;
                else if (i == 1) currentOptionTab = OptionsTab::TEXTURES;
                else if (i == 2) currentOptionTab = OptionsTab::SOUND;
                else if (i == 3) currentOptionTab = OptionsTab::MY_MAZES;
                return;
            }
        }

        // 2. Bouton BACK
        if (optionButtons.size() > 0 && optionButtons[0].contains(mousePos)) {
            appState = AppState::MAIN_MENU;
        }

        // 3. Interactions onglet SETTINGS
        if (currentOptionTab == OptionsTab::SETTINGS) {

            // Boutons Toggle
            if (optionButtons.size() > 1 && optionButtons[1].contains(mousePos)) {
                showExploredCells = !showExploredCells;
                optionButtons[1].setText(showExploredCells ? "Explored: ON" : "Explored: OFF", font);
            }
            else if (optionButtons.size() > 2 && optionButtons[2].contains(mousePos)) {
                showPath = !showPath;
                optionButtons[2].setText(showPath ? "Path: ON" : "Path: OFF", font);
            }
            else if (optionButtons.size() > 3 && optionButtons[3].contains(mousePos)) {
                preserveRobotState = !preserveRobotState;
                optionButtons[3].setText(preserveRobotState ? "Keep Pos: ON" : "Keep Pos: OFF", font);
            }

            // Sliders
            for (auto& slider : optionSliders) {
                if (slider->contains(mousePos)) {
                    slider->setDragging(true);
                    slider->setValueFromMouse(mousePos);

                    if (slider == optionSliders[0]) {
                        robotSpeed = slider->getValue();
                        if (playerRobot) playerRobot->setMoveDuration(robotSpeed);
                    }
                    else if (slider == optionSliders[1]) {
                        CELL_SIZE = slider->getValue();
                        updateMazePosition();
                    }
                }
            }
        }
    }

    // Relâchement souris
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        for (auto& slider : optionSliders) slider->setDragging(false);
    }

    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) appState = AppState::MAIN_MENU;

    // Add sound tab handling
    if (currentOptionTab == OptionsTab::SOUND) {
        // Setup UI on first activation
        if (!musicVolumeSlider) {
            setupSoundUI();
        }

        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));

        if (event.type == sf::Event::MouseMoved) {
            // Hover for sound UI
            musicMuteButton.setHovered(musicMuteButton.contains(mousePos));
            sfxMuteButton.setHovered(sfxMuteButton.contains(mousePos));
            musicTestButton.setHovered(musicTestButton.contains(mousePos));
            sfxTestButton.setHovered(sfxTestButton.contains(mousePos));

            // Handle slider dragging
            if (musicVolumeSlider && musicVolumeSlider->isDragging()) {
                musicVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setMusicVolume(musicVolumeSlider->getValue());
                config.musicVolume = musicVolumeSlider->getValue();
            }

            if (sfxVolumeSlider && sfxVolumeSlider->isDragging()) {
                sfxVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setSFXVolume(sfxVolumeSlider->getValue());
                config.sfxVolume = sfxVolumeSlider->getValue();
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            // Handle sound UI clicks
            if (musicMuteButton.contains(mousePos)) {
                bool newMuteState = !soundManager.isMusicMuted();
                soundManager.muteMusic(newMuteState);
                musicMuteButton.setText(newMuteState ? "Unmute" : "Mute", font);
                config.musicMuted = newMuteState;
            }
            else if (sfxMuteButton.contains(mousePos)) {
                bool newMuteState = !soundManager.isSFXMuted();
                soundManager.muteSFX(newMuteState);
                sfxMuteButton.setText(newMuteState ? "Unmute" : "Mute", font);
                config.sfxMuted = newMuteState;
            }
            else if (musicTestButton.contains(mousePos)) {
                soundManager.playTestMusic();
            }
            else if (sfxTestButton.contains(mousePos)) {
                soundManager.playTestSFX();
            }
            else if (musicVolumeSlider && musicVolumeSlider->contains(mousePos)) {
                musicVolumeSlider->setDragging(true);
                musicVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setMusicVolume(musicVolumeSlider->getValue());
                config.musicVolume = musicVolumeSlider->getValue();
            }
            else if (sfxVolumeSlider && sfxVolumeSlider->contains(mousePos)) {
                sfxVolumeSlider->setDragging(true);
                sfxVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setSFXVolume(sfxVolumeSlider->getValue());
                config.sfxVolume = sfxVolumeSlider->getValue();
            }
        }

        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
            if (musicVolumeSlider) musicVolumeSlider->setDragging(false);
            if (sfxVolumeSlider) sfxVolumeSlider->setDragging(false);
        }

        // Handle background music control button
        if (backgroundMusicControlButton.contains(mousePos)) {
            if (soundManager.isBackgroundMusicPlaying()) {
                soundManager.stopBackgroundMusic();
            }
            else {
                soundManager.startBackgroundMusic();
            }
            updateMusicStatusText();
        }
    }

}

void GameEngine::setTool(EditorTool tool)
{
    currentTool = tool;
    if (mazeEditor) mazeEditor->setTool(tool);
}

void GameEngine::handleGameEvents(sf::Event& event, sf::RenderWindow& window)
{
    // Lambda helper for clamping maze offset
    auto clampMazeOffset = [&]()
        {
            if (!currentMaze) return;

            float mazeW = currentMaze->width * CELL_SIZE;
            float mazeH = currentMaze->height * CELL_SIZE;

            const float areaW = 600.f;
            const float areaH = (float)Constants::WINDOW_HEIGHT;
            const float pad = 10.f;

            // X clamp
            if (mazeW <= areaW - 2.f * pad)
            {
                mazeOffset.x = (areaW - mazeW) / 2.f;
            }
            else
            {
                float minX = areaW - pad - mazeW;
                float maxX = pad;
                mazeOffset.x = std::max(minX, std::min(maxX, mazeOffset.x));
            }

            // Y clamp
            if (mazeH <= areaH - 2.f * pad)
            {
                mazeOffset.y = (areaH - mazeH) / 2.f;
            }
            else
            {
                float minY = areaH - pad - mazeH;
                float maxY = pad;
                mazeOffset.y = std::max(minY, std::min(maxY, mazeOffset.y));
            }
        };

    // ------------------------------------------------------------
    // RIGHT MOUSE = PAN (drag navigation)
    // ------------------------------------------------------------
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right)
    {
        sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);

        if (currentMaze && mousePos.x >= 0.f && mousePos.x <= 600.f)
        {
            isPanning = true;
            lastMousePos = mousePos;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Right)
    {
        isPanning = false;
    }

    if (event.type == sf::Event::MouseMoved && isPanning && currentMaze)
    {
        sf::Vector2f mousePos((float)event.mouseMove.x, (float)event.mouseMove.y);

        if (mousePos.x > 600.f)
        {
            isPanning = false;
        }
        else
        {
            sf::Vector2f delta = mousePos - lastMousePos;
            lastMousePos = mousePos;

            mazeOffset += delta;
            clampMazeOffset();
        }
        return;
    }

    // ------------------------------------------------------------
    // MOUSE MOVE: HOVER + EDIT PAINTING (LEFT DRAG)
    // ------------------------------------------------------------
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos((float)event.mouseMove.x, (float)event.mouseMove.y);

        controlPanel.handleHover(mousePos);

        if (state == GameState::EDIT_MODE)
        {
            if (mazeEditor) mazeEditor->updateGhost(window, CELL_SIZE, mazeOffset);
            editorToolbar.handleHover(mousePos);

            if (gameButtons.size() > 0) gameButtons[0].setHovered(gameButtons[0].contains(mousePos));
            if (gameButtons.size() > 1) gameButtons[1].setHovered(gameButtons[1].contains(mousePos));
            if (gameButtons.size() > 8) gameButtons[8].setHovered(gameButtons[8].contains(mousePos));
            if (gameButtons.size() > 9) gameButtons[9].setHovered(gameButtons[9].contains(mousePos));
            if (gameButtons.size() > 10) gameButtons[10].setHovered(gameButtons[10].contains(mousePos));

            // LEFT drag paint
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if (mousePos.x <= 600.f)
                {
                    if (mazeEditor && (currentTool == EditorTool::WALL || currentTool == EditorTool::ERASE))
                    {
                        mazeEditor->applyTool();
                    }
                }
            }
        }
        else
        {
            for (auto& button : gameButtons)
                button.setHovered(button.contains(mousePos));

            for (auto& slider : optionSliders)
            {
                if (slider->isDragging())
                    slider->setValueFromMouse(mousePos);
            }
        }
    }

    // ------------------------------------------------------------
    // LEFT CLICK
    // ------------------------------------------------------------
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);

        // ============================================================
        // ✅ NEW: RIGHT PANEL CAPTURE (textures panel)
        // Any click in x >= 600 belongs to the control panel.
        // We handle it and RETURN so old buttons under it never trigger.
        // ============================================================
        if (mousePos.x >= 600.f)
        {
            // 1) Tabs first (always)
            if (tabControlsBtn && tabControlsBtn->contains(mousePos))
            {
                activeTab = PanelTab::Controls;
                return;
            }
            if (tabTexturesBtn && tabTexturesBtn->contains(mousePos))
            {
                activeTab = PanelTab::Textures;
                return;
            }

            // 2) If we are on Textures tab -> panel consumes clicks
            if (activeTab == PanelTab::Textures)
            {
                controlPanel.handleClick(
                    mousePos,
                    [&](TextureManager::Id id)
                    {
                        std::string chosen = TextureManager::openFileDialog("Select texture");
                        if (!chosen.empty())
                        {
                            if (textureManager.setPath(id, chosen))
                            {
                                applyTexturesFromManager();
                            }
                            else
                            {
                                std::cout << "Texture change failed: "
                                    << textureManager.get(id).lastError << std::endl;
                            }
                        }
                    },
                    [&](TextureManager::Id id)
                    {
                        if (textureManager.reset(id))
                        {
                            applyTexturesFromManager();
                        }
                    }
                );

                return; // capture clicks on textures panel
            }

            // 3) Controls tab -> DO NOT return
            // let the old button handling run below
        }


        // --- A) EDIT MODE ---
        if (state == GameState::EDIT_MODE)
        {
            if (editorToolbar.handleClick(mousePos))
            {
                currentTool = editorToolbar.getSelectedTool();
                if (mazeEditor) mazeEditor->setTool(currentTool);
                std::cout << "Tool Selected: " << static_cast<int>(currentTool) << std::endl;
            }
            else if (gameButtons.size() > 8 && gameButtons[8].contains(mousePos))
            {
                toggleEditMode();
            }
            else if (gameButtons.size() > 9 && gameButtons[9].contains(mousePos))
            {
                if (mazeEditor) mazeEditor->undo();
            }
            else if (gameButtons.size() > 10 && gameButtons[10].contains(mousePos))
            {
                if (mazeEditor) mazeEditor->redo();
            }
            else if (gameButtons.size() > 0 && gameButtons[0].contains(mousePos))
            {
                zoomIn();
                clampMazeOffset();
            }
            else if (gameButtons.size() > 1 && gameButtons[1].contains(mousePos))
            {
                zoomOut();
                clampMazeOffset();
            }
            else
            {
                if (mousePos.x <= 600.f && mazeEditor)
                {
                    mazeEditor->updateGhost(window, CELL_SIZE, mazeOffset);
                    mazeEditor->applyTool();
                }
            }
        }

        // --- B) NORMAL MODE ---
        else
        {
            // Index 0: Zoom +
            if (gameButtons.size() > 0 && gameButtons[0].contains(mousePos))
            {
                zoomIn();
                clampMazeOffset();
            }
            // Index 1: Zoom -
            else if (gameButtons.size() > 1 && gameButtons[1].contains(mousePos))
            {
                zoomOut();
                clampMazeOffset();
            }
            // Index 2: Generate
            else if (gameButtons.size() > 2 && gameButtons[2].contains(mousePos))
            {
                generateMaze();
            }
            // Index 3: Run/Pause
            else if (gameButtons.size() > 3 && gameButtons[3].contains(mousePos))
            {
                toggleRunPause();
            }
            // Index 4: Tester
            else if (gameButtons.size() > 4 && gameButtons[4].contains(mousePos))
            {
                testMaze();
            }
            // Index 5: Sauver
            else if (gameButtons.size() > 5 && gameButtons[5].contains(mousePos))
            {
                saveMaze();
            }
            // Index 6: Resize
            else if (gameButtons.size() > 6 && gameButtons[6].contains(mousePos))
            {
                resizeMaze();
            }
            // Index 7: Menu
            else if (gameButtons.size() > 7 && gameButtons[7].contains(mousePos))
            {
                appState = AppState::MAIN_MENU;
            }
            // Index 8: Edit Mode
            else if (gameButtons.size() > 8 && gameButtons[8].contains(mousePos))
            {
                toggleEditMode();
            }
            // Index 11: Auto-Learn (nouveau bouton)
            else if (gameButtons.size() > 11 && gameButtons[11].contains(mousePos))
            {
                auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());
                if (learningRobot && currentMaze) {
                    std::cout << "\n=== Démarrage de l'apprentissage évolutif ===" << std::endl;

                    learningRobot->runEvolutionaryOptimization(50);
                    updateDashboards();

                    std::cout << "=== Apprentissage évolutif terminé ===" << std::endl;
                    learningRobot->generatePerformanceReport();
                }
                else {
                    std::cout << "Erreur: Robot d'apprentissage non disponible ou pas de labyrinthe!" << std::endl;
                }
            }
            // Index 12: Mode Autonomous
            else if (gameButtons.size() > 12 && gameButtons[12].contains(mousePos))
            {
                auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());
                if (learningRobot) {
                    if (learningRobot->getLearningMode() == LearningRobot::LearningMode::MANUAL) {
                        learningRobot->setLearningMode(LearningRobot::LearningMode::AUTONOMOUS);
                        gameButtons[12].setText("Manual", font);
                        std::cout << "Mode AUTONOME activé - Le robot choisit ses actions" << std::endl;

                        isRunning = true;
                        state = GameState::SOLVING;
                        gameButtons[3].setText("Pause", font);
                    }
                    else {
                        learningRobot->setLearningMode(LearningRobot::LearningMode::MANUAL);
                        gameButtons[12].setText("Autonomous", font);
                        std::cout << "Mode MANUEL activé - Le robot suit A*" << std::endl;

                        computePath();
                    }
                }
            }
            // Text inputs focus
            else if (mazeNameInput && mazeNameInput->contains(mousePos))
            {
                mazeNameInput->setFocused(true);
                if (mazeWidthInput) mazeWidthInput->setFocused(false);
                if (mazeHeightInput) mazeHeightInput->setFocused(false);
            }
            else if (mazeWidthInput && mazeWidthInput->contains(mousePos))
            {
                if (mazeNameInput) mazeNameInput->setFocused(false);
                mazeWidthInput->setFocused(true);
                if (mazeHeightInput) mazeHeightInput->setFocused(false);
            }
            else if (mazeHeightInput && mazeHeightInput->contains(mousePos))
            {
                if (mazeNameInput) mazeNameInput->setFocused(false);
                if (mazeWidthInput) mazeWidthInput->setFocused(false);
                mazeHeightInput->setFocused(true);
            }
            else
            {
                if (mazeNameInput)  mazeNameInput->setFocused(false);
                if (mazeWidthInput) mazeWidthInput->setFocused(false);
                if (mazeHeightInput) mazeHeightInput->setFocused(false);
            }
        }
    }

    // ------------------------------------------------------------
    // LEFT RELEASE (slider)
    // ------------------------------------------------------------
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        for (auto& slider : optionSliders)
            slider->setDragging(false);
    }

    // ------------------------------------------------------------
    // TEXT INPUT
    // ------------------------------------------------------------
    if (event.type == sf::Event::TextEntered)
    {
        if (mazeNameInput)  mazeNameInput->handleTextEntered(event.text.unicode);
        if (mazeWidthInput) mazeWidthInput->handleTextEntered(event.text.unicode);
        if (mazeHeightInput) mazeHeightInput->handleTextEntered(event.text.unicode);

        if (mazeNameInput)
            currentMazeName = mazeNameInput->getText();
    }

    // ------------------------------------------------------------
    // KEYBOARD SHORTCUTS
    // ------------------------------------------------------------
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::R && state != GameState::EDIT_MODE)
            loadLevel();

        if (event.key.code == sf::Keyboard::Escape)
            appState = AppState::MAIN_MENU;

        if (event.key.code == sf::Keyboard::E)
            toggleEditMode();

        if (event.key.code == sf::Keyboard::A && state != GameState::EDIT_MODE)
        {
            auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());
            if (learningRobot && currentMaze) {
                std::cout << "\n=== Démarrage de l'apprentissage évolutif (raccourci A) ===" << std::endl;
                learningRobot->runEvolutionaryOptimization(50);
                updateDashboards();
                std::cout << "=== Apprentissage terminé ===" << std::endl;
            }
        }

        if (state == GameState::EDIT_MODE && mazeEditor)
        {
            if (event.key.control && event.key.code == sf::Keyboard::Z)
                mazeEditor->undo();

            if (event.key.control && event.key.code == sf::Keyboard::Y)
                mazeEditor->redo();
        }
    }
}


void GameEngine::updateGame(float dt)
{
    if (state == GameState::EDIT_MODE) return;

    // Mettre à jour le robot
    playerRobot->update(dt);

    auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());

    // ← AJOUTER : MODE AUTONOME
    if (learningRobot &&
        learningRobot->getLearningMode() == LearningRobot::LearningMode::AUTONOMOUS &&
        isRunning)
    {
        // Le robot choisit ses propres actions
        if (!playerRobot->isMoving()) {
            Point currentPos = playerRobot->getPosition();

            // Si pas au but, continuer
            if (currentPos != currentMaze->endPos) {
                auto actions = learningRobot->getAvailableActions(currentPos);
                if (!actions.empty()) {
                    // Choisir une action via Q-learning
                    int action = learningRobot->qLearning->chooseAction(currentPos, actions);
                    Point nextPos = learningRobot->getNextState(currentPos, action);
                    playerRobot->moveTo(nextPos);
                }
            }
            else {
                // But atteint, redémarrer
                std::cout << "But atteint en mode autonome!" << std::endl;
                learningRobot->startNewTrial();
                playerRobot->setPosition(currentMaze->startPos);
                playerRobot->setState(RobotState::MOVING);
            }
        }
        return; // Ne pas exécuter la logique de pathfinding manuel
    }

    // MODE PATHFINDING : Suivre le chemin calculé
    if (isRunning && state == GameState::SOLVING)
    {
        // Si le robot ne bouge pas et qu'il reste des étapes
        if (!playerRobot->isMoving() && pathIndex < solutionPath.size())
        {
            playerRobot->moveTo(solutionPath[pathIndex]);
            pathIndex++;
        }

        // Vérifier si le but est atteint
        if (playerRobot->getPosition() == currentMaze->endPos)
        {
            state = GameState::COMPLETE;
            playerRobot->setState(RobotState::COMPLETED);

            std::cout << "Target Reached! Steps: " << playerRobot->getSteps() << std::endl;

            // Redémarrage automatique si c'est un robot d'apprentissage
            auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());
            if (learningRobot && isRunning) {
                // Attendre un court instant
                static sf::Clock restartClock;
                if (restartClock.getElapsedTime().asSeconds() > 0.5f) {
                    std::cout << "\n=== Redémarrage automatique ===" << std::endl;

                    // Nouveau trial
                    learningRobot->startNewTrial();
                    playerRobot->setPosition(currentMaze->startPos);
                    playerRobot->setState(RobotState::MOVING);

                    // Recalculer le chemin
                    state = GameState::SOLVING;
                    pathIndex = 1;
                    computePath();

                    restartClock.restart();
                }
            }
            else {
                isRunning = false;
                gameButtons[3].setText("Run", font);
            }
        }
    }
}

void GameEngine::drawMainMenu(sf::RenderWindow& window)
{
    if (!fontLoaded) return;
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    titleText.setPosition(400.0f, 150.0f);
    window.draw(titleText);
    for (const auto& button : menuButtons) button.draw(window);
}

void GameEngine::drawOptionsMenu(sf::RenderWindow& window)
{
    if (!fontLoaded) return;

    // 1. Titre
    sf::FloatRect titleBounds = optionsTitleText.getLocalBounds();
    optionsTitleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    optionsTitleText.setPosition(400.0f, 80.0f);
    window.draw(optionsTitleText);

    // 2. Dessiner les 4 Onglets
    for (size_t i = 0; i < optionTabButtons.size(); ++i) {
        optionTabButtons[i].draw(window);

        // Souligner l'onglet actif
        bool isActive = (i == 0 && currentOptionTab == OptionsTab::SETTINGS) ||
            (i == 1 && currentOptionTab == OptionsTab::TEXTURES) ||
            (i == 2 && currentOptionTab == OptionsTab::SOUND) ||
            (i == 3 && currentOptionTab == OptionsTab::MY_MAZES);

        if (isActive) {
            sf::RectangleShape underline(sf::Vector2f(140.0f, 3.0f)); // Largeur adaptée au bouton
            underline.setFillColor(sf::Color::Cyan);
            // Calcul précis de la position (doit correspondre au setup)
            float startX = 100.0f;
            float gap = 10.0f;
            underline.setPosition(startX + i * (140.0f + gap), 195.0f);
            window.draw(underline);
        }
    }

    // 3. Bouton BACK (Toujours visible)
    if (optionButtons.size() > 0) optionButtons[0].draw(window);

    // 4. CONTENU VARIABLE SELON L'ONGLET

    if (currentOptionTab == OptionsTab::SETTINGS) {
        // --- PAGE SETTINGS ---
        // Affiche les sliders
        for (const auto& slider : optionSliders) slider->draw(window);

        // Affiche les boutons d'options (Index 1, 2, 3)
        // On commence à 1 car 0 est le bouton BACK
        for (size_t i = 1; i < optionButtons.size(); ++i) {
            optionButtons[i].draw(window);
        }
    }
    else if (currentOptionTab == OptionsTab::TEXTURES) {
        // --- PAGE TEXTURES ---
        sf::Text msg("Texture Selection Coming Soon...", font, 24);
        sf::FloatRect bounds = msg.getLocalBounds();
        msg.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        msg.setPosition(400, 350);
        window.draw(msg);
    }
    else if (currentOptionTab == OptionsTab::SOUND) {
        sf::Text msg("Sound Settings Coming Soon...", font, 24);
        sf::FloatRect bounds = msg.getLocalBounds();
        msg.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        msg.setPosition(400, 350);
        window.draw(msg);


    }
    else if (currentOptionTab == OptionsTab::MY_MAZES) {
        sf::Text msg("Maze Browser Coming Soon...", font, 24);
        sf::FloatRect bounds = msg.getLocalBounds();
        msg.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        msg.setPosition(400, 350);
        window.draw(msg);
    }

    if (currentOptionTab == OptionsTab::SOUND) {
        // Setup UI if needed
        if (!musicVolumeSlider) {
            setupSoundUI();
        }

        // Draw sound UI
        if (musicVolumeSlider) musicVolumeSlider->draw(window);
        if (sfxVolumeSlider) sfxVolumeSlider->draw(window);

        musicMuteButton.draw(window);
        sfxMuteButton.draw(window);
        musicTestButton.draw(window);
        sfxTestButton.draw(window);

        // Draw sound info text
        if (soundManager.isInitialized()) {
            sf::Text statusText("Audio System: Active", font, 16);
            statusText.setPosition(200, 350);
            statusText.setFillColor(sf::Color::Green);
            window.draw(statusText);
        }
        else {
            sf::Text statusText("Audio System: Not Available", font, 16);
            statusText.setPosition(200, 350);
            statusText.setFillColor(sf::Color::Red);
            window.draw(statusText);
        }

        // Draw background music control
        backgroundMusicControlButton.draw(window);
        window.draw(musicStatusText);

        // Draw audio system status
        sf::Text statusText("Audio: " + soundManager.getStatus(), font, 16);
        statusText.setPosition(200, 400);
        statusText.setFillColor(sf::Color::Cyan);
        window.draw(statusText);
    }
}

void GameEngine::drawGame(sf::RenderWindow& window)
{
    // 0) Draw FLOOR first (background tiles)
    if (floorTexture.getSize().x > 0 && currentMaze)
    {
        floorSprite.setScale(
            CELL_SIZE / (float)floorTexture.getSize().x,
            CELL_SIZE / (float)floorTexture.getSize().y
        );

        for (int y = 0; y < currentMaze->height; ++y)
        {
            for (int x = 0; x < currentMaze->width; ++x)
            {
                floorSprite.setPosition(
                    x * CELL_SIZE + mazeOffset.x,
                    y * CELL_SIZE + mazeOffset.y
                );
                window.draw(floorSprite);
            }
        }
    }

    // 1) Draw the maze content
    drawMaze(window);

    // 2) Draw path and explored cells if enabled
    if (showPath) drawPathOverlay(window);
    if (showExploredCells) drawExploredCells(window);

    // 3) Draw robot
    drawRobot(window);

    // ------------------------------------------------------------
    // RIGHT PANEL: either Controls UI or Textures UI
    // ------------------------------------------------------------

    if (activeTab == PanelTab::Controls)
    {
        // Draw your classic controls (title, buttons, inputs, editor toolbar)
        gameTitleText.setPosition(610, 30);
        window.draw(gameTitleText);

        for (size_t i = 0; i < gameButtons.size(); ++i)
        {
            if (state == GameState::EDIT_MODE) {
                if (i != 0 && i != 1 && i != 8 && i != 9 && i != 10) continue;
            }
            else {
                if (i == 9 || i == 10) continue;
            }
            gameButtons[i].draw(window);
        }

        if (state != GameState::EDIT_MODE)
        {
            mazeNameInput->draw(window);
            mazeWidthInput->draw(window);
            mazeHeightInput->draw(window);
        }

        if (state == GameState::EDIT_MODE)
        {
            editorToolbar.draw(window);
            if (mazeEditor) mazeEditor->draw(window);
        }
    }
    else
    {
        // Textures tab UI
        controlPanel.draw(window);
    }

    // ------------------------------------------------------------
    // Tabs ALWAYS on top (so they remain clickable/visible)
    // ------------------------------------------------------------
    if (tabControlsBtn) tabControlsBtn->draw(window);
    if (tabTexturesBtn) tabTexturesBtn->draw(window);

    // ------------------------------------------------------------
    // Learning UI + Dashboards (keep exactly as you had)
    // ------------------------------------------------------------
    updateLearningUI();
    window.draw(learningPanel);
    window.draw(learningScoreText);
    window.draw(successRateText);
    window.draw(explorationRateText);

    if (trainingVisualizer) {
        trainingVisualizer->draw();
    }
    if (performanceDashboard) {
        performanceDashboard->draw();
    }
}


void GameEngine::drawMaze(sf::RenderWindow& window)
{
    if (!currentMaze) return;

    for (int y = 0; y < currentMaze->height; ++y)
    {
        for (int x = 0; x < currentMaze->width; ++x)
        {
            CellType t = currentMaze->grid[y][x]->getType();
            
            // Draw wall texture if it's a wall
            if (t == CellType::WALL && wallTexture.getSize().x > 0)
            {
                wallSprite.setScale(
                    CELL_SIZE / (float)wallTexture.getSize().x,
                    CELL_SIZE / (float)wallTexture.getSize().y
                );
                wallSprite.setPosition(
                    x * CELL_SIZE + mazeOffset.x,
                    y * CELL_SIZE + mazeOffset.y
                );
                window.draw(wallSprite);
            }
            else
            {
                // Draw colored rectangle for other cell types
                sf::RectangleShape cellShape(sf::Vector2f(CELL_SIZE - 2.0f, CELL_SIZE - 2.0f));
                cellShape.setPosition(
                    x * CELL_SIZE + mazeOffset.x + 1.0f,
                    y * CELL_SIZE + mazeOffset.y + 1.0f
                );
                
                switch (t)
                {
                case CellType::START:
                    cellShape.setFillColor(sf::Color(100, 220, 100));
                    break;
                case CellType::END:
                    cellShape.setFillColor(sf::Color(220, 100, 100));
                    break;
                default:
                    cellShape.setFillColor(sf::Color(200, 200, 200, 100)); // Semi-transparent
                    break;
                }
                window.draw(cellShape);
            }
        }
    }
}

void GameEngine::drawPathOverlay(sf::RenderWindow& window)
{
    if (solutionPath.empty()) return;

    sf::RectangleShape pathShape(sf::Vector2f(CELL_SIZE / 3.0f, CELL_SIZE / 3.0f));
    pathShape.setFillColor(sf::Color(50, 50, 255, 150)); // Bleu semi-transparent
    pathShape.setOrigin(pathShape.getSize() / 2.0f);

    for (const auto& point : solutionPath)
    {
        if (point == currentMaze->startPos || point == currentMaze->endPos) continue;

        pathShape.setPosition(
            point.x * CELL_SIZE + mazeOffset.x + CELL_SIZE / 2.0f,
            point.y * CELL_SIZE + mazeOffset.y + CELL_SIZE / 2.0f
        );
        window.draw(pathShape);
    }
}

void GameEngine::drawExploredCells(sf::RenderWindow& window)
{
    if (!pathFinder) return;

    const auto& explored = pathFinder->getExplored();
    sf::RectangleShape exploredShape(sf::Vector2f(CELL_SIZE - 4.0f, CELL_SIZE - 4.0f));
    exploredShape.setFillColor(sf::Color(255, 255, 0, 50)); // Jaune très transparent

    for (const auto& point : explored)
    {
        if (point == currentMaze->startPos || point == currentMaze->endPos) continue;

        exploredShape.setPosition(
            point.x * CELL_SIZE + mazeOffset.x + 2.0f,
            point.y * CELL_SIZE + mazeOffset.y + 2.0f
        );
        window.draw(exploredShape);
    }
}

void GameEngine::drawRobot(sf::RenderWindow& window)
{
    if (!playerRobot) return;

    // Draw robot sprite if texture is loaded
    if (robotTexture.getSize().x > 0)
    {
        robotSprite.setScale(
            CELL_SIZE / robotTexture.getSize().x,
            CELL_SIZE / robotTexture.getSize().y
        );
        
        // Get interpolated position for smooth animation
        sf::Vector2f floatPos = playerRobot->getFloatPos(CELL_SIZE);
        robotSprite.setPosition(
            floatPos.x + mazeOffset.x,
            floatPos.y + mazeOffset.y
        );
        window.draw(robotSprite);
    }
    else
    {
        // Fallback: draw a circle
        sf::CircleShape robotShape(CELL_SIZE / 2.5f);
        robotShape.setFillColor(sf::Color::Blue);
        robotShape.setOrigin(CELL_SIZE / 2.5f, CELL_SIZE / 2.5f);

        Point pos = playerRobot->getPosition();
        robotShape.setPosition(
            pos.x * CELL_SIZE + mazeOffset.x + CELL_SIZE / 2.0f,
            pos.y * CELL_SIZE + mazeOffset.y + CELL_SIZE / 2.0f
        );
        window.draw(robotShape);
    }
}

void GameEngine::setupSoundUI() {
    if (!fontLoaded) return;

    float startX = 200.0f;
    float startY = 220.0f;
    float sliderWidth = 300.0f;
    float verticalGap = 50.0f;

    // Create sliders if they don't exist
    
    musicVolumeSlider = std::make_unique<Slider>(
        sf::Vector2f(startX, startY),
        sliderWidth,
        0.0f, 100.0f,
        soundManager.getMusicVolume(),
        "Music Volume",
        font
    );
    

    
    sfxVolumeSlider = std::make_unique<Slider>(
        sf::Vector2f(startX, startY + verticalGap),
        sliderWidth,
        0.0f, 100.0f,
        soundManager.getSFXVolume(),
        "SFX Volume",
        font
    );
    

    // Setup mute buttons
    float buttonX = startX + sliderWidth + 20.0f;

    musicMuteButton = Button(
        sf::Vector2f(80, 30),
        sf::Vector2f(buttonX, startY - 10),
        soundManager.isMusicMuted() ? "Unmute" : "Mute",
        font,
        14
    );

    sfxMuteButton = Button(
        sf::Vector2f(80, 30),
        sf::Vector2f(buttonX, startY + verticalGap - 10),
        soundManager.isSFXMuted() ? "Unmute" : "Mute",
        font,
        14
    );

    // Setup test buttons
    float testButtonX = buttonX + 90.0f;

    musicTestButton = Button(
        sf::Vector2f(80, 30),
        sf::Vector2f(testButtonX, startY - 10),
        "Test",
        font,
        14
    );

    sfxTestButton = Button(
        sf::Vector2f(80, 30),
        sf::Vector2f(testButtonX, startY + verticalGap - 10),
        "Test",
        font,
        14
    );

    // Add a dedicated background music control button
    float bgMusicY = startY + verticalGap * 2;
    backgroundMusicControlButton = Button(
        sf::Vector2f(150, 30),
        sf::Vector2f(startX, bgMusicY),
        "Toggle Music",
        font,
        16
    );

    // Add music status display
    musicStatusText.setFont(font);
    musicStatusText.setCharacterSize(16);
    musicStatusText.setPosition(startX + 160, bgMusicY + 5);
    updateMusicStatusText();
}

void GameEngine::updateMusicStatusText() {
    if (soundManager.isBackgroundMusicPlaying()) {
        musicStatusText.setString("Playing");
        musicStatusText.setFillColor(sf::Color::Green);
    }
    else {
        musicStatusText.setString("Stopped");
        musicStatusText.setFillColor(sf::Color::Red);
    }
}

void GameEngine::toggleEditMode()
{
    if (!currentMaze) return;

    if (state == GameState::EDIT_MODE)
    {
        // Exit edit mode
        std::cout << "Sortie du Mode Édition." << std::endl;
        state = GameState::IDLE;

        if (gameButtons.size() > 8) {
            gameButtons[8].setText("Edit Mode", font);
        }

        // IMPORTANT : Recalculate path (Keep robot position)
        computePath();
    }
    else
    {
        // Enter edit mode
        state = GameState::EDIT_MODE;
        isRunning = false;

        if (gameButtons.size() > 3) gameButtons[3].setText("Run", font);
        if (gameButtons.size() > 8) gameButtons[8].setText("Done", font);
    }

    // Play sound
    soundManager.playSound("test_sfx");
}

