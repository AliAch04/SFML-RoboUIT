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
    updateInterval(0.1f)
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

    optionsTitleText.setString("OPTIONS");
    optionsTitleText.setCharacterSize(48);
    optionsTitleText.setFillColor(sf::Color::White);
    optionsTitleText.setStyle(sf::Text::Bold);

    optionButtons.clear();
    // Index 0 : Back
    optionButtons.emplace_back(sf::Vector2f(150, 40), sf::Vector2f(325, 500), "BACK", font, 20);

    optionSliders.clear();
    optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(250, 150), 300, 0.1f, 1.0f, robotSpeed, "Robot Speed", font));
    optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(250, 220), 300, 20.0f, 80.0f, cellSizeValue, "Cell Size", font));

    // Toggle buttons
    optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, 290),
        showExploredCells ? "Explored: ON" : "Explored: OFF", font, 18);

    optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, 350),
        showPath ? "Path: ON" : "Path: OFF", font, 18);

    // --- BOUTON : KEEP POS (Index 3) ---
    optionButtons.emplace_back(sf::Vector2f(200, 40), sf::Vector2f(250, 410),
        preserveRobotState ? "Keep Pos: ON" : "Keep Pos: OFF", font, 18);
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
}

void GameEngine::zoomOut()
{
    CELL_SIZE = std::max(Constants::MIN_CELL_SIZE, CELL_SIZE - 5.0f);
    updateMazePosition();
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
}

void GameEngine::toggleRunPause() {
    if (!currentMaze) return;

    if (isRunning) {
        playerRobot->pause();
        isRunning = false;
        gameButtons[3].setText("Run", font);
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
    }
}

void GameEngine::testMaze()
{
    if (!currentMaze) return;
    bool solvable = pathFinder->isSolvable(currentMaze.get());
    std::cout << "Maze is " << (solvable ? "SOLVABLE" : "NOT SOLVABLE") << std::endl;
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
}

void GameEngine::run()
{
    sf::RenderWindow window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
        "Robot A* Simulation", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    sf::Clock deltaClock;
    worldView = window.getDefaultView();
    uiView = window.getDefaultView();

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

    // Save Config
    config.robotSpeed = robotSpeed;
    config.cellSize = CELL_SIZE;
    config.showExploredCells = showExploredCells;
    config.showPath = showPath;
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
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));

        for (auto& button : optionButtons) button.setHovered(button.contains(mousePos));

        for (auto& slider : optionSliders) {
            if (slider->isDragging()) slider->setValueFromMouse(mousePos);
        }
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

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
        else if (optionButtons.size() > 3 && optionButtons[3].contains(mousePos)) {
            preserveRobotState = !preserveRobotState;
            optionButtons[3].setText(preserveRobotState ? "Keep Pos: ON" : "Keep Pos: OFF", font);
        }

        for (auto& slider : optionSliders) {
            if (slider->contains(mousePos)) {
                slider->setDragging(true);
                slider->setValueFromMouse(mousePos);
                if (slider.get() == optionSliders[0].get()) {
                    robotSpeed = slider->getValue();
                    if (playerRobot) playerRobot->setMoveDuration(robotSpeed);
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
        for (auto& slider : optionSliders) slider->setDragging(false);
    }
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) appState = AppState::MAIN_MENU;
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

                    // Option 1: Lancer l'optimisation évolutive complète
                    learningRobot->runEvolutionaryOptimization(50);

                    // Option 2: Ou simplement trouver le chemin évolutif
                    // auto path = learningRobot->findEvolutionaryPath();

                    // Mettre à jour les dashboards immédiatement
                    updateDashboards();

                    std::cout << "=== Apprentissage évolutif terminé ===" << std::endl;

                    // Afficher le rapport de performance
                    learningRobot->generatePerformanceReport();
                }
                else {
                    std::cout << "Erreur: Robot d'apprentissage non disponible ou pas de labyrinthe!" << std::endl;
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
                // Défocus tous les inputs si clic ailleurs
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
        // R: Reload level
        if (event.key.code == sf::Keyboard::R && state != GameState::EDIT_MODE)
            loadLevel();

        // Escape: Return to main menu
        if (event.key.code == sf::Keyboard::Escape)
            appState = AppState::MAIN_MENU;

        // E: Toggle edit mode
        if (event.key.code == sf::Keyboard::E)
            toggleEditMode();

        // A: Auto-Learn shortcut
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

        // Edit mode shortcuts
        if (state == GameState::EDIT_MODE && mazeEditor)
        {
            // Ctrl+Z: Undo
            if (event.key.control && event.key.code == sf::Keyboard::Z)
                mazeEditor->undo();

            // Ctrl+Y: Redo
            if (event.key.control && event.key.code == sf::Keyboard::Y)
                mazeEditor->redo();
        }
    }
}

void GameEngine::updateGame(float dt)
{
    if (state == GameState::EDIT_MODE) return;

    if (isRunning && state == GameState::SOLVING && !playerRobot->isMoving() && pathIndex < solutionPath.size())
    {
        playerRobot->moveTo(solutionPath[pathIndex]);
        pathIndex++;
    }

    playerRobot->update(dt);

    // ← AJOUTER : Auto-redémarrage quand le but est atteint
    if (state == GameState::SOLVING && playerRobot->getPosition() == currentMaze->endPos)
    {
        state = GameState::COMPLETE;
        playerRobot->setState(RobotState::COMPLETED);

        std::cout << "Target Reached! Steps: " << playerRobot->getSteps() << std::endl;

        // ← REDÉMARRER AUTOMATIQUEMENT POUR L'APPRENTISSAGE CONTINU
        auto* learningRobot = dynamic_cast<LearningRobot*>(playerRobot.get());
        if (learningRobot && isRunning) {
            // Attendre 1 seconde avant de redémarrer
            static sf::Clock restartClock;
            if (restartClock.getElapsedTime().asSeconds() > 1.0f) {
                learningRobot->startNewTrial();
                playerRobot->setPosition(currentMaze->startPos);
                playerRobot->setState(RobotState::MOVING);
                state = GameState::SOLVING;
                pathIndex = 1;

                // Recalculer le chemin
                computePath();

                restartClock.restart();
                std::cout << "Nouveau trial démarré automatiquement" << std::endl;
            }
        }
        else {
            isRunning = false;
            gameButtons[3].setText("Run", font);
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
    sf::FloatRect titleBounds = optionsTitleText.getLocalBounds();
    optionsTitleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    optionsTitleText.setPosition(400.0f, 80.0f);
    window.draw(optionsTitleText);
    for (const auto& slider : optionSliders) slider->draw(window);
    for (const auto& button : optionButtons) button.draw(window);
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

    // 4) Draw UI LAST so it stays visible
    sf::RectangleShape panel(sf::Vector2f(200, 600));
    panel.setPosition(600, 0);
    panel.setFillColor(sf::Color(50, 50, 50));
    window.draw(panel);

    gameTitleText.setPosition(610, 30);
    window.draw(gameTitleText);

    // Draw buttons
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

    // Q-learning UI (ancien)
    updateLearningUI();
    window.draw(learningPanel);
    window.draw(learningScoreText);
    window.draw(successRateText);
    window.draw(explorationRateText);

    // ← AJOUTER : DESSINER LES NOUVEAUX DASHBOARDS
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
}