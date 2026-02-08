
#include "GameEngine.h"
#include "SimpleJSON.h"
#include "Logger.h"
#include "Config.h"
#include "MazeBrowser.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm> 
#include <memory>
#include <filesystem>

GameEngine::GameEngine() :
    playerRobot(std::make_unique<LearningRobot>()),
    pathFinder(std::make_unique<AStar>()),
    savedRobotPos({ 0, 0 }),
    savedRobotState(RobotState::IDLE),
    updateInterval(0.1f),

    // Sound UI buttons 
    musicMuteButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Mute", font, 14),
    sfxMuteButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Mute", font, 14),
    musicTestButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Test", font, 14),
    sfxTestButton(sf::Vector2f(80, 30), sf::Vector2f(0, 0), "Test", font, 14),
    backgroundMusicControlButton(sf::Vector2f(150, 30), sf::Vector2f(0, 0), "Toggle Music", font, 16),
    dashboardToggleButton(sf::Vector2f(70, 30), sf::Vector2f(1050 + 480 - 70, 91), "Hide", font, 12),

    mazeBrowserWindow(font, "mazes")

{
    std::cout << "[BUILD CHECK] GameEngine constructor running from edited file\n";
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

    if (!config.load("config.txt")) {
        std::cout << "[CONFIG] load FAILED\n";
    }
    else {
        std::cout << "[CONFIG] load OK\n";
        std::cout << "[CONFIG] robotTexturePath=" << config.robotTexturePath << "\n";
        std::cout << "[CONFIG] wallTexturePath=" << config.wallTexturePath << "\n";
        std::cout << "[CONFIG] floorTexturePath=" << config.floorTexturePath << "\n";
        std::cout << "[CONFIG] obstacleTexturePath=" << config.obstacleTexturePath << "\n";
    }


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
        std::cout << "Background music is playing" << std::endl;
    }
    else {
        std::cout << "Background music is NOT playing" << std::endl;
        // Try to start it manually
        soundManager.startBackgroundMusic();
    }

    std::cout << "Music Volume: " << soundManager.getMusicVolume() << std::endl;
    std::cout << "SFX Volume: " << soundManager.getSFXVolume() << std::endl;
    std::cout << "=== AUDIO INITIALIZATION COMPLETE ===\n" << std::endl;

    // Configurer le callback du navigateur
    mazeBrowserWindow.setOnMazeSelectedCallback([this](const MazeInfo& info) {
        std::cout << "\n=== CHARGEMENT DU LABYRINTHE ===" << std::endl;
        std::cout << "Fichier: " << info.filename << std::endl;

        if (MazeBrowser::LoadMaze(*currentMaze, info.fullPath)) {
            std::cout << "[UI] Chargé avec succès!" << std::endl;

            // IMPORTANT: Effacer le pathfinder et le chemin
            if (pathFinder) {
                pathFinder->clearExplored();  // Vider les cellules explorées
            }
            solutionPath.clear();  // Vider le chemin solution
            pathIndex = 1;  // Réinitialiser l'index du chemin

            // Réinitialiser l'état du robot
            playerRobot->setPosition(currentMaze->startPos);
            playerRobot->setState(RobotState::IDLE);

            // Recréer l'éditeur
            mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
            mazeEditor->setTool(currentTool);

            // Mettre à jour la vue
            updateMazePosition();

            // Recalculer le chemin (il sera vide si besoin)
            computePath();

            // Réinitialiser l'état du jeu
            state = GameState::IDLE;
            isRunning = false;

            if (gameButtons.size() > 3) {
                gameButtons[3].setText("Run", font);
            }

            // Mettre à jour le nom du labyrinthe
            currentMazeName = info.displayName;
            if (mazeNameInput) {
                mazeNameInput->setText(currentMazeName);
            }

            // Message de confirmation
            showTemporaryMessage("Labyrinthe chargé: " + info.displayName, false);

            std::cout << "=== CHARGEMENT TERMINÉ ===\n" << std::endl;

            soundManager.playSound("test_sfx");

        }
        else {
            std::cout << "[ERROR] Échec du chargement!" << std::endl;
            showTemporaryMessage("Échec du chargement!", true);
        }
        });

    // CONFIGURATION DU CALLBACK POUR LE NAVIGATEUR DE LABYRINTHES
    mazeBrowserWindow.setOnMazeSelectedCallback([this](const MazeInfo& info) {
        std::cout << "\n=== CHARGEMENT DEPUIS LE NAVIGATEUR ===" << std::endl;
        std::cout << "Fichier: " << info.filename << std::endl;
        std::cout << "Dimensions: " << info.width << "x" << info.height << std::endl;

        // 1. Charger le labyrinthe
        if (MazeBrowser::LoadMaze(*currentMaze, info.fullPath)) {
            std::cout << "[SUCCÈS] Labyrinthe chargé!" << std::endl;

            // 2. Effacer les données de l'ancien labyrinthe
            if (pathFinder) {
                pathFinder->clearExplored();
            }
            solutionPath.clear();
            pathIndex = 1;

            // 3. Réinitialiser le robot
            playerRobot->setPosition(currentMaze->startPos);
            playerRobot->setState(RobotState::IDLE);

            // 4. Recréer l'éditeur
            mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
            mazeEditor->setTool(currentTool);

            // 5. Mettre à jour la vue
            updateMazePosition();

            // 6. Recalculer le chemin
            computePath();

            // 7. Réinitialiser l'état du jeu
            state = GameState::IDLE;
            isRunning = false;

            // 8. Mettre à jour l'interface
            if (gameButtons.size() > 3) {
                gameButtons[3].setText("Run", font);
            }

            // 9. Mettre à jour le nom du labyrinthe
            currentMazeName = info.displayName;
            if (mazeNameInput) {
                mazeNameInput->setText(currentMazeName);
            }

            // 10. IMPORTANT: Si on est dans l'onglet MY MAZES, retourner au jeu
            if (appState == AppState::OPTIONS && currentOptionTab == OptionsTab::MY_MAZES) {
                appState = AppState::GAME;
                std::cout << "Retour automatique au mode jeu..." << std::endl;
            }

            // 11. Message de confirmation
            showTemporaryMessage("✓ Labyrinthe chargé: " + info.displayName, false);

            std::cout << "=== CHARGEMENT TERMINÉ ===\n" << std::endl;

            // 12. Jouer un son
            soundManager.playSound("test_sfx");

        }
        else {
            std::cout << "[ERREUR] Échec du chargement!" << std::endl;
            showTemporaryMessage("✗ Échec du chargement!", true);
        }
        });

    // Position et taille par défaut du browser (pour l'onglet MY MAZES)
    mazeBrowserWindow.setPosition(sf::Vector2f(200.0f, 200.0f));
    mazeBrowserWindow.setSize(sf::Vector2f(1200.0f, 500.0f));


    // Initialiser les messages
    if (fontLoaded) {
        // Set up dashboard toggle button
        dashboardToggleButton.setPosition(sf::Vector2f(1050.0f + 500.0f - 70.0f, 88.0f));

        // Message de sauvegarde
        saveMessage.setFont(font);
        saveMessage.setCharacterSize(24);
        saveMessage.setFillColor(sf::Color::Green);
        saveMessage.setStyle(sf::Text::Bold);
        saveMessage.setString("");

        // Message d'erreur
        errorMessage.setFont(font);
        errorMessage.setCharacterSize(24);
        errorMessage.setFillColor(sf::Color::Red);
        errorMessage.setStyle(sf::Text::Bold);
        errorMessage.setString("");
    }



    // -------------------- TEXTURE MANAGER INIT (STEP 3) --------------------


    {
        auto abs = [](const std::string& rel) {
            return std::filesystem::absolute(rel).string();
            };

        textureManager.setBaseDir(std::filesystem::absolute("config.txt").parent_path().string());

        // Set defaults ONCE
        textureManager.setDefaults(
            abs("assets/textures/robot.png"),
            abs("assets/textures/wall.png"),
            abs("assets/textures/floor.png"),
            abs("assets/textures/obstacle.png")
        );

        // If config paths are empty, ensure they default to assets paths
        if (config.robotTexturePath.empty())    config.robotTexturePath = "assets/textures/robot.png";
        if (config.wallTexturePath.empty())     config.wallTexturePath = "assets/textures/wall.png";
        if (config.floorTexturePath.empty())    config.floorTexturePath = "assets/textures/floor.png";
        if (config.obstacleTexturePath.empty()) config.obstacleTexturePath = "assets/textures/obstacle.png";

        // Pull persisted paths from config
        textureManager.get(TextureManager::Id::Robot).currentPath = config.robotTexturePath;
        textureManager.get(TextureManager::Id::Wall).currentPath = config.wallTexturePath;
        textureManager.get(TextureManager::Id::Floor).currentPath = config.floorTexturePath;
        textureManager.get(TextureManager::Id::Obstacle).currentPath = config.obstacleTexturePath;

        // Load (robust now, because TextureManager::load resolves relative)
        textureManager.loadAll();

        // Fallback to defaults if something fails
        if (!textureManager.get(TextureManager::Id::Robot).loaded)    textureManager.reset(TextureManager::Id::Robot);
        if (!textureManager.get(TextureManager::Id::Wall).loaded)     textureManager.reset(TextureManager::Id::Wall);
        if (!textureManager.get(TextureManager::Id::Floor).loaded)    textureManager.reset(TextureManager::Id::Floor);
        if (!textureManager.get(TextureManager::Id::Obstacle).loaded) textureManager.reset(TextureManager::Id::Obstacle);

        applyTexturesFromManager();
    }
    // --------------------------------------------------------------


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

    if (trainingVisualizer && dashboardVisible) {
        double loss = 0.0;  // Placeholder since getCurrentLoss might not exist
        double reward = learningRobot->getTotalReward();
        double successRate = learningRobot->getSuccessRate();
        int trainingSteps = learningRobot->getTotalTrials();

        trainingVisualizer->update(loss, reward, successRate, trainingSteps);
    }
}

void GameEngine::setupMainMenu()
{
    if (!fontLoaded) return;

    // --- 1. Chargement de l'arrière-plan (Background) ---
    // Assure-toi que l'image est bien dans le dossier assets
    if (m_bgTexture.loadFromFile("assets/menu_background.png")) {
        m_bgSprite.setTexture(m_bgTexture);

        // Mise à l'échelle pour remplir la fenêtre 1600x900
        // On calcule le facteur d'échelle X et Y
        sf::Vector2u textureSize = m_bgTexture.getSize();
        float scaleX = 1600.0f / textureSize.x;
        float scaleY = 900.0f / textureSize.y;
        m_bgSprite.setScale(scaleX, scaleY);
    }

    // --- 2. Configuration du Titre ---
    titleText.setString("MAZE ROBOT SIMULATION");
    titleText.setCharacterSize(60); // Un peu plus grand pour le style
    titleText.setFillColor(sf::Color::Cyan); // Couleur néon/tech
    titleText.setStyle(sf::Text::Bold);

    // ALGORITHME DE CENTRAGE DU TEXTE
    // On récupère la taille exacte du texte
    sf::FloatRect textRect = titleText.getLocalBounds();
    // On définit l'origine du texte en son centre exact
    titleText.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    // On place le texte au milieu de l'écran (X=800) et en haut (Y=150)
    titleText.setPosition(1600.0f / 2.0f, 150.0f);


    // --- 3. Configuration des Boutons ---
    menuButtons.clear();

    // Dimensions des boutons
    float btnWidth = 250.0f;  // Plus large pour faire pro
    float btnHeight = 60.0f;

    // Calcul pour centrer le bouton : (Largeur_Ecran / 2) - (Largeur_Bouton / 2)
    // 800 - 125 = 675
    float centerX = (1600.0f / 2.0f) - (btnWidth / 2.0f);
    float startY = 400.0f;    // Hauteur du premier bouton
    float gap = 90.0f;        // Espace entre les boutons

    // Ajout des boutons centrés
    // START
    menuButtons.emplace_back(sf::Vector2f(btnWidth, btnHeight),
        sf::Vector2f(centerX, startY),
        "START", font);

    // OPTIONS
    menuButtons.emplace_back(sf::Vector2f(btnWidth, btnHeight),
        sf::Vector2f(centerX, startY + gap),
        "OPTIONS", font);

    // EXIT
    menuButtons.emplace_back(sf::Vector2f(btnWidth, btnHeight),
        sf::Vector2f(centerX, startY + (gap * 2)),
        "EXIT", font);
}
void GameEngine::setupOptionsMenu()
{
    if (!fontLoaded) return;

    // --- 1. CONFIGURATION DU FOND (Background) ---
    // On charge l'image et on l'adapte à la fenêtre 1600x900
    if (m_bgTexture.loadFromFile("assets/menu_background.png")) {
        m_bgSprite.setTexture(m_bgTexture);
        sf::Vector2u textureSize = m_bgTexture.getSize();
        float scaleX = 1600.0f / textureSize.x;
        float scaleY = 900.0f / textureSize.y;
        m_bgSprite.setScale(scaleX, scaleY);
    }

    // --- 2. TITRE (Style Neon/Tech) ---
    optionsTitleText.setString("OPTIONS");
    optionsTitleText.setCharacterSize(60); // Plus grand
    optionsTitleText.setFillColor(sf::Color::Cyan);
    optionsTitleText.setStyle(sf::Text::Bold);

    // Centrage parfait du titre
    sf::FloatRect titleRect = optionsTitleText.getLocalBounds();
    optionsTitleText.setOrigin(titleRect.left + titleRect.width / 2.0f,
        titleRect.top + titleRect.height / 2.0f);
    optionsTitleText.setPosition(1600.0f / 2.0f, 80.0f);

    // --- 3. CRÉATION DES 4 ONGLETS ---
    optionTabButtons.clear();

    float tabW = 180.0f;
    float tabH = 45.0f;
    float gap = 20.0f;
    float totalWidth = (4 * tabW) + (3 * gap);
    float startX = (1600.0f - totalWidth) / 2.0f;
    float tabY = 150.0f;

    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX, tabY), "SETTINGS", font, 18);
    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX + tabW + gap, tabY), "TEXTURES", font, 18);
    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX + (tabW + gap) * 2, tabY), "SOUND", font, 18);
    optionTabButtons.emplace_back(sf::Vector2f(tabW, tabH), sf::Vector2f(startX + (tabW + gap) * 3, tabY), "MY MAZES", font, 18);

    // --- 4. CONTENU (Sliders & Toggles) ---
    optionButtons.clear();
    optionSliders.clear();

    // Paramètres de mise en page
    float sliderWidth = 500.0f;
    float sliderX = (1600.0f - sliderWidth) / 2.0f;

    // Positionnement vertical dynamique : on commence sous les onglets
    float currentY = 280.0f;

    // A. Sliders
    optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(sliderX, currentY), sliderWidth, 0.05f, 0.5f, robotSpeed, "Robot Speed", font));
    currentY += 100.0f; // Espace après slider 1

    optionSliders.push_back(std::make_unique<Slider>(sf::Vector2f(sliderX, currentY), sliderWidth, 10.0f, 60.0f, CELL_SIZE, "Cell Size", font));
    currentY += 120.0f; // Espace plus grand avant les boutons toggle

    // B. Préparation des positions pour les Boutons Toggle
    float toggleBtnWidth = 250.0f;
    float toggleBtnHeight = 50.0f;
    float toggleStartX = (1600.0f - toggleBtnWidth) / 2.0f;
    float gapToggle = 70.0f;

    // --- CALCUL DYNAMIQUE POUR LE BOUTON RETOUR ---
    // Nous avons 3 toggles à placer. On calcule où se trouvera le bas du dernier toggle.
    // Position Toggle 1 = currentY
    // Position Toggle 2 = currentY + gapToggle
    // Position Toggle 3 = currentY + gapToggle * 2
    float lastContentY = currentY + (2 * gapToggle) + toggleBtnHeight;
    float backButtonY = lastContentY + 80.0f; // On ajoute 50px de marge en bas

    // IMPORTANT : On insère le bouton BACK en PREMIER (Index 0)
    // C'est nécessaire car ta fonction drawOptionsMenu dessine optionButtons[0] séparément.
    // Cependant, on lui donne la position Y finale qu'on vient de calculer (backButtonY).
    optionButtons.emplace_back(sf::Vector2f(180, 50), sf::Vector2f((1600.0f - 180.0f) / 2.0f, backButtonY), "BACK", font, 22);

    // C. Ajout des Toggles (qui seront aux Index 1, 2, 3...)

    // Index 1: Explored
    optionButtons.emplace_back(sf::Vector2f(toggleBtnWidth, toggleBtnHeight),
        sf::Vector2f(toggleStartX, currentY),
        showExploredCells ? "Explored: ON" : "Explored: OFF",
        font, 20);

    currentY += gapToggle;

    // Index 2: Path
    optionButtons.emplace_back(sf::Vector2f(toggleBtnWidth, toggleBtnHeight),
        sf::Vector2f(toggleStartX, currentY),
        showPath ? "Path: ON" : "Path: OFF",
        font, 20);

    currentY += gapToggle;

    // Index 3: Keep Pos
    optionButtons.emplace_back(sf::Vector2f(toggleBtnWidth, toggleBtnHeight),
        sf::Vector2f(toggleStartX, currentY),
        preserveRobotState ? "Keep Pos: ON" : "Keep Pos: OFF",
        font, 20);
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

void GameEngine::persistTextureConfig()
{
    config.robotTexturePath = textureManager.get(TextureManager::Id::Robot).currentPath;
    config.wallTexturePath = textureManager.get(TextureManager::Id::Wall).currentPath;
    config.floorTexturePath = textureManager.get(TextureManager::Id::Floor).currentPath;
    config.obstacleTexturePath = textureManager.get(TextureManager::Id::Obstacle).currentPath;

    config.save("config.txt");
}

void GameEngine::createSectionTitle(const std::string& title, float x, float y) {
    sf::Text text;
    text.setFont(font);
    text.setString(title);
    text.setCharacterSize(14);
    text.setFillColor(sf::Color(200, 200, 200)); // Gris clair
    text.setStyle(sf::Text::Bold);
    text.setPosition(x, y);
    sectionTitles.push_back(text);
}

void GameEngine::setupGameUI()
{
    if (!fontLoaded) return;

    // ... Sauvegarde des valeurs inchangée ...

    // --- DÉCLARATION DES VARIABLES DE SAUVEGARDE ---
    std::string nVal = "MyMaze";
    std::string wVal = "20";
    std::string hVal = "20";

    // Récupération des anciennes valeurs si les inputs existent déjà
    if (mazeNameInput) nVal = mazeNameInput->getText();
    if (mazeWidthInput) wVal = mazeWidthInput->getText();
    if (mazeHeightInput) hVal = mazeHeightInput->getText();

    gameButtons.clear();
    sectionTitles.clear();

    const float panelStartX = 1000.0f;
    const float contentX = panelStartX + 20.0f;
    const float fullWidth = 270.0f;
    const float halfWidth = 130.0f;

    // --- RÉGLAGES D'ESPACEMENT CONVENABLES ---
    float currentY = 50.0f;
    const float titleGap = 35.0f; // Espace entre titre et bouton
    const float rowGap = 38.0f;   // Espace entre lignes de boutons
    const float groupGap = 45.0f; // Espace entre groupes
    const int standardFontSize = 14; // TAILLE UNIQUE

    auto addTitle = [&](const std::string& textStr) {
        createSectionTitle(textStr, contentX, currentY);
        currentY += titleGap;
        };

    // 0. CAMERA
    addTitle("Camera Controls");
    gameButtons.emplace_back(sf::Vector2f(50, 28), sf::Vector2f(contentX, currentY), "+", font, 18);
    gameButtons.emplace_back(sf::Vector2f(50, 28), sf::Vector2f(contentX + 55, currentY), "-", font, 18);
    currentY += groupGap;

    // 1. SIMULATION
    if (state != GameState::EDIT_MODE) {
        addTitle("Simulation Controls");
        gameButtons.emplace_back(sf::Vector2f(fullWidth, 28), sf::Vector2f(contentX, currentY), "Generate New", font, standardFontSize);
        currentY += rowGap;
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX, currentY), "Run", font, standardFontSize);
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX + halfWidth + 10, currentY), "Stats", font, standardFontSize);
        currentY += groupGap;
    }

    // 2. MAZE CONFIG
    if (state != GameState::EDIT_MODE) {
        addTitle("MAZE CONFIG");
        mazeNameInput = std::make_unique<TextInput>(sf::Vector2f(fullWidth, 28), sf::Vector2f(contentX, currentY), "Name", font, standardFontSize);
        mazeNameInput->setText(nVal);
        currentY += 52.0f;
        mazeWidthInput = std::make_unique<TextInput>(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX, currentY), "Width", font, standardFontSize);
        mazeWidthInput->setText(wVal);
        mazeHeightInput = std::make_unique<TextInput>(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX + halfWidth + 10, currentY), "Height", font, standardFontSize);
        mazeHeightInput->setText(hVal);
        currentY += groupGap;

        // 3. FILE OPERATIONS (Remis dans le bloc IF pour être caché en Edit Mode)
        addTitle("FILE OPERATIONS");
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX, currentY), "Save", font, standardFontSize);
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX + halfWidth + 10, currentY), "Load", font, standardFontSize);
        currentY += rowGap;
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX, currentY), "Resize", font, standardFontSize);
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX + halfWidth + 10, currentY), "Menu", font, standardFontSize);
        currentY += groupGap;
    }

    // 3. EDITOR TOOLS
    addTitle("Editor Controls");
    std::string editBtnText = (state == GameState::EDIT_MODE) ? "Done" : "Edit Mode";
    gameButtons.emplace_back(sf::Vector2f(fullWidth, 28), sf::Vector2f(contentX, currentY), editBtnText, font, standardFontSize);
    currentY += rowGap;

    if (state == GameState::EDIT_MODE) {
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX, currentY), "Undo", font, standardFontSize);
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX + halfWidth + 10, currentY), "Redo", font, standardFontSize);
        currentY += 40.0f;
        editorToolbar.init(font, contentX, currentY);
    }
    else {
        currentY += groupGap;
    }

    // 4. AI LEARNING
    if (state != GameState::EDIT_MODE) {
        addTitle("Learning Controls:");
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX, currentY), "Auto-Train", font, standardFontSize);
        gameButtons.emplace_back(sf::Vector2f(halfWidth, 28), sf::Vector2f(contentX + halfWidth + 10, currentY), "Auto Mode", font, standardFontSize);
    }

    setupOptionsMenu();
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

    // Center in the LEFT area (0..1000 instead of 0..600 for bigger window)
    float leftAreaWidth = 1000.0f;  // Increased from 600
    mazeOffset.x = (leftAreaWidth - mazeW) / 2.f;
    mazeOffset.y = (600.f - mazeH) / 2.f;  // Keep same vertical centering

    // Clamp so maze stays inside left area with padding
    const float leftPadding = 10.f;
    const float topPadding = 10.f;

    // maxX means: left edge can't go beyond padding
    float maxX = leftPadding;
    // minX means: right edge can't cross leftAreaWidth - leftPadding
    float minX = leftAreaWidth - leftPadding - mazeW;

    // If maze is smaller than area, keep centered
    if (mazeW <= leftAreaWidth - 2.f * leftPadding) {
        mazeOffset.x = (leftAreaWidth - mazeW) / 2.f;
    }
    else {
        mazeOffset.x = std::max(minX, std::min(maxX, mazeOffset.x));
    }

    // For Y clamp within window height
    float windowH = (float)Constants::WINDOW_HEIGHT;  // Now 900
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
    if (!currentMaze) return;

    // 1) Save the CURRENT maze center in SCREEN coords (before zoom)
    float mazeW = currentMaze->width * CELL_SIZE;
    float mazeH = currentMaze->height * CELL_SIZE;
    sf::Vector2f screenCenter = { mazeOffset.x + mazeW / 2.f, mazeOffset.y + mazeH / 2.f };

    // 2) Convert to WORLD coords (cell units)
    sf::Vector2f worldCenter = {
        (screenCenter.x - mazeOffset.x) / CELL_SIZE,
        (screenCenter.y - mazeOffset.y) / CELL_SIZE
    };

    // 3) Apply zoom
    float newCellSize = std::min(Constants::MAX_CELL_SIZE, CELL_SIZE + 5.0f);
    if (newCellSize == CELL_SIZE) return;
    CELL_SIZE = newCellSize;

    // 4) Re-apply offset so the SAME center stays in the SAME place
    mazeOffset.x = screenCenter.x - worldCenter.x * CELL_SIZE;
    mazeOffset.y = screenCenter.y - worldCenter.y * CELL_SIZE;

    soundManager.playSound("test_sfx");
}

void GameEngine::zoomOut()
{
    if (!currentMaze) return;

    float mazeW = currentMaze->width * CELL_SIZE;
    float mazeH = currentMaze->height * CELL_SIZE;
    sf::Vector2f screenCenter = { mazeOffset.x + mazeW / 2.f, mazeOffset.y + mazeH / 2.f };

    sf::Vector2f worldCenter = {
        (screenCenter.x - mazeOffset.x) / CELL_SIZE,
        (screenCenter.y - mazeOffset.y) / CELL_SIZE
    };

    float newCellSize = std::max(Constants::MIN_CELL_SIZE, CELL_SIZE - 5.0f);
    if (newCellSize == CELL_SIZE) return;
    CELL_SIZE = newCellSize;

    mazeOffset.x = screenCenter.x - worldCenter.x * CELL_SIZE;
    mazeOffset.y = screenCenter.y - worldCenter.y * CELL_SIZE;

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

    // Après calcul, vérifier que le path est valide
    if (solutionPath.empty()) {
        std::cout << "Aucun chemin trouvé!" << std::endl;
        state = GameState::FAILED;
    }
    else {
        std::cout << "Chemin trouvé avec " << solutionPath.size() << " étapes" << std::endl;
        state = GameState::SOLVING;
        pathIndex = 1;
    }
}

void GameEngine::generateMaze() {
    if (!currentMaze) return;

    try {
        int width = std::stoi(mazeWidthInput->getText());
        int height = std::stoi(mazeHeightInput->getText());
        int originalW = width;
        int originalH = height;
        width = std::max(5, std::min(30, width));
        height = std::max(5, std::min(30, height));
        mazeWidthInput->setText(std::to_string(width));
        mazeHeightInput->setText(std::to_string(height));
        if (originalW != width || originalH != height)
        {
            std::string msg = "Invalid size (5-30). Auto-corrected to " +
                std::to_string(width) + "x" + std::to_string(height);
            showTemporaryMessage(msg, true);
            std::cout << "[SizeValidation] Input " << originalW << "x" << originalH
                << " -> corrected to " << width << "x" << height << std::endl;
        }


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

void GameEngine::showTemporaryMessage(const std::string& message, bool isError) {
    std::cout << "DEBUG: showTemporaryMessage called: " << message << std::endl;

    if (!fontLoaded) {
        std::cout << "DEBUG: Font not loaded!" << std::endl;
        return;
    }

    std::cout << "DEBUG: Font is loaded, setting text..." << std::endl;

    if (isError) {
        errorMessage.setString(message);
        errorMessage.setFont(font);  // Assurez-vous que la police est définie
        errorMessage.setCharacterSize(24);
        errorMessage.setFillColor(sf::Color::White);  // Texte blanc sur fond rouge
        errorMessage.setStyle(sf::Text::Bold);

        // Centrer le message
        sf::FloatRect bounds = errorMessage.getLocalBounds();
        errorMessage.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        errorMessage.setPosition(Constants::WINDOW_WIDTH / 2.0f,
            Constants::WINDOW_HEIGHT / 2.0f);


        std::cout << "DEBUG: Error message set at position: "
            << errorMessage.getPosition().x << ", "
            << errorMessage.getPosition().y << std::endl;
    }
    else {
        saveMessage.setString(message);
        saveMessage.setFont(font);  // Assurez-vous que la police est définie
        saveMessage.setCharacterSize(24);
        saveMessage.setFillColor(sf::Color::White);  // Texte blanc sur fond vert
        saveMessage.setStyle(sf::Text::Bold);

        // Centrer le message
        sf::FloatRect bounds = saveMessage.getLocalBounds();
        saveMessage.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        saveMessage.setPosition(Constants::WINDOW_WIDTH / 2.0f,
            Constants::WINDOW_HEIGHT / 2.0f);


        std::cout << "DEBUG: Save message set at position: "
            << saveMessage.getPosition().x << ", "
            << saveMessage.getPosition().y << std::endl;
    }

    showMessage = true;
    isErrorMessage = isError;
    messageTimer.restart();
}

void GameEngine::saveMaze()
{
    if (!currentMaze) return;

    if (currentMazeName.empty()) {
        showTemporaryMessage("ERREUR: Nom du labyrinthe vide!", true);
        return;
    }

    std::filesystem::create_directory("mazes");
    std::string filename = "mazes/" + currentMazeName + ".json";

    bool success = MazeBrowser::SaveMaze(*currentMaze, filename);

    if (success)
    {
        std::cout << "[GameEngine] Labyrinthe sauvegarde!" << std::endl;
        showTemporaryMessage("Labyrinthe sauvegarde: " + currentMazeName, false);

        if (mazeBrowserWindow.isVisible()) {
            mazeBrowserWindow.show(); // Rafraîchir
        }
    }
    else
    {
        std::cout << "[GameEngine] ERREUR de sauvegarde!" << std::endl;
        showTemporaryMessage("Échec de la sauvegarde!", true);
    }

    soundManager.playSound("test_sfx");
}

void GameEngine::resizeMaze() {
    if (!currentMaze) return;
    try {
        int newWidth = std::stoi(mazeWidthInput->getText());
        int newHeight = std::stoi(mazeHeightInput->getText());
        newWidth = std::max(5, std::min(30, newWidth));
        newHeight = std::max(5, std::min(30, newHeight));
        mazeWidthInput->setText(std::to_string(newWidth));
        mazeHeightInput->setText(std::to_string(newHeight));


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
    std::cout << "[DEBUG] CWD = " << std::filesystem::current_path() << std::endl;
    std::cout << "[DEBUG] config.txt full path = "
        << (std::filesystem::current_path() / "config.txt") << std::endl;
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
        trainingVisualizer->setPanelMode(true);
        trainingVisualizer->setPanelPosition(sf::Vector2f(1050.0f, 80.0f));  // Moved right from 620
        trainingVisualizer->setPanelSize(sf::Vector2f(500.0f, 400.0f));     // Larger panel
        trainingVisualizer->setVisible(dashboardVisible);
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

        // Mettre à jour le navigateur si visible
        if (mazeBrowserWindow.isVisible()) {
            // Gérer les fermetures avec Escape
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                mazeBrowserWindow.hide();
            }
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

        // Contenu TEXTURES
        if (currentOptionTab == OptionsTab::TEXTURES) {
            optionsTexturePanel.handleHover(mousePos);
        }

        // Contenu SOUND
        if (currentOptionTab == OptionsTab::SOUND) {
            // Setup UI on first activation
            if (!musicVolumeSlider) {
                setupSoundUI();
            }

            // Hover for sound UI
            if (fontLoaded) {
                musicMuteButton.setHovered(musicMuteButton.contains(mousePos));
                sfxMuteButton.setHovered(sfxMuteButton.contains(mousePos));
                musicTestButton.setHovered(musicTestButton.contains(mousePos));
                sfxTestButton.setHovered(sfxTestButton.contains(mousePos));
                backgroundMusicControlButton.setHovered(backgroundMusicControlButton.contains(mousePos));
            }

            // Handle slider dragging
            if (musicVolumeSlider && musicVolumeSlider->isDragging()) {
                musicVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setMusicVolume(musicVolumeSlider->getValue());
                config.musicVolume = musicVolumeSlider->getValue();
                updateMusicStatusText();
            }

            if (sfxVolumeSlider && sfxVolumeSlider->isDragging()) {
                sfxVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setSFXVolume(sfxVolumeSlider->getValue());
                config.sfxVolume = sfxVolumeSlider->getValue();
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
                else if (i == 3) {
                    currentOptionTab = OptionsTab::MY_MAZES;
                    // Rafraîchir la liste quand on entre dans cet onglet
                    mazeBrowserWindow.show();
                }

                // Setup sound UI when switching to sound tab
                if (i == 2 && !musicVolumeSlider) {
                    setupSoundUI();
                }
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

        // 4. Interactions onglet TEXTURES
        else if (currentOptionTab == OptionsTab::TEXTURES) {
            // Position the control panel for the options screen
            controlPanel.setPosition(sf::Vector2f(120.f, 200.f));
            controlPanel.setSize(sf::Vector2f(560.f, 420.f));

            // Handle clicks on the control panel
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

                            // Persist immediately
                            config.robotTexturePath = textureManager.get(TextureManager::Id::Robot).currentPath;
                            config.wallTexturePath = textureManager.get(TextureManager::Id::Wall).currentPath;
                            config.floorTexturePath = textureManager.get(TextureManager::Id::Floor).currentPath;
                            config.obstacleTexturePath = textureManager.get(TextureManager::Id::Obstacle).currentPath;
                            config.save("config.txt");
                        }
                    }
                },
                [&](TextureManager::Id id)
                {
                    if (textureManager.reset(id))
                    {
                        applyTexturesFromManager();

                        // Persist immediately
                        config.robotTexturePath = textureManager.get(TextureManager::Id::Robot).currentPath;
                        config.wallTexturePath = textureManager.get(TextureManager::Id::Wall).currentPath;
                        config.floorTexturePath = textureManager.get(TextureManager::Id::Floor).currentPath;
                        config.obstacleTexturePath = textureManager.get(TextureManager::Id::Obstacle).currentPath;
                        config.save("config.txt");
                    }
                }
            );
        }

        // 5. Interactions onglet MY MAZES
        else if (currentOptionTab == OptionsTab::MY_MAZES) {
            // Gérer les clics dans le navigateur
            mazeBrowserWindow.handleEvent(event, mousePos);
        }

        // 6. Interactions onglet SOUND
        else if (currentOptionTab == OptionsTab::SOUND) {
            // Setup UI on first activation
            if (!musicVolumeSlider) {
                setupSoundUI();
            }

            // Handle sound UI clicks
            if (fontLoaded && musicMuteButton.contains(mousePos)) {
                bool newMuteState = !soundManager.isMusicMuted();
                soundManager.muteMusic(newMuteState);
                musicMuteButton.setText(newMuteState ? "Unmute" : "Mute", font);
                config.musicMuted = newMuteState;
                updateMusicStatusText();
                backgroundMusicControlButton.setText(
                    soundManager.isBackgroundMusicPlaying() ? "Stop Music" : "Start Music",
                    font
                );
            }
            else if (fontLoaded && sfxMuteButton.contains(mousePos)) {
                bool newMuteState = !soundManager.isSFXMuted();
                soundManager.muteSFX(newMuteState);
                sfxMuteButton.setText(newMuteState ? "Unmute" : "Mute", font);
                config.sfxMuted = newMuteState;
            }
            else if (fontLoaded && musicTestButton.contains(mousePos)) {
                soundManager.playTestMusic();
                updateMusicStatusText();
                backgroundMusicControlButton.setText("Stop Music", font);
            }
            else if (fontLoaded && sfxTestButton.contains(mousePos)) {
                soundManager.playTestSFX();
            }
            else if (fontLoaded && backgroundMusicControlButton.contains(mousePos)) {
                if (soundManager.isBackgroundMusicPlaying()) {
                    soundManager.stopBackgroundMusic();
                    backgroundMusicControlButton.setText("Start Music", font);
                }
                else {
                    soundManager.startBackgroundMusic();
                    backgroundMusicControlButton.setText("Stop Music", font);
                }
                updateMusicStatusText();
            }
            else if (musicVolumeSlider && musicVolumeSlider->contains(mousePos)) {
                musicVolumeSlider->setDragging(true);
                musicVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setMusicVolume(musicVolumeSlider->getValue());
                config.musicVolume = musicVolumeSlider->getValue();
                updateMusicStatusText();
            }
            else if (sfxVolumeSlider && sfxVolumeSlider->contains(mousePos)) {
                sfxVolumeSlider->setDragging(true);
                sfxVolumeSlider->setValueFromMouse(mousePos);
                soundManager.setSFXVolume(sfxVolumeSlider->getValue());
                config.sfxVolume = sfxVolumeSlider->getValue();
            }
        }
    }

    // Relâchement souris
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        for (auto& slider : optionSliders) slider->setDragging(false);
        if (musicVolumeSlider) musicVolumeSlider->setDragging(false);
        if (sfxVolumeSlider) sfxVolumeSlider->setDragging(false);
    }

    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        appState = AppState::MAIN_MENU;
}

void GameEngine::setTool(EditorTool tool)
{
    currentTool = tool;
    if (mazeEditor) mazeEditor->setTool(tool);
}

void GameEngine::handleGameEvents(sf::Event& event, sf::RenderWindow& window)
{
    // --- HELPER : GARDE LE LABYRINTHE À L'ÉCRAN ---
    auto clampMazeOffset = [&]() {
        if (!currentMaze) return;
        float mazeW = currentMaze->width * CELL_SIZE;
        float mazeH = currentMaze->height * CELL_SIZE;
        const float areaW = 1000.f;
        const float areaH = (float)Constants::WINDOW_HEIGHT;
        const float pad = 10.f;

        if (mazeW <= areaW - 2.f * pad) mazeOffset.x = (areaW - mazeW) / 2.f;
        else mazeOffset.x = std::max(areaW - pad - mazeW, std::min(pad, mazeOffset.x));

        if (mazeH <= areaH - 2.f * pad) mazeOffset.y = (areaH - mazeH) / 2.f;
        else mazeOffset.y = std::max(areaH - pad - mazeH, std::min(pad, mazeOffset.y));
        };

    // 1. GESTION DES MOUVEMENTS SOURIS (SURVOL ET DRAG)
    if (event.type == sf::Event::MouseMoved)
    {
        sf::Vector2f mousePos((float)event.mouseMove.x, (float)event.mouseMove.y);

        if (isPanning && currentMaze) {
            if (mousePos.x > 1000.f) isPanning = false;
            else {
                sf::Vector2f delta = mousePos - lastMousePos;
                lastMousePos = mousePos;
                mazeOffset += delta;
                clampMazeOffset();
            }
            return;
        }

        dashboardToggleButton.setHovered(dashboardToggleButton.contains(mousePos));

        // Survol des boutons du panneau
        for (auto& btn : gameButtons) {
            btn.setHovered(btn.contains(mousePos));
        }

        // Mode Édition : Toolbar et Ghost
        if (state == GameState::EDIT_MODE) {
            editorToolbar.handleHover(mousePos);
            if (mazeEditor) mazeEditor->updateGhost(window, CELL_SIZE, mazeOffset);

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && mousePos.x <= 1000.f && mazeEditor) {
                mazeEditor->applyTool();
            }
        }
    }

    // 2. GESTION DES CLICS SOURIS (BOUTON PRESSED)
    if (event.type == sf::Event::MouseButtonPressed)
    {
        sf::Vector2f mousePos((float)event.mouseButton.x, (float)event.mouseButton.y);

        // CLIC DROIT : Panoramique
        if (event.mouseButton.button == sf::Mouse::Right) {
            if (currentMaze && mousePos.x <= 1000.f) {
                isPanning = true;
                lastMousePos = mousePos;
            }
        }

        // CLIC GAUCHE : Interactions UI
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            // A. Dashboard Toggle
            if (dashboardToggleButton.contains(mousePos)) {
                dashboardVisible = !dashboardVisible;
                if (trainingVisualizer) trainingVisualizer->setVisible(dashboardVisible);
                dashboardToggleButton.setText(dashboardVisible ? "Hide" : "Show", font);
                return;
            }

            // B. Outils de dessin (Mode Édition)
            if (state == GameState::EDIT_MODE) {
                if (editorToolbar.handleClick(mousePos)) {
                    currentTool = editorToolbar.getSelectedTool();
                    if (mazeEditor) mazeEditor->setTool(currentTool);
                    return;
                }
                else if (mousePos.x <= 1000.f && mazeEditor) {
                    mazeEditor->applyTool();
                    return;
                }
            }

            // C. LOGIQUE UNIFIÉE DES BOUTONS (Basée sur le texte)
            for (size_t i = 0; i < gameButtons.size(); ++i)
            {
                if (gameButtons[i].contains(mousePos))
                {
                    std::string btnText = gameButtons[i].getText();

                    // --- CAMERA ---
                    if (btnText == "+") { zoomIn(); clampMazeOffset(); }
                    else if (btnText == "-") { zoomOut(); clampMazeOffset(); }

                    // --- SIMULATION ---
                    else if (btnText == "Generate New") generateMaze();
                    else if (btnText == "Run" || btnText == "Pause") toggleRunPause();
                    else if (btnText == "Stats") testMaze();

                    // --- CONFIG & FILES ---
                    else if (btnText == "Sauver") saveMaze();
                    else if (btnText == "Charger") mazeBrowserWindow.show();
                    else if (btnText == "Resize") resizeMaze();
                    else if (btnText == "Menu") appState = AppState::MAIN_MENU;

                    // --- EDITOR TOOLS (Boutons qui étaient bloqués) ---
                    else if (btnText == "Edit Mode" || btnText == "Done") toggleEditMode();
                    else if (btnText == "Undo") { if (mazeEditor) mazeEditor->undo(); }
                    else if (btnText == "Redo") { if (mazeEditor) mazeEditor->redo(); }

                    // --- AI LEARNING ---
                    else if (btnText == "Auto-Train") {
                        auto* robot = dynamic_cast<LearningRobot*>(playerRobot.get());
                        if (robot) { robot->runEvolutionaryOptimization(50); updateDashboards(); }
                    }
                    else if (btnText == "Auto Mode" || btnText == "Manual") {
                        auto* robot = dynamic_cast<LearningRobot*>(playerRobot.get());
                        if (robot) {
                            bool isAuto = (robot->getLearningMode() == LearningRobot::LearningMode::AUTONOMOUS);
                            robot->setLearningMode(isAuto ? LearningRobot::LearningMode::MANUAL : LearningRobot::LearningMode::AUTONOMOUS);
                            gameButtons[i].setText(isAuto ? "Auto Mode" : "Manual", font);
                        }
                    }
                    return; // Clic traité, on quitte la fonction
                }
            }

            // D. GESTION DES INPUTS TEXTE (Focus)
            if (state != GameState::EDIT_MODE) {
                bool nameClick = mazeNameInput && mazeNameInput->contains(mousePos);
                bool wClick = mazeWidthInput && mazeWidthInput->contains(mousePos);
                bool hClick = mazeHeightInput && mazeHeightInput->contains(mousePos);

                if (mazeNameInput) mazeNameInput->setFocused(nameClick);
                if (mazeWidthInput) mazeWidthInput->setFocused(wClick);
                if (mazeHeightInput) mazeHeightInput->setFocused(hClick);
            }
        }
    }

    // 3. RELÂCHEMENT SOURIS
    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Right) isPanning = false;
    }

    // 4. ÉVÉNEMENTS TEXTE ET CLAVIER
    if (mazeBrowserWindow.isVisible()) {
        mazeBrowserWindow.handleEvent(event, sf::Vector2f((float)sf::Mouse::getPosition(window).x, (float)sf::Mouse::getPosition(window).y));
    }

    if (event.type == sf::Event::TextEntered) {
        if (mazeNameInput && mazeNameInput->isFocused()) mazeNameInput->handleTextEntered(event.text.unicode);
        if (mazeWidthInput && mazeWidthInput->isFocused()) mazeWidthInput->handleTextEntered(event.text.unicode);
        if (mazeHeightInput && mazeHeightInput->isFocused()) mazeHeightInput->handleTextEntered(event.text.unicode);
        if (mazeNameInput) currentMazeName = mazeNameInput->getText();
    }

    if (event.type == sf::Event::KeyPressed) {
        bool typing = (mazeNameInput && mazeNameInput->isFocused()) || (mazeWidthInput && mazeWidthInput->isFocused()) || (mazeHeightInput && mazeHeightInput->isFocused());

        if (event.key.code == sf::Keyboard::Escape) {
            if (typing) {
                if (mazeNameInput) mazeNameInput->setFocused(false);
                if (mazeWidthInput) mazeWidthInput->setFocused(false);
                if (mazeHeightInput) mazeHeightInput->setFocused(false);
            }
            else appState = AppState::MAIN_MENU;
            return;
        }

        if (!typing) {
            if (event.key.code == sf::Keyboard::E) toggleEditMode();
            if (event.key.code == sf::Keyboard::K) saveMaze();
            if (state == GameState::EDIT_MODE && mazeEditor) {
                if (event.key.control && event.key.code == sf::Keyboard::Z) mazeEditor->undo();
                if (event.key.control && event.key.code == sf::Keyboard::Y) mazeEditor->redo();
            }
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
    // 1. DESSINER LE FOND
    window.draw(m_bgSprite);

    // 2. DESSINER LE TITRE
    if (fontLoaded) {
        window.draw(titleText);
    }

    // 3. DESSINER LES BOUTONS - Remove const from loop
    for (auto& button : menuButtons) {  // Change 'const auto&' to 'auto&'
        button.draw(window);
    }
}

void GameEngine::setupTexturesUI() {
    if (!fontLoaded) return;
    
    // --- TITLE TEXT (from issue39) ---
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    titleText.setPosition(400.0f, 150.0f);

    // --- PANEL CONFIGURATION (from main) ---
    float panelWidth = 800.0f;
    float panelX = (1600.0f - panelWidth) / 2.0f; // 400.0f
    float panelY = 250.0f;
    float panelHeight = 500.0f;

    // --- POSITIONS ---
    float centerX = 1600.0f / 2.0f;       // 800.0f
    float centerY = panelY + (panelHeight / 2.0f); // Centre vertical du panneau

    // --- CREATION OF ARROW BUTTONS ---
    // Previous Button (<) to LEFT of robot
    texturePrevButton = Button(
        sf::Vector2f(60, 60),             // Square size
        sf::Vector2f(centerX - 250, centerY - 30), // Position: 250px left of center
        "<", font, 30
    );

    // Next Button (>) to RIGHT of robot
    textureNextButton = Button(
        sf::Vector2f(60, 60),             // Square size
        sf::Vector2f(centerX + 190, centerY - 30), // Position: 190px right (symmetrical)
        ">", font, 30
    );

    // "Select" Button (Optional, at bottom)
    textureSelectButton = Button(
        sf::Vector2f(200, 50),
        sf::Vector2f(centerX - 100, panelY + panelHeight - 80),
        "SELECT SKIN", font, 20
    );
}

void GameEngine::drawOptionsMenu(sf::RenderWindow& window)
{
    // --- 1. DESSINER LE FOND (AJOUT CRUCIAL) ---
    // On dessine l'arrière-plan "Espace/Circuit" en premier
    window.draw(m_bgSprite);

    if (!fontLoaded) return;

    // --- 2. TITRE DU MENU ---
    sf::FloatRect titleBounds = optionsTitleText.getLocalBounds();
    optionsTitleText.setOrigin(titleBounds.width / 2.0f, titleBounds.height / 2.0f);
    optionsTitleText.setPosition(800.0f, 80.0f);  // Centré à 800 (1600/2)
    window.draw(optionsTitleText);

    // --- 3. DESSINER LES 4 ONGLETS ---
    for (size_t i = 0; i < optionTabButtons.size(); ++i) {
        optionTabButtons[i].draw(window);

        // Souligner l'onglet actif
        bool isActive = (i == 0 && currentOptionTab == OptionsTab::SETTINGS) ||
            (i == 1 && currentOptionTab == OptionsTab::TEXTURES) ||
            (i == 2 && currentOptionTab == OptionsTab::SOUND) ||
            (i == 3 && currentOptionTab == OptionsTab::MY_MAZES);

        if (isActive) {
            sf::RectangleShape underline(sf::Vector2f(180.0f, 3.0f)); // Largeur fixe de l'onglet
            underline.setFillColor(sf::Color::Cyan);

            // Calcul de la position exacte pour s'aligner sous l'onglet
            float tabW = 180.0f;
            float gap = 20.0f;
            float totalWidth = (4 * tabW) + (3 * gap);
            float startX = (1600.0f - totalWidth) / 2.0f;

            // Positionnement sous l'onglet
            underline.setPosition(startX + i * (tabW + gap), 150.0f + 55.0f); // tabY + tabHeight
            window.draw(underline);
        }
    }

    // --- 4. BOUTON RETOUR (Toujours visible) ---
    if (optionButtons.size() > 0) {
        optionButtons[0].draw(window);
    }

    // --- 5. CONTENU VARIABLE SELON L'ONGLET ---

    // CAS 1 : SETTINGS
    if (currentOptionTab == OptionsTab::SETTINGS) {
        // Affiche les sliders
        for (const auto& slider : optionSliders) slider->draw(window);

        // Affiche les boutons d'options (On commence à 1 car 0 est le bouton Retour)
        for (size_t i = 1; i < optionButtons.size(); ++i) {
            optionButtons[i].draw(window);
        }
    }
    // CAS 2 : TEXTURES (Configuration)
    else if (currentOptionTab == OptionsTab::TEXTURES) {

        // --- 1. LE STYLE (FOND ET TITRE) ---
        float panelWidth = 900.0f;
        float panelHeight = 480.0f;
        float panelX = (1600.0f - panelWidth) / 2.0f;
        float panelY = 220.0f;

        sf::RectangleShape panel(sf::Vector2f(panelWidth, panelHeight));
        panel.setPosition(panelX, panelY);
        panel.setFillColor(sf::Color(30, 30, 30, 220));
        panel.setOutlineThickness(2.0f);
        panel.setOutlineColor(sf::Color::Cyan);
        window.draw(panel);

        sf::Text title("TEXTURE CONFIGURATION", font, 26);
        title.setFillColor(sf::Color::Cyan);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.width / 2.0f, 0);
        title.setPosition(panelX + panelWidth / 2.0f, panelY + 15.0f);
        window.draw(title);

        // --- 2. POSITIONS (GRID) ---
        float startX = panelX + 60.0f;
        float colButtonsX = panelX + 300.0f; // Là où iront les boutons plus tard

        float row1Y = panelY + 80.0f;  // Robot
        float row2Y = panelY + 170.0f; // Mur
        float row3Y = panelY + 260.0f; // Sol
        float row4Y = panelY + 350.0f; // Obstacle

        // --- 3. LABELS (Ce qui est fait) ---

        // Ligne 1 : Robot
        sf::Text lblRobot("ROBOT", font, 20);
        lblRobot.setPosition(startX, row1Y);
        window.draw(lblRobot);
        // TODO: Intégrer les boutons Upload/Reset pour Robot ici (Position : colButtonsX, row1Y)

        // Ligne 2 : Wall
        sf::Text lblWall("WALL", font, 20);
        lblWall.setPosition(startX, row2Y);
        window.draw(lblWall);
        // TODO: Intégrer les boutons Upload/Reset pour Wall ici (Position : colButtonsX, row2Y)

        // Ligne 3 : Floor
        sf::Text lblFloor("FLOOR", font, 20);
        lblFloor.setPosition(startX, row3Y);
        window.draw(lblFloor);
        // TODO: Intégrer les boutons Upload/Reset pour Floor ici (Position : colButtonsX, row3Y)

        // Ligne 4 : Obstacle
        sf::Text lblObs("OBSTACLE", font, 20);
        lblObs.setPosition(startX, row4Y);
        window.draw(lblObs);
        // TODO: Intégrer les boutons Upload/Reset pour Obstacle ici (Position : colButtonsX, row4Y)
    }
    // --- PAGE SOUND ---
    else if (currentOptionTab == OptionsTab::SOUND) {
        if (!musicVolumeSlider) setupSoundUI();

        // 1. DESSINER LE FOND (CENTRÉ)
        // Les dimensions et positions doivent matcher celles de setupSoundUI
        float panelWidth = 560.0f;
        float panelHeight = 300.0f;
        float panelX = (1600.0f - panelWidth) / 2.0f; // Centrage horizontal
        float panelY = 250.0f;                        // Position verticale fixe

        sf::RectangleShape panel(sf::Vector2f(panelWidth, panelHeight));
        panel.setPosition(panelX, panelY);
        panel.setFillColor(sf::Color(30, 30, 30, 200));
        panel.setOutlineThickness(1.0f);
        panel.setOutlineColor(sf::Color::Cyan);
        window.draw(panel);

        // Variables pour aligner les textes comme dans setupSoundUI
        float contentStartX = panelX + 40.0f; // Marge gauche interne
        float contentStartY = panelY + 40.0f; // Marge haute interne
        float rowHeight = 80.0f;

        // 2. LIGNE MUSIQUE
        sf::Text musicLabel("Music Volume", font, 16);
        musicLabel.setPosition(contentStartX, contentStartY);
        musicLabel.setFillColor(sf::Color::Cyan);
        window.draw(musicLabel);

        if (musicVolumeSlider) musicVolumeSlider->draw(window);

        // 3. LIGNE SFX
        sf::Text sfxLabel("SFX Volume", font, 16);
        sfxLabel.setPosition(contentStartX, contentStartY + rowHeight);
        sfxLabel.setFillColor(sf::Color::Cyan);
        window.draw(sfxLabel);

        if (sfxVolumeSlider) sfxVolumeSlider->draw(window);

        // 4. DESSINER LES BOUTONS
        // Leurs positions sont déjà gérées dans setupSoundUI, on fait juste draw
        musicMuteButton.draw(window);
        sfxMuteButton.draw(window);
        musicTestButton.draw(window);
        sfxTestButton.draw(window);
        backgroundMusicControlButton.draw(window);

        // 5. STATUS EN PETIT (MUTED)
        // On aligne le texte "MUTED" juste à côté du label "Volume"
        if (soundManager.isMusicMuted()) {
            sf::Text m("MUTED", font, 12);
            m.setFillColor(sf::Color::Red);
            m.setPosition(contentStartX + 120.0f, contentStartY + 2);
            window.draw(m);
        }
        if (soundManager.isSFXMuted()) {
            sf::Text s("MUTED", font, 12);
            s.setFillColor(sf::Color::Red);
            s.setPosition(contentStartX + 110.0f, contentStartY + rowHeight + 2);
            window.draw(s);
        }

        // 6. STATUS GENERAL (Playing/Paused)
        sf::Text statusText;
        statusText.setFont(font);
        statusText.setCharacterSize(14);

        if (soundManager.isBackgroundMusicPlaying()) {
            statusText.setString("Status: Playing");
            statusText.setFillColor(sf::Color::Green);
        }
        else {
            statusText.setString("Status: Paused");
            statusText.setFillColor(sf::Color(255, 255, 0));
        }

        // Centrage du texte statut par rapport au panneau
        sf::FloatRect bounds = statusText.getLocalBounds();
        statusText.setOrigin(bounds.width / 2.0f, 0);

        // Position X = Centre du panneau, Y = sous le 2ème slider
        float centerX = panelX + (panelWidth / 2.0f);
        statusText.setPosition(centerX, contentStartY + rowHeight * 2 + 45.0f);

        window.draw(statusText);
    }
    // CAS 4 : MY MAZES
    else if (currentOptionTab == OptionsTab::MY_MAZES) {
        // Navigateur de labyrinthes - CENTRÉ
        float browserWidth = 1200.0f;
        float browserHeight = 500.0f;
        float browserX = (1600.0f - browserWidth) / 2.0f;
        float browserY = 230.0f;

        mazeBrowserWindow.setPosition(sf::Vector2f(browserX, browserY));
        mazeBrowserWindow.setSize(sf::Vector2f(browserWidth, browserHeight));
        mazeBrowserWindow.update(); // Note: Update dans le draw n'est pas idéal mais conservé pour ta logique
        mazeBrowserWindow.draw(window);

    }
}

void GameEngine::drawGame(sf::RenderWindow& window)
{
    // 1. Fond et Labyrinthe (Inchangé)
    if (floorTexture.getSize().x > 0 && currentMaze) {
        // ... (votre code de dessin du sol) ...
        floorSprite.setScale(CELL_SIZE / (float)floorTexture.getSize().x, CELL_SIZE / (float)floorTexture.getSize().y);
        for (int y = 0; y < currentMaze->height; ++y) {
            for (int x = 0; x < currentMaze->width; ++x) {
                floorSprite.setPosition(x * CELL_SIZE + mazeOffset.x, y * CELL_SIZE + mazeOffset.y);
                window.draw(floorSprite);
            }
        }
    }
    drawMaze(window);
    if (showPath) drawPathOverlay(window);
    if (showExploredCells) drawExploredCells(window);
    drawRobot(window);

    // 2. PANNEAU LATÉRAL
    window.draw(controlPanelBackground);
    gameTitleText.setPosition(1060, 20);
    window.draw(gameTitleText);

    // Dessiner tous les titres de sections créés
    for (const auto& title : sectionTitles) {
        window.draw(title);
    }

    // Dessiner tous les boutons présents dans le vecteur
    // On cache juste Undo/Redo en mode normal
    for (size_t i = 0; i < gameButtons.size(); ++i)
    {
        if (state != GameState::EDIT_MODE) {
            // Dans votre setupUI, Undo/Redo sont créés après Edit Mode
            // On vérifie le texte pour être sûr de ne pas se tromper d'index
            if (gameButtons[i].getText() == "Undo" || gameButtons[i].getText() == "Redo") continue;
        }
        gameButtons[i].draw(window);
    }

    // Dessiner les Inputs seulement en mode normal
    if (state != GameState::EDIT_MODE) {
        if (mazeNameInput) mazeNameInput->draw(window);
        if (mazeWidthInput) mazeWidthInput->draw(window);
        if (mazeHeightInput) mazeHeightInput->draw(window);
    }

    // Barre d'outils en mode édition
    if (state == GameState::EDIT_MODE) {
        editorToolbar.draw(window);
        if (mazeEditor) mazeEditor->draw(window);
    }

    // 4. Overlays (Browser, Dashboard, Messages) - Inchangé
    mazeBrowserWindow.update();
    mazeBrowserWindow.draw(window);

    if (dashboardVisible && trainingVisualizer) {
        trainingVisualizer->draw();
    }
    dashboardToggleButton.draw(window);

    // Messages Popup
    if (showMessage && messageTimer.getElapsedTime().asSeconds() < 3.0f) {
        // Combined approach: using references but with improved positioning

        sf::Text* currentMsg = isErrorMessage ? &errorMessage : &saveMessage;
        sf::Color bgColor = isErrorMessage ? sf::Color(50, 0, 0, 230) : sf::Color(0, 50, 0, 230);
        sf::Color outlineColor = isErrorMessage ? sf::Color::Red : sf::Color::Green;

        // Calculate message bounds with minimum width
        sf::FloatRect textBounds = currentMsg->getLocalBounds();
        float minWidth = 300.0f;  // Minimum width
        float backgroundWidth = std::max(minWidth, textBounds.width + 40.0f);
        float backgroundHeight = textBounds.height + 20.0f;

        // Create background
        sf::RectangleShape msgBackground(sf::Vector2f(backgroundWidth, backgroundHeight));

        // Position background (centered)
        msgBackground.setPosition(
            (Constants::WINDOW_WIDTH - backgroundWidth) / 2.0f,
            isErrorMessage ?
            (Constants::WINDOW_HEIGHT / 2.0f) - backgroundHeight / 2.0f : // Center vertically for errors
            300.0f - backgroundHeight / 2.0f  // Fixed position for save messages
        );

        // Style background
        msgBackground.setFillColor(bgColor);
        msgBackground.setOutlineThickness(3);
        msgBackground.setOutlineColor(outlineColor);

        // Position text over background
        currentMsg->setPosition(
            (Constants::WINDOW_WIDTH - textBounds.width) / 2.0f,
            msgBackground.getPosition().y + (backgroundHeight - textBounds.height) / 2.0f
        );

        // Draw everything
        window.draw(msgBackground);
        window.draw(*currentMsg);

        // Debug (optional)
#ifdef DEBUG
        std::cout << "DEBUG: Drawing message: " << currentMsg->getString().toAnsiString()
            << " at " << currentMsg->getPosition().x << ", "
            << currentMsg->getPosition().y << std::endl;
#endif
    }
    else {
        showMessage = false;
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
    if (!currentMaze) return;  // Vérification supplémentaire

    sf::RectangleShape pathShape(sf::Vector2f(CELL_SIZE / 3.0f, CELL_SIZE / 3.0f));
    pathShape.setFillColor(sf::Color(50, 50, 255, 150));
    pathShape.setOrigin(pathShape.getSize() / 2.0f);

    for (const auto& point : solutionPath) {
        // Vérifier que le point est dans les limites du labyrinthe courant
        if (point.x < 0 || point.x >= currentMaze->width ||
            point.y < 0 || point.y >= currentMaze->height) {
            continue;  // Ignorer les points hors limites
        }

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
    if (!currentMaze) return;  // Vérification supplémentaire

    const auto& explored = pathFinder->getExplored();
    sf::RectangleShape exploredShape(sf::Vector2f(CELL_SIZE - 4.0f, CELL_SIZE - 4.0f));
    exploredShape.setFillColor(sf::Color(255, 255, 0, 50));

    for (const auto& point : explored) {
        // Vérifier que le point est dans les limites du labyrinthe courant
        if (point.x < 0 || point.x >= currentMaze->width ||
            point.y < 0 || point.y >= currentMaze->height) {
            continue;  // Ignorer les points hors limites
        }

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

    // --- CALCUL DU CENTRAGE (Alignement sur 1600px) ---
    // On définit les dimensions du panneau pour qu'il soit identique au visuel
    float panelWidth = 560.0f;
    float panelX = (1600.0f - panelWidth) / 2.0f; // Centré horizontalement (approx 520.0f)

    // Position Verticale (Doit correspondre à la zone de contenu sous les onglets)
    float startY = 250.0f;

    // Marges internes
    float paddingX = 40.0f;
    float paddingTop = 40.0f;

    // --- CONFIGURATION DES POSITIONS ---
    float sliderWidth = 250.0f;
    float rowHeight = 80.0f; // Espace entre ligne Musique et ligne SFX

    // Position X des éléments
    float sliderX = panelX + paddingX;
    float buttonsX = sliderX + sliderWidth + 80.0f; // Les boutons démarrent 30px après le slider

    // 1. SLIDERS (Titre vide car géré par drawOptionsMenu)
    musicVolumeSlider = std::make_unique<Slider>(
        sf::Vector2f(sliderX, startY + paddingTop + 25),
        sliderWidth, 0.0f, 100.0f,
        soundManager.getMusicVolume(),
        "", font
    );

    sfxVolumeSlider = std::make_unique<Slider>(
        sf::Vector2f(sliderX, startY + paddingTop + rowHeight + 25),
        sliderWidth, 0.0f, 100.0f,
        soundManager.getSFXVolume(),
        "", font
    );

    // 2. BOUTONS MUTE (À droite des sliders)
    musicMuteButton = Button(sf::Vector2f(70, 30), sf::Vector2f(buttonsX, startY + paddingTop + 15),
        soundManager.isMusicMuted() ? "Unmute" : "Mute", font, 14);

    sfxMuteButton = Button(sf::Vector2f(70, 30), sf::Vector2f(buttonsX, startY + paddingTop + rowHeight + 15),
        soundManager.isSFXMuted() ? "Unmute" : "Mute", font, 14);

    // 3. BOUTONS TEST (À droite des boutons Mute)
    // On ajoute un petit espace (10px) après le bouton Mute
    float testButtonX = buttonsX + 70.0f + 10.0f;

    musicTestButton = Button(sf::Vector2f(60, 30), sf::Vector2f(testButtonX, startY + paddingTop + 15), "Test", font, 14);
    sfxTestButton = Button(sf::Vector2f(60, 30), sf::Vector2f(testButtonX, startY + paddingTop + rowHeight + 15), "Test", font, 14);

    // 4. GROS BOUTON STOP/PLAY (Centré en bas du panneau)
    float bigButtonWidth = 160.0f;
    // Calcul pour centrer le bouton DANS le panneau : panelX + (panelWidth - buttonWidth) / 2
    float bigButtonX = panelX + (panelWidth - bigButtonWidth) / 2.0f;

    backgroundMusicControlButton = Button(
        sf::Vector2f(bigButtonWidth, 35),
        sf::Vector2f(bigButtonX, startY + paddingTop + rowHeight * 2 + 10.0f),
        soundManager.isBackgroundMusicPlaying() ? "Stop Music" : "Play Music",
        font, 16
    );

    updateMusicStatusText();
}

void GameEngine::updateMusicStatusText()
{
    if (!fontLoaded) return;

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
        // Sortie du mode édition
        state = GameState::IDLE;
        computePath();
    }
    else
    {
        // Entrée en mode édition
        state = GameState::EDIT_MODE;

        // Init éditeur
        mazeEditor = std::make_unique<MazeEditor>(*currentMaze);
        mazeEditor->setTool(currentTool);
    }

    // --- IMPORTANT : ON RECALCULE LA DISPOSITION DES BOUTONS ---
    // Cela va "effacer" les espaces vides et faire remonter les outils
    setupGameUI();
}
