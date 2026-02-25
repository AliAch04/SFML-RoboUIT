#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <array>
#include <memory>
#include "Button.h"
#include "TextureManager.h"

class ControlPanelWidget {
public:
    ControlPanelWidget();

    void init(sf::Font& font);
    void setTextureManager(TextureManager* tm);

    void handleHover(const sf::Vector2f& mousePos);

    bool handleClick(const sf::Vector2f& mousePos,
        const std::function<void(TextureManager::Id)>& onUpload,
        const std::function<void(TextureManager::Id)>& onReset);

    void draw(sf::RenderWindow& window);

    // grid layout
    void setGridMode(bool enabled, int columns = 2) {
        gridMode = enabled;
        gridColumns = columns;
    }

    void setGridPosition(const sf::Vector2f& pos, const sf::Vector2f& size) {
        gridPosition = pos;
        gridSize = size;
    }

public:
    void setPosition(const sf::Vector2f& pos);
    void setSize(const sf::Vector2f& size);

private:
    // Private drawing methods
    void drawVerticalLayout(sf::RenderWindow& window);
    void drawGridLayout(sf::RenderWindow& window);

    sf::RectangleShape panel;

    sf::Font* fontPtr = nullptr;
    TextureManager* texMgr = nullptr;

    sf::Text title;

    struct RowUI {
        sf::Text label;
        std::unique_ptr<Button> uploadBtn;
        std::unique_ptr<Button> resetBtn;
        sf::Vector2f thumbPos;
    };

    std::array<RowUI, 4> rows;

private:
    static TextureManager::Id indexToId(int idx);
    bool isInPanel(const sf::Vector2f& p) const;

    bool gridMode = false;
    int gridColumns = 2;
    sf::Vector2f gridPosition;
    sf::Vector2f gridSize;

};