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

private:
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
};
