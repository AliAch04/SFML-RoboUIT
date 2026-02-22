#include "ControlPanelWidget.h"
#include <iostream>
#include <cmath>

ControlPanelWidget::ControlPanelWidget() {
    panel.setSize(sf::Vector2f(200.f, 600.f));
    panel.setPosition(600.f, 0.f);
    panel.setFillColor(sf::Color(50, 50, 50));
}

void ControlPanelWidget::init(sf::Font& font) {
    fontPtr = &font;

    title.setFont(font);
    title.setCharacterSize(24);
    title.setFillColor(sf::Color::Cyan);
    title.setStyle(sf::Text::Bold);

    // Initialize rows (will be positioned later)
    const float startY = 60.f;
    const float rowH = 120.f;

    const sf::Vector2f btnSize(80.f, 25.f); // Slightly smaller buttons

    for (int i = 0; i < 4; ++i) {
        float y = startY + i * rowH;

        rows[i].label.setFont(font);
        rows[i].label.setCharacterSize(16);
        rows[i].label.setFillColor(sf::Color::White);
        rows[i].label.setStyle(sf::Text::Bold);

        if (i == 0) rows[i].label.setString("Robot");
        if (i == 1) rows[i].label.setString("Wall");
        if (i == 2) rows[i].label.setString("Floor");
        if (i == 3) rows[i].label.setString("Obstacle");

        rows[i].uploadBtn = std::make_unique<Button>(btnSize, sf::Vector2f(0, 0), "Upload", font, 12);
        rows[i].resetBtn = std::make_unique<Button>(btnSize, sf::Vector2f(0, 0), "Reset", font, 12);

        // Thumbnail placeholder
        rows[i].thumbPos = sf::Vector2f(0, 0);
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

    if (gridMode) {
        // Draw in grid layout
        drawGridLayout(window);
    }
    else {
        // Original vertical layout
        drawVerticalLayout(window);
    }
}

void ControlPanelWidget::drawVerticalLayout(sf::RenderWindow& window) {
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
                thumb.setScale(0.5f, 0.5f); // Smaller thumbnails
                window.draw(thumb);
            }
            else {
                sf::RectangleShape box(sf::Vector2f(64.f, 64.f));
                box.setPosition(rows[i].thumbPos);
                box.setFillColor(sf::Color(40, 40, 50));
                box.setOutlineThickness(2.f);
                box.setOutlineColor(sf::Color(80, 80, 100));
                window.draw(box);

                sf::Text err;
                err.setFont(*fontPtr);
                err.setCharacterSize(10);
                err.setFillColor(sf::Color(255, 150, 150));
                err.setString("Not loaded");
                err.setPosition(rows[i].thumbPos.x + 5.f, rows[i].thumbPos.y + 70.f);
                window.draw(err);
            }
        }
    }
}

void ControlPanelWidget::drawGridLayout(sf::RenderWindow& window) {
    float cellWidth = (gridSize.x - 60.f) / gridColumns;
    float cellHeight = 180.f; // Increased from 160 to give more space at bottom

    int rowsCount = std::ceil(4.0f / gridColumns);
    float totalGridHeight = rowsCount * cellHeight;
    float startY = gridPosition.y + (gridSize.y - totalGridHeight) / 2.0f;

    for (int i = 0; i < 4; ++i) {
        int col = i % gridColumns;
        int row = i / gridColumns;

        float x = gridPosition.x + 30.f + col * cellWidth;
        float y = startY + row * cellHeight;

        // Draw cell background (optional)
        sf::RectangleShape cellBg(sf::Vector2f(cellWidth - 20.f, cellHeight - 20.f));
        cellBg.setPosition(x + 10.f, y + 10.f);
        cellBg.setFillColor(sf::Color(40, 40, 50, 100));
        cellBg.setOutlineColor(sf::Color(80, 80, 100, 150));
        cellBg.setOutlineThickness(1.f);
        window.draw(cellBg);

        // Label centered at top of cell
        sf::FloatRect labelBounds = rows[i].label.getLocalBounds();
        rows[i].label.setOrigin(labelBounds.width / 2.0f, labelBounds.height / 2.0f);
        rows[i].label.setPosition(x + cellWidth / 2.0f - 10.f, y + 30.f);
        window.draw(rows[i].label);

        // Thumbnail perfectly centered in the middle of the cell
        float thumbSize = 64.f;
        float thumbX = x + (cellWidth - thumbSize) / 2.0f - 10.f;
        float thumbY = y + 60.f; // Positioned below label

        if (texMgr) {
            auto id = indexToId(i);
            const auto& e = texMgr->get(id);

            if (e.loaded) {
                sf::Sprite thumb = e.thumbnailSprite;

                // Calculate scale to fit thumbnail in 64x64 box while preserving aspect ratio
                sf::Vector2u texSize = e.thumbnailSprite.getTexture()->getSize();
                float scaleX = thumbSize / texSize.x;
                float scaleY = thumbSize / texSize.y;
                float scale = std::min(scaleX, scaleY); // Use the smaller scale to fit entirely

                // Center the thumbnail within the 64x64 box
                float scaledWidth = texSize.x * scale;
                float scaledHeight = texSize.y * scale;
                float offsetX = (thumbSize - scaledWidth) / 2.0f;
                float offsetY = (thumbSize - scaledHeight) / 2.0f;

                thumb.setScale(scale, scale);
                thumb.setPosition(thumbX + offsetX, thumbY + offsetY);
                window.draw(thumb);

                // Draw a subtle border around the thumbnail area
                sf::RectangleShape thumbBorder(sf::Vector2f(thumbSize, thumbSize));
                thumbBorder.setPosition(thumbX, thumbY);
                thumbBorder.setFillColor(sf::Color::Transparent);
                thumbBorder.setOutlineColor(sf::Color(100, 150, 255, 100));
                thumbBorder.setOutlineThickness(1.5f);
                window.draw(thumbBorder);
            }
            else {
                sf::RectangleShape box(sf::Vector2f(thumbSize, thumbSize));
                box.setPosition(thumbX, thumbY);
                box.setFillColor(sf::Color(40, 40, 50));
                box.setOutlineThickness(2.f);
                box.setOutlineColor(sf::Color(120, 120, 140));
                window.draw(box);

                sf::Text err;
                err.setFont(*fontPtr);
                err.setCharacterSize(9);
                err.setFillColor(sf::Color(200, 200, 200));
                err.setString("No texture");

                sf::FloatRect errBounds = err.getLocalBounds();
                err.setOrigin(errBounds.width / 2.0f, errBounds.height / 2.0f);
                err.setPosition(thumbX + thumbSize / 2.0f, thumbY + thumbSize / 2.0f);
                window.draw(err);
            }
        }

        // Buttons centered below thumbnail with proper bottom margin
        float btnY = thumbY + 70.f; // Positioned below thumbnail

        // Create a container for buttons to center them
        float btnWidth = 70.f; // Slightly smaller buttons
        float btnHeight = 22.f; // Slightly smaller height
        float btnSpacing = 15.f; // Space between buttons
        float totalBtnWidth = btnWidth + btnSpacing + btnWidth;

        // Recalculate button start X to center the button group
        float btnStartX = x + (cellWidth - totalBtnWidth) / 2.0f - 10.f;

        if (rows[i].uploadBtn) {
            // Resize button (if Button class has setSize method)
            // rows[i].uploadBtn->setSize(sf::Vector2f(btnWidth, btnHeight));
            rows[i].uploadBtn->setPosition(sf::Vector2f(btnStartX, btnY));
            rows[i].uploadBtn->draw(window);
        }
        if (rows[i].resetBtn) {
            // rows[i].resetBtn->setSize(sf::Vector2f(btnWidth, btnHeight));
            rows[i].resetBtn->setPosition(sf::Vector2f(btnStartX + btnWidth + btnSpacing, btnY));
            rows[i].resetBtn->draw(window);
        }

        // Add bottom margin text (optional - shows the file path)
        if (texMgr) {
            auto id = indexToId(i);
            const auto& e = texMgr->get(id);

            if (e.loaded && !e.currentPath.empty()) {
                // Extract just the filename from the path
                std::string filename = e.currentPath;
                size_t pos = filename.find_last_of("/\\");
                if (pos != std::string::npos) {
                    filename = filename.substr(pos + 1);
                }

                // Truncate if too long
                if (filename.length() > 15) {
                    filename = filename.substr(0, 12) + "...";
                }

                sf::Text pathText;
                pathText.setFont(*fontPtr);
                pathText.setCharacterSize(8);
                pathText.setFillColor(sf::Color(150, 150, 150));
                pathText.setString(filename);

                sf::FloatRect pathBounds = pathText.getLocalBounds();
                pathText.setOrigin(pathBounds.width / 2.0f, pathBounds.height / 2.0f);
                pathText.setPosition(x + cellWidth / 2.0f - 10.f, btnY + 25.f);
                window.draw(pathText);
            }
        }
    }
}

void ControlPanelWidget::setPosition(const sf::Vector2f& pos)
{
    panel.setPosition(pos);

    if (!fontPtr) return;

    // Move title relative to panel
    title.setPosition(pos.x + 30.f, pos.y + 15.f);

    const float startY = pos.y + 60.f;
    const float rowH = 120.f;

    const sf::Vector2f btnSize(80.f, 25.f);
    const float btnX = pos.x + 30.f;
    const float thumbX = pos.x + 140.f;

    for (int i = 0; i < 4; ++i) {
        float y = startY + i * rowH;

        rows[i].label.setPosition(pos.x + 30.f, y);

        if (rows[i].uploadBtn) rows[i].uploadBtn->setPosition(sf::Vector2f(btnX, y + 55.f));
        if (rows[i].resetBtn)  rows[i].resetBtn->setPosition(sf::Vector2f(btnX, y + 85.f));

        rows[i].thumbPos = sf::Vector2f(thumbX, y + 35.f);
    }
}

void ControlPanelWidget::setSize(const sf::Vector2f& size)
{
    panel.setSize(size);
}