#include "EditorToolbar.h"
#include <iostream>

EditorToolbar::EditorToolbar() {
    // Initialize selection highlight
    selectionHighlight.setFillColor(sf::Color(100, 150, 255, 80)); // Semi-transparent blue
    selectionHighlight.setOutlineColor(sf::Color::Cyan);
    selectionHighlight.setOutlineThickness(2.0f);

    // Initialize glow effect
    glowEffect.setFillColor(sf::Color(100, 150, 255, 40));
    glowEffect.setRadius(30.0f);
    glowEffect.setOrigin(30.0f, 30.0f);
}

void EditorToolbar::init(sf::Font& font, float startX, float startY, float panelWidth) {
    this->position = sf::Vector2f(startX, startY);
    this->panelWidth = panelWidth;
    this->currentFont = &font;

    tools.clear();

    // Calculate centered positions for 5 tools
    float buttonSize = 50.0f;
    float gap = 15.0f;
    int numButtons = 5; // WALL, ERASE, START, END, SPECIAL
    float totalWidth = numButtons * buttonSize + (numButtons - 1) * gap;
    float startButtonX = startX + (panelWidth - totalWidth) / 2.0f;

    // WALL Button
    auto wallBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX, startY),
        "W", font, 20
    );
    tools.push_back(std::move(wallBtn));

    // ERASE Button
    auto eraseBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + buttonSize + gap, startY),
        "E", font, 20
    );
    tools.push_back(std::move(eraseBtn));

    // START Button
    auto startBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + (buttonSize + gap) * 2, startY),
        "S", font, 20
    );
    tools.push_back(std::move(startBtn));

    // END Button
    auto endBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + (buttonSize + gap) * 3, startY),
        "F", font, 20
    );
    tools.push_back(std::move(endBtn));

    // SPECIAL Button
    auto specialBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + (buttonSize + gap) * 4, startY),
        "SP", font, 16
    );
    tools.push_back(std::move(specialBtn));
}

void EditorToolbar::draw(sf::RenderWindow& window) {
    for (size_t i = 0; i < tools.size(); ++i) {
        // Draw glow effect for selected tool
        if (i == static_cast<size_t>(selectedTool)) {
            sf::Vector2f btnPos = tools[i]->getPosition();
            sf::Vector2f btnSize = tools[i]->getSize();

            // Position glow at center of button
            glowEffect.setPosition(btnPos.x + btnSize.x / 2.0f, btnPos.y + btnSize.y / 2.0f);
            window.draw(glowEffect);

            // Draw selection highlight around button
            selectionHighlight.setSize(btnSize + sf::Vector2f(8.0f, 8.0f));
            selectionHighlight.setPosition(btnPos - sf::Vector2f(4.0f, 4.0f));
            window.draw(selectionHighlight);
        }

        // Draw the button
        tools[i]->draw(window);

        // Draw tooltip on hover
        if (tools[i]->isHovered()) {
            std::string tooltip;
            switch (static_cast<EditorTool>(i)) {
            case EditorTool::WALL: tooltip = "Wall"; break;
            case EditorTool::ERASE: tooltip = "Erase"; break;
            case EditorTool::START: tooltip = "Start Position"; break;
            case EditorTool::END: tooltip = "End Position"; break;
            case EditorTool::SPECIAL: tooltip = "Special Cell"; break;
            }

            sf::Text tip(tooltip, *currentFont, 12);
            sf::Vector2f btnPos = tools[i]->getPosition();
            tip.setPosition(btnPos.x, btnPos.y - 25.0f);
            tip.setFillColor(sf::Color::White);

            // Draw background for tooltip
            sf::FloatRect tipBounds = tip.getLocalBounds();
            sf::RectangleShape tipBg(sf::Vector2f(tipBounds.width + 10.0f, tipBounds.height + 6.0f));
            tipBg.setPosition(btnPos.x - 5.0f, btnPos.y - 30.0f);
            tipBg.setFillColor(sf::Color(40, 40, 50, 220));
            tipBg.setOutlineColor(sf::Color::Cyan);
            tipBg.setOutlineThickness(1.0f);
            window.draw(tipBg);

            window.draw(tip);
        }
    }
}

bool EditorToolbar::handleClick(sf::Vector2f mousePos) {
    for (size_t i = 0; i < tools.size(); ++i) {
        if (tools[i]->contains(mousePos)) {
            if (static_cast<EditorTool>(i) != selectedTool) {
                selectedTool = static_cast<EditorTool>(i);

                // Trigger click effect on the selected button
                tools[i]->triggerClickEffect();

                // Update button colors based on selection
                // This is optional - you can add visual feedback here

                return true;
            }
        }
    }
    return false;
}

void EditorToolbar::handleHover(sf::Vector2f mousePos) {
    for (auto& btn : tools) {
        btn->setHovered(btn->contains(mousePos));
    }
}

EditorTool EditorToolbar::getSelectedTool() const {
    return selectedTool;
}

void EditorToolbar::setSelectedTool(EditorTool tool) {
    selectedTool = tool;
}