#include "GameEngine.h"
#include "SimpleJSON.h"
#include "Logger.h"
#include "Config.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm> // Pour std::max, std::min, std::find

// -------------------------------------------------------------------------
// CONSTRUCTEUR
// -------------------------------------------------------------------------
GameEngine::GameEngine() :
    playerRobot(std::make_unique<Robot>()),
    pathFinder(std::make_unique<PathFinder>()),
    savedRobotPos({ 0, 0 }),
    savedRobotState(RobotState::IDLE)
{
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

    if (fontLoaded)
    {
        titleText.setFont(font);
        optionsTitleText.setFont(font);
        gameTitleText.setFont(font);
        setupMainMenu();
        setupOptionsMenu();
        setupGameUI();
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

    float mazeWidth = currentMaze->width * CELL_SIZE;
    float mazeHeight = currentMaze->height * CELL_SIZE;

    mazeOffset.x = (600 - mazeWidth) / 2.0f;
    mazeOffset.y = (600 - mazeHeight) / 2.0f;

    if (mazeOffset.x + mazeWidth > 600) mazeOffset.x = 600 - mazeWidth - 10;
    if (mazeOffset.x < 10) mazeOffset.x = 10;
    if (mazeOffset.y < 10) mazeOffset.y = 10;
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

void GameEngine::generateMaze()
{
    if (!currentMaze) return;

    try
    {
        int width = std::stoi(mazeWidthInput->getText());
        int height = std::stoi(mazeHeightInput->getText());
        width = std::max(5, std::min(30, width));
        height = std::max(5, std::min(30, height));

        currentMaze = std::make_unique<Maze>(width, height);
        currentMaze->generateSolvableMaze();

        mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
        mazeEditor->setTool(currentTool);

        playerRobot->setPosition(currentMaze->startPos);
        state = GameState::IDLE;
        isRunning = false;

        computePath();
        updateMazePosition();
        std::cout << "Generated new maze: " << width << "x" << height << std::endl;
    }
    catch (...)
    {
        std::cout << "Invalid size input for maze generation!" << std::endl;
    }
}

void GameEngine::toggleRunPause()
{
    if (!currentMaze) return;

    if (isRunning)
    {
        playerRobot->pause();
        isRunning = false;
        gameButtons[3].setText("Run", font);
    }
    else
    {
        if (state == GameState::COMPLETE || state == GameState::FAILED)
        {
            playerRobot->setPosition(currentMaze->startPos);
            pathIndex = 1;
            state = GameState::SOLVING;
        }
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

void GameEngine::resizeMaze()
{
    if (!currentMaze) return;
    try
    {
        int newWidth = std::stoi(mazeWidthInput->getText());
        int newHeight = std::stoi(mazeHeightInput->getText());
        newWidth = std::max(5, std::min(30, newWidth));
        newHeight = std::max(5, std::min(30, newHeight));

        currentMaze->resize(newWidth, newHeight);

        mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
        mazeEditor->setTool(currentTool);

        playerRobot->setPosition(currentMaze->startPos);
        state = GameState::IDLE;
        isRunning = false;
        computePath();
        updateMazePosition();
        std::cout << "Maze resized to: " << newWidth << "x" << newHeight << std::endl;
    }
    catch (...)
    {
        std::cout << "Invalid size input!" << std::endl;
    }
}

void GameEngine::run()
{
    sf::RenderWindow window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
        "Robot A* Simulation", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    sf::Clock deltaClock;

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

        if (appState == AppState::GAME) updateGame(dt);

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
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));

        if (state == GameState::EDIT_MODE)
        {
            if (mazeEditor) mazeEditor->updateGhost(window, CELL_SIZE, mazeOffset);
            editorToolbar.handleHover(mousePos);

            if (gameButtons.size() > 0) gameButtons[0].setHovered(gameButtons[0].contains(mousePos));
            if (gameButtons.size() > 1) gameButtons[1].setHovered(gameButtons[1].contains(mousePos));
            if (gameButtons.size() > 8) gameButtons[8].setHovered(gameButtons[8].contains(mousePos));
            if (gameButtons.size() > 9) gameButtons[9].setHovered(gameButtons[9].contains(mousePos));
            if (gameButtons.size() > 10) gameButtons[10].setHovered(gameButtons[10].contains(mousePos));

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                if (mazeEditor && (currentTool == EditorTool::WALL || currentTool == EditorTool::ERASE)) {
                    mazeEditor->applyTool();
                }
            }
        }
        else
        {
            for (auto& button : gameButtons) button.setHovered(button.contains(mousePos));
        }
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y));

        if (state == GameState::EDIT_MODE)
        {
            if (editorToolbar.handleClick(mousePos)) {
                currentTool = editorToolbar.getSelectedTool();
                if (mazeEditor) mazeEditor->setTool(currentTool);
            }
            else if (gameButtons.size() > 8 && gameButtons[8].contains(mousePos)) toggleEditMode();
            else if (gameButtons.size() > 9 && gameButtons[9].contains(mousePos)) { if (mazeEditor) mazeEditor->undo(); }
            else if (gameButtons.size() > 10 && gameButtons[10].contains(mousePos)) { if (mazeEditor) mazeEditor->redo(); }
            else if (gameButtons.size() > 0 && gameButtons[0].contains(mousePos)) zoomIn();
            else if (gameButtons.size() > 1 && gameButtons[1].contains(mousePos)) zoomOut();
            else {
                if (mazeEditor) {
                    mazeEditor->updateGhost(window, CELL_SIZE, mazeOffset);
                    mazeEditor->applyTool();
                }
            }
        }
        else
        {
            if (gameButtons.size() > 0 && gameButtons[0].contains(mousePos)) zoomIn();
            else if (gameButtons.size() > 1 && gameButtons[1].contains(mousePos)) zoomOut();
            else if (gameButtons.size() > 2 && gameButtons[2].contains(mousePos)) generateMaze();
            else if (gameButtons.size() > 3 && gameButtons[3].contains(mousePos)) toggleRunPause();
            else if (gameButtons.size() > 4 && gameButtons[4].contains(mousePos)) testMaze();
            else if (gameButtons.size() > 5 && gameButtons[5].contains(mousePos)) saveMaze();
            else if (gameButtons.size() > 6 && gameButtons[6].contains(mousePos)) resizeMaze();
            else if (gameButtons.size() > 7 && gameButtons[7].contains(mousePos)) appState = AppState::MAIN_MENU;
            else if (gameButtons.size() > 8 && gameButtons[8].contains(mousePos)) toggleEditMode();

            if (mazeNameInput->contains(mousePos)) { mazeNameInput->setFocused(true); mazeWidthInput->setFocused(false); mazeHeightInput->setFocused(false); }
            else if (mazeWidthInput->contains(mousePos)) { mazeNameInput->setFocused(false); mazeWidthInput->setFocused(true); mazeHeightInput->setFocused(false); }
            else if (mazeHeightInput->contains(mousePos)) { mazeNameInput->setFocused(false); mazeWidthInput->setFocused(false); mazeHeightInput->setFocused(true); }
            else { mazeNameInput->setFocused(false); mazeWidthInput->setFocused(false); mazeHeightInput->setFocused(false); }
        }
    }

    if (event.type == sf::Event::TextEntered)
    {
        mazeNameInput->handleTextEntered(event.text.unicode);
        mazeWidthInput->handleTextEntered(event.text.unicode);
        mazeHeightInput->handleTextEntered(event.text.unicode);
        currentMazeName = mazeNameInput->getText();
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::R && state != GameState::EDIT_MODE) loadLevel();
        if (event.key.code == sf::Keyboard::Escape) appState = AppState::MAIN_MENU;
        if (event.key.code == sf::Keyboard::E) toggleEditMode();

        if (state == GameState::EDIT_MODE && mazeEditor)
        {
            if (event.key.control && event.key.code == sf::Keyboard::Z) mazeEditor->undo();
            if (event.key.control && event.key.code == sf::Keyboard::Y) mazeEditor->redo();
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

    if (state == GameState::SOLVING && playerRobot->getPosition() == currentMaze->endPos)
    {
        state = GameState::COMPLETE;
        playerRobot->setState(RobotState::COMPLETED);
        isRunning = false;
        gameButtons[3].setText("Run", font);
        std::cout << "Target Reached! Steps: " << playerRobot->getSteps() << std::endl;
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
    sf::RectangleShape panel(sf::Vector2f(200, 600));
    panel.setPosition(600, 0);
    panel.setFillColor(sf::Color(50, 50, 50));
    window.draw(panel);

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

    if (state == GameState::EDIT_MODE) editorToolbar.draw(window);

    drawMaze(window);
    if (showPath) drawPathOverlay(window);
    if (showExploredCells) drawExploredCells(window);

    drawRobot(window);

    if (state == GameState::EDIT_MODE && mazeEditor) mazeEditor->draw(window);
}

void GameEngine::drawMaze(sf::RenderWindow& window)
{
    if (!currentMaze) return;

    sf::RectangleShape cellShape(sf::Vector2f(CELL_SIZE - 2.0f, CELL_SIZE - 2.0f));
    for (int y = 0; y < currentMaze->height; ++y)
    {
        for (int x = 0; x < currentMaze->width; ++x)
        {
            CellType t = currentMaze->grid[y][x]->getType();
            cellShape.setPosition(x * CELL_SIZE + mazeOffset.x + 1.0f,
                y * CELL_SIZE + mazeOffset.y + 1.0f);
            switch (t)
            {
            case CellType::WALL:
                cellShape.setFillColor(sf::Color::Black);
                break;
            case CellType::START:
                cellShape.setFillColor(sf::Color(100, 220, 100));
                break;
            case CellType::END:
                cellShape.setFillColor(sf::Color(220, 100, 100));
                break;
            default:
                cellShape.setFillColor(sf::Color(200, 200, 200));
                break;
            }
            window.draw(cellShape);
        }
    }
}

// --------------------------------------------------------------------------------
// FONCTIONS DE DESSIN SUPPLEMENTAIRES (MANQUANTES DANS TON ANCIEN CODE)
// --------------------------------------------------------------------------------

void GameEngine::drawPathOverlay(sf::RenderWindow& window)
{
    if (solutionPath.empty()) return;

    sf::RectangleShape pathShape(sf::Vector2f(CELL_SIZE / 3.0f, CELL_SIZE / 3.0f));
    pathShape.setFillColor(sf::Color(50, 50, 255, 150)); // Bleu semi-transparent
    pathShape.setOrigin(pathShape.getSize() / 2.0f);

    for (const auto& point : solutionPath)
    {
        // On ne dessine pas sur le depart ni l'arrivee pour rester propre
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
    exploredShape.setFillColor(sf::Color(255, 255, 0, 50)); // Jaune tres transparent

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

    // On suppose que le robot a une methode draw qui prend la window,
    // sinon on le dessine manuellement ici :

    // Calcul de la position interpolation (si disponible) ou grille
    // Pour l'instant on utilise la logique simple basÃ©e sur le code existant
    // Si votre classe Robot a sa propre methode draw, decommenter :
    // playerRobot->draw(window, mazeOffset, CELL_SIZE);

    // Sinon, dessin manuel :
    sf::CircleShape robotShape(CELL_SIZE / 2.5f);
    robotShape.setFillColor(sf::Color::Blue);
    robotShape.setOrigin(CELL_SIZE / 2.5f, CELL_SIZE / 2.5f);

    // Note: playerRobot->getPosition() retourne des entiers (Case de grille)
    // Pour une animation fluide, il faudrait que le robot stocke sa "vraie" position float.
    // Ici on dessine simplement sur la case courante.
    Point pos = playerRobot->getPosition();

    robotShape.setPosition(
        pos.x * CELL_SIZE + mazeOffset.x + CELL_SIZE / 2.0f,
        pos.y * CELL_SIZE + mazeOffset.y + CELL_SIZE / 2.0f
    );

    window.draw(robotShape);
}

// --------------------------------------------------------------------------------
// FONCTION CRUCIALE AJOUTÉE : TOGGLE EDIT MODE
// --------------------------------------------------------------------------------
void GameEngine::toggleEditMode()
{
    if (state == GameState::EDIT_MODE)
    {
        // --- ON QUITTE LE MODE ÉDITION (Clic sur "Done") ---
        state = GameState::IDLE;

        if (gameButtons.size() > 8) {
            gameButtons[8].setText("Edit Mode", font);
        }

        // IMPORTANT : Recalcul intelligent (Garde la position du robot)
        computePath();
    }
    else
    {
        // --- ON ENTRE EN MODE ÉDITION (Clic sur "Edit Mode") ---
        state = GameState::EDIT_MODE;
        isRunning = false;

        if (gameButtons.size() > 3) gameButtons[3].setText("Run", font);
        if (gameButtons.size() > 8) gameButtons[8].setText("Done", font);
    }
}