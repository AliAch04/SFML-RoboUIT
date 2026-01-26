#define _CRT_SECURE_NO_WARNINGS
#include <ctime>
#include "MazeBrowserWindow.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include "includes/json.hpp" 

using json = nlohmann::json;
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

    // Mettre à jour les positions et tailles
    background.setPosition(position);
    background.setSize(size);

    // Barre de titre
    titleBar.setPosition(position);
    titleBar.setSize(sf::Vector2f(size.x, 40.0f));

    // Texte du titre
    titleText.setPosition(position.x + 10.0f, position.y + 5.0f);

    // Boutons de contrôle
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

    // Positionner les entrées de labyrinthes
    float startY = position.y + 50.0f;
    float entryHeight = 60.0f;
    float margin = 5.0f;
    float scrollableHeight = size.y - 60.0f; // Hauteur disponible pour le défilement

    // Calculer la position de défilement si nécessaire
    float totalContentHeight = mazeEntries.size() * entryHeight;
    float visibleStartY = startY;

    // Pour l'instant, pas de défilement, on affiche tout ce qui rentre
    for (size_t i = 0; i < mazeEntries.size(); i++) {
        auto& entry = mazeEntries[i];
        float entryY = startY + (i * entryHeight);

        // Vérifier si l'entrée est visible
        if (entryY < position.y + size.y && entryY + entryHeight > position.y + 50.0f) {
            entry.background.setPosition(position.x + margin, entryY);
            entry.background.setSize(sf::Vector2f(size.x - 2 * margin, entryHeight - margin));

            entry.nameText.setPosition(
                position.x + 15.0f,
                entryY + 8.0f
            );

            entry.detailsText.setPosition(
                position.x + 15.0f,
                entryY + 30.0f
            );

            entry.loadButton.setPosition(
                position.x + size.x - 120.0f,
                entryY + 15.0f
            );
            entry.loadButton.setSize(sf::Vector2f(100.0f, 30.0f));

            // Centrer le texte dans le bouton
            sf::FloatRect buttonBounds = entry.loadButton.getLocalBounds();
            sf::FloatRect textBounds = entry.loadButtonText.getLocalBounds();
            entry.loadButtonText.setPosition(
                entry.loadButton.getPosition().x + (buttonBounds.width - textBounds.width) / 2.0f,
                entry.loadButton.getPosition().y + 5.0f
            );
        }
    }

    // Positionner le texte "aucun labyrinthe" au centre
    sf::FloatRect noMazesBounds = noMazesText.getLocalBounds();
    noMazesText.setPosition(
        position.x + (size.x - noMazesBounds.width) / 2.0f,
        position.y + (size.y - noMazesBounds.height) / 2.0f
    );
}


void MazeBrowserWindow::refreshMazeList() {
    mazeEntries.clear();

    std::cout << "\n=== MAZE BROWSER: Recherche de labyrinthes ===" << std::endl;
    std::cout << "Dossier: " << mazeDirectory << std::endl;

    try {
        // Vérifier si le dossier existe, sinon le créer
        if (!fs::exists(mazeDirectory)) {
            std::cout << "Le dossier n'existe pas, création..." << std::endl;
            if (!fs::create_directory(mazeDirectory)) {
                std::cerr << "ERREUR: Impossible de créer le dossier " << mazeDirectory << std::endl;
                return;
            }
            std::cout << "Dossier créé avec succès." << std::endl;
        }

        // Compter les fichiers
        int totalFiles = 0;
        int validMazes = 0;

        for (const auto& entry : fs::directory_iterator(mazeDirectory)) {
            totalFiles++;

            // Vérifier si c'est un fichier .json
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                MazeInfo info;
                info.filename = entry.path().filename().string();
                info.fullPath = entry.path().string();
                info.displayName = entry.path().stem().string();

                // Récupérer la date de modification
                try {
                    auto ftime = fs::last_write_time(entry.path());
                    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                    );
                    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                    char timeStr[100];

                    #ifdef _WIN32
                    struct tm timeinfo;
                    localtime_s(&timeinfo, &cftime);
                    std::strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M", &timeinfo);
                    #else
                    struct tm timeinfo;
                    localtime_r(&cftime, &timeinfo);
                    std::strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M", &timeinfo);
                    #endif

                    info.lastModified = timeStr;
                }
                catch (...) {
                    info.lastModified = "Date inconnue";
                }

                // Lire les métadonnées avec nlohmann/json
                bool metadataLoaded = false;

                try {
                    std::ifstream file(info.fullPath);
                    if (file.is_open()) {
                        // Essayer de parser avec nlohmann/json
                        try {
                            json j;
                            file >> j;

                            // Lire les dimensions (avec valeurs par défaut)
                            info.width = j.value("width", 0);
                            info.height = j.value("height", 0);

                            // Lire le nom si disponible
                            std::string name = j.value("name", "");
                            if (!name.empty()) {
                                info.displayName = name;
                            }

                            metadataLoaded = true;
                            validMazes++;

                            std::cout << "Success " << info.displayName
                                << " (" << info.width << "x" << info.height << ")" << std::endl;

                        }
                        catch (const json::exception& e) {
                            std::cout << "  Erreur JSON pour " << info.filename
                                << ": " << e.what() << std::endl;

                            // Fallback: essayer de lire comme ancien format texte
                            file.clear();
                            file.seekg(0);
                            std::string firstLine;
                            if (std::getline(file, firstLine)) {
                                std::istringstream lineStream(firstLine);
                                if (lineStream >> info.width >> info.height) {
                                    metadataLoaded = true;
                                    validMazes++;
                                    std::cout << "Success " << info.displayName
                                        << " [format texte] ("
                                        << info.width << "x" << info.height << ")" << std::endl;
                                }
                            }
                        }
                        file.close();
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Erreur de lecture pour " << info.filename
                        << ": " << e.what() << std::endl;
                }

                // Créer l'entrée seulement si les métadonnées sont valides
                if (metadataLoaded && info.width > 0 && info.height > 0) {
                    MazeEntry mazeEntry;
                    mazeEntry.info = info;

                    // Style pour l'entrée
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
                        "  - Modifié: " + info.lastModified;
                    mazeEntry.detailsText.setString(details);

                    mazeEntry.loadButton.setFillColor(BUTTON_COLOR);
                    mazeEntry.loadButton.setOutlineThickness(1.0f);
                    mazeEntry.loadButton.setOutlineColor(sf::Color(100, 160, 220));

                    mazeEntry.loadButtonText.setFont(font);
                    mazeEntry.loadButtonText.setCharacterSize(14);
                    mazeEntry.loadButtonText.setFillColor(sf::Color::White);
                    mazeEntry.loadButtonText.setString("Charger");

                    mazeEntries.push_back(mazeEntry);
                }
                else {
                    std::cout << "Failed " << info.filename << " (métadonnées invalides)" << std::endl;
                }
            }
        }

        std::cout << "=== RÉSUMÉ ===" << std::endl;
        std::cout << "Fichiers totaux: " << totalFiles << std::endl;
        std::cout << "Labyrinthes valides: " << validMazes << std::endl;
        std::cout << "Entrées affichées: " << mazeEntries.size() << std::endl;

        // Si pas de labyrinthes, afficher un message
        if (mazeEntries.empty()) {
            noMazesText.setString("Aucun labyrinthe sauvegardé\n\nSauvegardez d'abord un labyrinthe en cliquant sur 'Sauver'");
            noMazesText.setCharacterSize(14);
        }
        else {
            noMazesText.setString("");
        }

    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Erreur d'accès au dossier: " << e.what() << std::endl;
        noMazesText.setString("Erreur d'accès au dossier\n" + std::string(e.what()));
        noMazesText.setFillColor(sf::Color::Red);
    }

    // Trier les entrées par nom
    std::sort(mazeEntries.begin(), mazeEntries.end(),
        [](const MazeEntry& a, const MazeEntry& b) {
            return a.info.displayName < b.info.displayName;
        });

    std::cout << "=== RECHERCHE TERMINÉE ===\n" << std::endl;
}


void MazeBrowserWindow::draw(sf::RenderWindow& window) {
    if (!visible) return;

    // Dessiner l'arrière-plan avec une ombre
    sf::RectangleShape shadow(background.getSize());
    shadow.setPosition(background.getPosition() + sf::Vector2f(5.0f, 5.0f));
    shadow.setFillColor(sf::Color(0, 0, 0, 100));
    window.draw(shadow);

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
        // Dessiner seulement les entrées visibles
        float visibleStartY = position.y + 50.0f;
        float visibleEndY = position.y + size.y;

        for (const auto& entry : mazeEntries) {
            float entryY = entry.background.getPosition().y;
            if (entryY >= visibleStartY - 60.0f && entryY <= visibleEndY) {
                window.draw(entry.background);
                window.draw(entry.nameText);
                window.draw(entry.detailsText);
                window.draw(entry.loadButton);
                window.draw(entry.loadButtonText);
            }
        }

        // Dessiner une barre de défilement si nécessaire
        float totalContentHeight = mazeEntries.size() * 60.0f;
        if (totalContentHeight > size.y - 60.0f) {
            sf::RectangleShape scrollbar(sf::Vector2f(8.0f, size.y - 60.0f));
            scrollbar.setPosition(position.x + size.x - 12.0f, position.y + 50.0f);
            scrollbar.setFillColor(sf::Color(100, 100, 120, 150));
            window.draw(scrollbar);
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