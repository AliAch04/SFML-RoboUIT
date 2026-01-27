#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>

struct MazeInfo {
    std::string filename;
    std::string fullPath;
    std::string displayName;
    int width = 0;
    int height = 0;
    std::string lastModified;
};

class MazeBrowserWindow {
public:
    MazeBrowserWindow(sf::Font& font, const std::string& mazeDirectory = "mazes");

    void setPosition(const sf::Vector2f& position);
    void setSize(const sf::Vector2f& size);

    void update();
    void draw(sf::RenderWindow& window);

    bool isVisible() const { return visible; }
    void show() { visible = true; needsRefresh = true; }
    void hide() { visible = false; }

    void handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);

    void setOnMazeSelectedCallback(std::function<void(const MazeInfo&)> callback);

private:
    void loadMazeList();
    void refreshMazeList();
    void createUI();

    sf::Font& font;
    std::string mazeDirectory;

    bool visible = false;
    bool needsRefresh = false;

    sf::Vector2f position;
    sf::Vector2f size;

    sf::RectangleShape background;
    sf::RectangleShape titleBar;
    sf::Text titleText;
    sf::Text noMazesText;

    struct MazeEntry {
        MazeInfo info;
        sf::RectangleShape background;
        sf::Text nameText;
        sf::Text detailsText;
        sf::RectangleShape loadButton;
        sf::Text loadButtonText;
        bool hovered = false;
        bool buttonHovered = false;
    };

    std::vector<MazeEntry> mazeEntries;

    sf::RectangleShape closeButton;
    sf::Text closeButtonText;

    sf::RectangleShape refreshButton;
    sf::Text refreshButtonText;

    std::function<void(const MazeInfo&)> onMazeSelectedCallback;

    static const sf::Color WINDOW_BG_COLOR;
    static const sf::Color TITLE_BAR_COLOR;
    static const sf::Color ENTRY_BG_COLOR;
    static const sf::Color ENTRY_HOVER_COLOR;
    static const sf::Color BUTTON_COLOR;
    static const sf::Color BUTTON_HOVER_COLOR;
};