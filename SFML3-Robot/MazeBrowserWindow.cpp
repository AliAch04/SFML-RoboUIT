#include "MazeBrowserWindow.h"
#include <filesystem>
#include <fstream>
#include <ctime>
#include <iostream>
#include "SimpleJSON.h"

namespace fs = std::filesystem;

const sf::Color MazeBrowserWindow::WINDOW_BG_COLOR = sf::Color(40, 40, 50, 240);
const sf::Color MazeBrowserWindow::TITLE_BAR_COLOR = sf::Color(60, 60, 80);
const sf::Color MazeBrowserWindow::ENTRY_BG_COLOR = sf::Color(50, 50, 60, 200);
const sf::Color MazeBrowserWindow::ENTRY_HOVER_COLOR = sf::Color(70, 70, 90, 220);
const sf::Color MazeBrowserWindow::BUTTON_COLOR = sf::Color(80, 140, 200);
const sf::Color MazeBrowserWindow::BUTTON_HOVER_COLOR = sf::Color(100, 160, 220);

MazeBrowserWindow::MazeBrowserWindow(sf::Font& font, const std::string& mazeDirectory)
    : font(font), mazeDirectory(mazeDirectory) {

    // Créer le dossier s'il n'existe pas
    if (!fs::exists(mazeDirectory)) {
        fs::create_directory(mazeDirectory);
    }

    // Configuration par défaut
    size = sf::Vector2f(500.0f, 400.0f);
    position = sf::Vector2f(150.0f, 100.0f);

    createUI();
}

void MazeBrowserWindow::createUI() {
    // Fond de la fenêtre
    background.setFillColor(WINDOW_BG_COLOR);
    background.setOutlineThickness(2.0f);
    background.setOutlineColor(sf::Color(100, 100, 120));

    // Barre de titre
    titleBar.setFillColor(TITLE_BAR_COLOR);

    // Texte du titre
    titleText.setFont(font);
    titleText.setString("Navigateur de Labyrinthes");
    titleText.setCharacterSize(20);
    titleText.setFillColor(sf::Color::White);

    // Texte "aucun labyrinthe"
    noMazesText.setFont(font);
    noMazesText.setString("Aucun labyrinthe sauvegardé");
    noMazesText.setCharacterSize(16);
    noMazesText.setFillColor(sf::Color(150, 150, 150));

    // Bouton de fermeture
    closeButton.setSize(sf::Vector2f(30.0f, 30.0f));
    closeButton.setFillColor(sf::Color(200, 80, 80));

    closeButtonText.setFont(font);
    closeButtonText.setString("X");
    closeButtonText.setCharacterSize(16);
    closeButtonText.setFillColor(sf::Color::White);

    // Bouton de rafraîchissement
    refreshButton.setSize(sf::Vector2f(100.0f, 30.0f));
    refreshButton.setFillColor(BUTTON_COLOR);

    refreshButtonText.setFont(font);
    refreshButtonText.setString("Rafraîchir");
    refreshButtonText.setCharacterSize(14);
    refreshButtonText.setFillColor(sf::Color::White);
}

void MazeBrowserWindow::setPosition(const sf::Vector2f& position) {
    this->position = position;
}

void MazeBrowserWindow::setSize(const sf::Vector2f& size) {
    this->size = size;
}

void MazeBrowserWindow::update() {
    if (needsRefresh) {
        refreshMazeList();
        needsRefresh = false;
    }

    // Mettre à jour les positions
    background.setPosition(position);
    background.setSize(size);

    titleBar.setPosition(position);
    titleBar.setSize(sf::Vector2f(size.x, 40.0f));

    titleText.setPosition(position.x + 10.0f, position.y + 5.0f);

    closeButton.setPosition(position.x + size.x - 40.0f, position.y + 5.0f);
    closeButtonText.setPosition(
        closeButton.getPosition().x + 10.0f,
        closeButton.getPosition().y + 5.0f
    );

    refreshButton.setPosition(position.x + size.x - 150.0f, position.y + 5.0f);
    refreshButtonText.setPosition(
        refreshButton.getPosition().x + 20.0f,
        refreshButton.getPosition().y + 5.0f
    );

    // Positionner les entrées
    float entryY = position.y + 50.0f;
    float entryHeight = 60.0f;
    float margin = 5.0f;

    for (auto& entry : mazeEntries) {
        entry.background.setPosition(position.x + margin, entryY);
        entry.background.setSize(sf::Vector2f(size.x - 2 * margin, entryHeight - margin));

        entry.nameText.setPosition(
            position.x + 10.0f,
            entryY + 5.0f
        );

        entry.detailsText.setPosition(
            position.x + 10.0f,
            entryY + 25.0f
        );

        entry.loadButton.setPosition(
            position.x + size.x - 110.0f,
            entryY + 15.0f
        );
        entry.loadButton.setSize(sf::Vector2f(100.0f, 30.0f));

        entry.loadButtonText.setPosition(
            entry.loadButton.getPosition().x + 25.0f,
            entry.loadButton.getPosition().y + 5.0f
        );

        entryY += entryHeight;
    }

    // Positionner le texte "aucun labyrinthe"
    noMazesText.setPosition(
        position.x + size.x / 2.0f - noMazesText.getLocalBounds().width / 2.0f,
        position.y + size.y / 2.0f
    );
}

void MazeBrowserWindow::refreshMazeList() {
    mazeEntries.clear();

    try {
        for (const auto& entry : fs::directory_iterator(mazeDirectory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                MazeInfo info;
                info.filename = entry.path().filename().string();
                info.fullPath = entry.path().string();
                info.displayName = entry.path().stem().string();

                // Récupérer la date de modification
                auto ftime = fs::last_write_time(entry.path());
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                char timeStr[100];
                // Version sécurisée de localtime
                struct tm timeinfo;
                localtime_s(&timeinfo, &cftime);
                info.lastModified = timeStr;

                // Lire les métadonnées depuis le fichier JSON
                try {
                    std::ifstream file(info.fullPath);
                    std::string jsonStr((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
                    file.close();

                    SimpleJSON::Object jsonObj;
                    if (SimpleJSON::parse(jsonStr, jsonObj)) {
                        // Utilisez les méthodes statiques de SimpleJSON
                        info.width = SimpleJSON::getInt(jsonObj, "width", 0);
                        info.height = SimpleJSON::getInt(jsonObj, "height", 0);

                        // Optionnel: lire le nom aussi
                        std::string name = SimpleJSON::getString(jsonObj, "name", "");
                        if (!name.empty()) {
                            info.displayName = name;
                        }
                    }
                }
                catch (const std::exception& e) {
                    std::cout << "Erreur de lecture JSON pour " << info.filename
                        << ": " << e.what() << std::endl;
                }
                catch (...) {
                    // Si la lecture échoue, on utilise des valeurs par défaut
                    std::cout << "Lecture des métadonnées échouée pour: " << info.filename << std::endl;
                }

                // Créer l'entrée d'affichage
                MazeEntry mazeEntry;
                mazeEntry.info = info;

                mazeEntry.background.setFillColor(ENTRY_BG_COLOR);
                mazeEntry.background.setOutlineThickness(1.0f);
                mazeEntry.background.setOutlineColor(sf::Color(80, 80, 100));

                mazeEntry.nameText.setFont(font);
                mazeEntry.nameText.setCharacterSize(16);
                mazeEntry.nameText.setFillColor(sf::Color::White);
                mazeEntry.nameText.setString(info.displayName);

                mazeEntry.detailsText.setFont(font);
                mazeEntry.detailsText.setCharacterSize(12);
                mazeEntry.detailsText.setFillColor(sf::Color(180, 180, 200));

                std::string details = "Dimensions: " + std::to_string(info.width) + "x" +
                    std::to_string(info.height) +
                    "  •  Modifié: " + info.lastModified;
                mazeEntry.detailsText.setString(details);

                mazeEntry.loadButton.setFillColor(BUTTON_COLOR);

                mazeEntry.loadButtonText.setFont(font);
                mazeEntry.loadButtonText.setCharacterSize(14);
                mazeEntry.loadButtonText.setFillColor(sf::Color::White);
                mazeEntry.loadButtonText.setString("Charger");

                mazeEntries.push_back(mazeEntry);
            }
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Erreur lors du scan du dossier: " << e.what() << std::endl;
    }

    // Trier par nom
    std::sort(mazeEntries.begin(), mazeEntries.end(),
        [](const MazeEntry& a, const MazeEntry& b) {
            return a.info.displayName < b.info.displayName;
        });
}

void MazeBrowserWindow::draw(sf::RenderWindow& window) {
    if (!visible) return;

    window.draw(background);
    window.draw(titleBar);
    window.draw(titleText);
    window.draw(closeButton);
    window.draw(closeButtonText);
    window.draw(refreshButton);
    window.draw(refreshButtonText);

    if (mazeEntries.empty()) {
        window.draw(noMazesText);
    }
    else {
        for (const auto& entry : mazeEntries) {
            window.draw(entry.background);
            window.draw(entry.nameText);
            window.draw(entry.detailsText);
            window.draw(entry.loadButton);
            window.draw(entry.loadButtonText);
        }
    }
}

void MazeBrowserWindow::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (!visible) return;

    if (event.type == sf::Event::MouseMoved) {
        // Vérifier la survol du bouton de fermeture
        closeButton.setFillColor(closeButton.getGlobalBounds().contains(mousePos)
            ? sf::Color(220, 100, 100) : sf::Color(200, 80, 80));

        // Vérifier la survol du bouton de rafraîchissement
        refreshButton.setFillColor(refreshButton.getGlobalBounds().contains(mousePos)
            ? BUTTON_HOVER_COLOR : BUTTON_COLOR);

        // Vérifier la survol des entrées
        for (auto& entry : mazeEntries) {
            entry.hovered = entry.background.getGlobalBounds().contains(mousePos);
            entry.buttonHovered = entry.loadButton.getGlobalBounds().contains(mousePos);

            entry.background.setFillColor(entry.hovered ? ENTRY_HOVER_COLOR : ENTRY_BG_COLOR);
            entry.loadButton.setFillColor(entry.buttonHovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR);
        }
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        // Bouton de fermeture
        if (closeButton.getGlobalBounds().contains(mousePos)) {
            hide();
            return;
        }

        // Bouton de rafraîchissement
        if (refreshButton.getGlobalBounds().contains(mousePos)) {
            refreshMazeList();
            return;
        }

        // Boutons de chargement des labyrinthes
        for (const auto& entry : mazeEntries) {
            if (entry.loadButton.getGlobalBounds().contains(mousePos)) {
                if (onMazeSelectedCallback) {
                    onMazeSelectedCallback(entry.info);
                }
                hide(); // Fermer la fenêtre après sélection
                return;
            }
        }
    }
}

void MazeBrowserWindow::setOnMazeSelectedCallback(std::function<void(const MazeInfo&)> callback) {
    onMazeSelectedCallback = callback;
}