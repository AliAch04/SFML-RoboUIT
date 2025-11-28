#include "GameEngine.h"
#include "SimpleJSON.h"
#include <fstream>
#include <iostream>
#include <string>

GameEngine::GameEngine() : playerRobot(std::make_unique<Robot>()),
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

void GameEngine::setupMainMenu() {
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

void GameEngine::setupOptionsMenu() {
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

void GameEngine::setupGameUI() {
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

void GameEngine::loadLevel() {
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

void GameEngine::updateMazePosition() {
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

void GameEngine::computePath() {
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

void GameEngine::zoomIn() {
    CELL_SIZE = std::min(Constants::MAX_CELL_SIZE, CELL_SIZE + 5.0f);
    updateMazePosition();
}

void GameEngine::zoomOut() {
    CELL_SIZE = std::max(Constants::MIN_CELL_SIZE, CELL_SIZE - 5.0f);
    updateMazePosition();
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

void GameEngine::toggleRunPause() {
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

void GameEngine::testMaze() {
    if (!currentMaze) return;
    bool solvable = pathFinder->isSolvable(currentMaze.get());
    std::cout << "Maze is " << (solvable ? "SOLVABLE" : "NOT SOLVABLE") << std::endl;
}

void GameEngine::saveMaze() {
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

void GameEngine::resizeMaze() {
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

void GameEngine::run() {
    sf::RenderWindow window(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
        "Robot A* Simulation", sf::Style::Titlebar | sf::Style::Close);
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

void GameEngine::handleMenuEvents(sf::Event& event, sf::RenderWindow& window) {
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

void GameEngine::handleOptionsEvents(sf::Event& event, sf::RenderWindow& window) {
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

void GameEngine::handleGameEvents(sf::Event& event, sf::RenderWindow& window) {
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

void GameEngine::updateGame(float dt) {
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

void GameEngine::drawMainMenu(sf::RenderWindow& window) {
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

void GameEngine::drawOptionsMenu(sf::RenderWindow& window) {
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

void GameEngine::drawGame(sf::RenderWindow& window) {
    // Draw control panel background
    sf::RectangleShape panel(sf::Vector2f(Constants::CONTROL_PANEL_WIDTH, Constants::WINDOW_HEIGHT));
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

void GameEngine::drawMaze(sf::RenderWindow& window) {
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

void GameEngine::drawExploredCells(sf::RenderWindow& window) {
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

void GameEngine::drawPathOverlay(sf::RenderWindow& window) {
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

void GameEngine::drawRobot(sf::RenderWindow& window) {
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