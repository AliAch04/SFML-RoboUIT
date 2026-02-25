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

    // Create directory if it doesn't exist
    if (!fs::exists(mazeDirectory)) {
        fs::create_directory(mazeDirectory);
    }

    // Default configuration
    size = sf::Vector2f(500.0f, 400.0f);
    position = sf::Vector2f(150.0f, 100.0f);
    showCloseButton = true;

    createUI();
}

void MazeBrowserWindow::createUI() {
    // Window background
    background.setFillColor(WINDOW_BG_COLOR);
    background.setOutlineThickness(2.0f);
    background.setOutlineColor(sf::Color(100, 100, 120));

    // Title bar
    titleBar.setFillColor(TITLE_BAR_COLOR);

    // Title text - ENGLISH
    titleText.setFont(font);
    titleText.setString("Maze Browser");
    titleText.setCharacterSize(20);
    titleText.setFillColor(sf::Color::White);

    // "No mazes" text - ENGLISH
    noMazesText.setFont(font);
    noMazesText.setString("No saved mazes found");
    noMazesText.setCharacterSize(16);
    noMazesText.setFillColor(sf::Color(150, 150, 150));

    // Close button
    closeButton.setSize(sf::Vector2f(30.0f, 30.0f));
    closeButton.setFillColor(sf::Color(200, 80, 80));

    closeButtonText.setFont(font);
    closeButtonText.setString("X");
    closeButtonText.setCharacterSize(16);
    closeButtonText.setFillColor(sf::Color::White);

    // Refresh button - ENGLISH
    refreshButton.setSize(sf::Vector2f(100.0f, 30.0f));
    refreshButton.setFillColor(BUTTON_COLOR);

    refreshButtonText.setFont(font);
    refreshButtonText.setString("Refresh");
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

    // Update positions and sizes
    background.setPosition(position);
    background.setSize(size);

    // Title bar
    titleBar.setPosition(position);
    titleBar.setSize(sf::Vector2f(size.x, 40.0f));

    // Title text
    titleText.setPosition(position.x + 10.0f, position.y + 5.0f);

    // Control buttons
    closeButton.setPosition(position.x + size.x - 40.0f, position.y + 5.0f);
    closeButtonText.setPosition(
        closeButton.getPosition().x + 10.0f,
        closeButton.getPosition().y + 5.0f
    );

    refreshButton.setPosition(position.x + size.x - 150.0f, position.y + 5.0f);
    refreshButtonText.setPosition(
        refreshButton.getPosition().x + 25.0f,
        refreshButton.getPosition().y + 5.0f
    );

    // Position maze entries
    float startY = position.y + 50.0f;
    float entryHeight = 70.0f; // Increased height for better spacing
    float margin = 5.0f;

    for (size_t i = 0; i < mazeEntries.size(); i++) {
        auto& entry = mazeEntries[i];
        float entryY = startY + (i * entryHeight);

        // Check if entry is visible
        if (entryY < position.y + size.y && entryY + entryHeight > position.y + 50.0f) {
            entry.background.setPosition(position.x + margin, entryY);
            entry.background.setSize(sf::Vector2f(size.x - 2 * margin, entryHeight - margin));

            entry.nameText.setPosition(
                position.x + 15.0f,
                entryY + 8.0f
            );

            entry.detailsText.setPosition(
                position.x + 15.0f,
                entryY + 32.0f
            );

            entry.loadButton.setPosition(
                position.x + size.x - 110.0f,
                entryY + 18.0f
            );
            entry.loadButton.setSize(sf::Vector2f(90.0f, 30.0f));

            // Center text in button
            sf::FloatRect buttonBounds = entry.loadButton.getLocalBounds();
            sf::FloatRect textBounds = entry.loadButtonText.getLocalBounds();
            entry.loadButtonText.setPosition(
                entry.loadButton.getPosition().x + (buttonBounds.width - textBounds.width) / 2.0f,
                entry.loadButton.getPosition().y + 6.0f
            );
        }
    }

    // Position "no mazes" text at center
    sf::FloatRect noMazesBounds = noMazesText.getLocalBounds();
    noMazesText.setPosition(
        position.x + (size.x - noMazesBounds.width) / 2.0f,
        position.y + (size.y - noMazesBounds.height) / 2.0f
    );
}

void MazeBrowserWindow::refreshMazeList() {
    mazeEntries.clear();

    std::cout << "\n=== MAZE BROWSER: Refreshing list... ===" << std::endl;

    if (!fs::exists(mazeDirectory)) {
        fs::create_directory(mazeDirectory);
    }

    for (const auto& entry : fs::directory_iterator(mazeDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            MazeInfo info;
            info.filename = entry.path().filename().string();
            info.fullPath = entry.path().string();
            info.displayName = entry.path().stem().string();

            // Attempt to read JSON for real dimensions
            try {
                std::ifstream file(info.fullPath);
                json j;
                file >> j;
                info.width = j.value("width", 10);
                info.height = j.value("height", 10);
                if (j.contains("name")) info.displayName = j["name"];

                // Get last modified time
                auto ftime = fs::last_write_time(entry.path());
                auto time_t = std::chrono::system_clock::to_time_t(
                    std::chrono::clock_cast<std::chrono::system_clock>(ftime));

                char timeStr[100];
#ifdef _WIN32
                struct tm timeinfo;
                localtime_s(&timeinfo, &time_t);
                std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d", &timeinfo);
#else
                struct tm timeinfo;
                localtime_r(&time_t, &timeinfo);
                std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d", &timeinfo);
#endif
                info.lastModified = timeStr;
            }
            catch (...) {
                info.width = 10;
                info.height = 10;
                info.lastModified = "Unknown";
            }

            MazeEntry mazeEntry;
            mazeEntry.info = info;

            // Setup visual styles
            mazeEntry.background.setFillColor(ENTRY_BG_COLOR);

            mazeEntry.nameText.setFont(font);
            mazeEntry.nameText.setString(info.displayName);
            mazeEntry.nameText.setFillColor(sf::Color::White);
            mazeEntry.nameText.setCharacterSize(16);

            mazeEntry.detailsText.setFont(font);
            mazeEntry.detailsText.setString(
                std::to_string(info.width) + "x" + std::to_string(info.height) +
                " | " + info.lastModified
            );
            mazeEntry.detailsText.setFillColor(sf::Color(180, 180, 200));
            mazeEntry.detailsText.setCharacterSize(12);

            mazeEntry.loadButton.setFillColor(BUTTON_COLOR);
            mazeEntry.loadButtonText.setFont(font);
            mazeEntry.loadButtonText.setString("Load"); // ENGLISH
            mazeEntry.loadButtonText.setFillColor(sf::Color::White);
            mazeEntry.loadButtonText.setCharacterSize(14);

            mazeEntries.push_back(mazeEntry);
        }
    }

    if (mazeEntries.empty()) {
        noMazesText.setString("No saved mazes found"); // ENGLISH
    }
    else {
        noMazesText.setString("");
    }

    std::cout << "Found " << mazeEntries.size() << " maze(s)" << std::endl;
}

void MazeBrowserWindow::draw(sf::RenderWindow& window) {
    if (!visible) return;

    // Draw shadow
    sf::RectangleShape shadow(background.getSize());
    shadow.setPosition(background.getPosition() + sf::Vector2f(5.0f, 5.0f));
    shadow.setFillColor(sf::Color(0, 0, 0, 100));
    window.draw(shadow);

    window.draw(background);
    window.draw(titleBar);
    window.draw(titleText);

    // Conditionally draw close button
    if (showCloseButton) {
        window.draw(closeButton);
        window.draw(closeButtonText);
    }

    window.draw(refreshButton);
    window.draw(refreshButtonText);

    if (mazeEntries.empty()) {
        window.draw(noMazesText);
    }
    else {
        // Draw only visible entries
        float visibleStartY = position.y + 50.0f;
        float visibleEndY = position.y + size.y;

        for (const auto& entry : mazeEntries) {
            float entryY = entry.background.getPosition().y;
            if (entryY >= visibleStartY - 70.0f && entryY <= visibleEndY) {
                window.draw(entry.background);
                window.draw(entry.nameText);
                window.draw(entry.detailsText);
                window.draw(entry.loadButton);
                window.draw(entry.loadButtonText);
            }
        }

        // Draw scrollbar if needed
        float totalContentHeight = mazeEntries.size() * 70.0f;
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
        // Check close button hover
        if (showCloseButton) {
            closeButton.setFillColor(closeButton.getGlobalBounds().contains(mousePos)
                ? sf::Color(220, 100, 100) : sf::Color(200, 80, 80));
        }

        // Check refresh button hover
        refreshButton.setFillColor(refreshButton.getGlobalBounds().contains(mousePos)
            ? BUTTON_HOVER_COLOR : BUTTON_COLOR);

        // Check entry hover - ONLY if mouse is within the main window area
        if (background.getGlobalBounds().contains(mousePos)) {
            for (auto& entry : mazeEntries) {
                entry.hovered = entry.background.getGlobalBounds().contains(mousePos);
                entry.buttonHovered = entry.loadButton.getGlobalBounds().contains(mousePos);

                entry.background.setFillColor(entry.hovered ? ENTRY_HOVER_COLOR : ENTRY_BG_COLOR);
                entry.loadButton.setFillColor(entry.buttonHovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR);
            }
        }
        else {
            // Reset hover states when mouse is outside the window
            for (auto& entry : mazeEntries) {
                entry.hovered = false;
                entry.buttonHovered = false;
                entry.background.setFillColor(ENTRY_BG_COLOR);
                entry.loadButton.setFillColor(BUTTON_COLOR);
            }
        }
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        // Close button - only if visible and clicked
        if (showCloseButton && closeButton.getGlobalBounds().contains(mousePos)) {
            hide();
            return;
        }

        // Refresh button
        if (refreshButton.getGlobalBounds().contains(mousePos)) {
            refreshMazeList();
            return;
        }

        // Load buttons - ONLY process if click is within the window background
        if (background.getGlobalBounds().contains(mousePos)) {
            for (const auto& entry : mazeEntries) {
                if (entry.loadButton.getGlobalBounds().contains(mousePos)) {
                    if (onMazeSelectedCallback) {
                        onMazeSelectedCallback(entry.info);
                    }
                    hide();
                    return;
                }
            }
        }
    }
}

void MazeBrowserWindow::setOnMazeSelectedCallback(std::function<void(const MazeInfo&)> callback) {
    onMazeSelectedCallback = callback;
}