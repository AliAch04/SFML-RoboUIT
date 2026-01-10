#include "ControlPanelWidget.h"
#include <iostream>

ControlPanelWidget::ControlPanelWidget() {
    panel.setSize(sf::Vector2f(200.f, 600.f));
    panel.setPosition(600.f, 0.f);
    panel.setFillColor(sf::Color(50, 50, 50));
}

void ControlPanelWidget::init(sf::Font& font) {
    fontPtr = &font;

    title.setFont(font);
    title.setString("Textures");
    title.setCharacterSize(20);
    title.setFillColor(sf::Color::White);
    title.setPosition(610.f, 15.f);

    const float startY = 60.f;
    const float rowH = 120.f;

    const sf::Vector2f btnSize(90.f, 28.f);
    const float btnX = 610.f;
    const float thumbX = 720.f;

    for (int i = 0; i < 4; ++i) {
        float y = startY + i * rowH;

        rows[i].label.setFont(font);
        rows[i].label.setCharacterSize(16);
        rows[i].label.setFillColor(sf::Color::White);
        rows[i].label.setPosition(610.f, y);

        if (i == 0) rows[i].label.setString("Robot");
        if (i == 1) rows[i].label.setString("Wall");
        if (i == 2) rows[i].label.setString("Floor");
        if (i == 3) rows[i].label.setString("Obstacle");

        rows[i].uploadBtn = std::make_unique<Button>(btnSize, sf::Vector2f(btnX, y + 55.f), "Upload", font, 14);
        rows[i].resetBtn = std::make_unique<Button>(btnSize, sf::Vector2f(btnX, y + 85.f), "Reset", font, 14);

        rows[i].thumbPos = sf::Vector2f(thumbX, y + 40.f);
    }
}

void ControlPanelWidget::setTextureManager(TextureManager* tm) {
    texMgr = tm;
}

bool ControlPanelWidget::isInPanel(const sf::Vector2f& p) const {
    return panel.getGlobalBounds().contains(p);
}

TextureManager::Id ControlPanelWidget::indexToId(int idx) {
    return (TextureManager::Id)idx; // Robot=0 Wall=1 Floor=2 Obstacle=3
}

void ControlPanelWidget::handleHover(const sf::Vector2f& mousePos) {
    if (!fontPtr) return;

    if (!isInPanel(mousePos)) {
        for (auto& r : rows) {
            if (r.uploadBtn) r.uploadBtn->setHovered(false);
            if (r.resetBtn)  r.resetBtn->setHovered(false);
        }
        return;
    }

    for (auto& r : rows) {
        if (r.uploadBtn) r.uploadBtn->setHovered(r.uploadBtn->contains(mousePos));
        if (r.resetBtn)  r.resetBtn->setHovered(r.resetBtn->contains(mousePos));
    }
}

bool ControlPanelWidget::handleClick(const sf::Vector2f& mousePos,
    const std::function<void(TextureManager::Id)>& onUpload,
    const std::function<void(TextureManager::Id)>& onReset)
{
    if (!fontPtr) return false;
    if (!isInPanel(mousePos)) return false;

    for (int i = 0; i < 4; ++i) {
        if (rows[i].uploadBtn && rows[i].uploadBtn->contains(mousePos)) {
            onUpload(indexToId(i));
            return true;
        }
        if (rows[i].resetBtn && rows[i].resetBtn->contains(mousePos)) {
            onReset(indexToId(i));
            return true;
        }
    }
    return false;
}

void ControlPanelWidget::draw(sf::RenderWindow& window) {
    window.draw(panel);
    if (!fontPtr) return;

    window.draw(title);

    for (int i = 0; i < 4; ++i) {
        window.draw(rows[i].label);

        if (rows[i].uploadBtn) rows[i].uploadBtn->draw(window);
        if (rows[i].resetBtn)  rows[i].resetBtn->draw(window);

        if (texMgr) {
            auto id = indexToId(i);
            const auto& e = texMgr->get(id);

            if (e.loaded) {
                sf::Sprite thumb = e.thumbnailSprite;
                thumb.setPosition(rows[i].thumbPos);
                window.draw(thumb);
            }
            else {
                sf::RectangleShape box(sf::Vector2f(72.f, 72.f));
                box.setPosition(rows[i].thumbPos);
                box.setFillColor(sf::Color(30, 30, 30));
                box.setOutlineThickness(1.f);
                box.setOutlineColor(sf::Color(120, 120, 120));
                window.draw(box);

                sf::Text err;
                err.setFont(*fontPtr);
                err.setCharacterSize(10);
                err.setFillColor(sf::Color(255, 120, 120));
                err.setString(e.lastError.empty() ? "Not loaded" : e.lastError);
                err.setPosition(rows[i].thumbPos.x, rows[i].thumbPos.y + 75.f);
                window.draw(err);
            }
        }
    }
}

