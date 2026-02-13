#include "EditorToolbar.h"
#include <iostream>

EditorToolbar::EditorToolbar() {
    // Initialize selection highlight
    selectionHighlight.setFillColor(sf::Color(100, 150, 255, 80)); // Semi-transparent blue
    selectionHighlight.setOutlineColor(sf::Color::Cyan);
    selectionHighlight.setOutlineThickness(2.0f);

    // Initialize glow effect
    glowEffect.setFillColor(sf::Color(100, 150, 255, 40));
    glowEffect.setRadius(35.0f); // Slightly larger glow
    glowEffect.setOrigin(35.0f, 35.0f);
}

void EditorToolbar::init(sf::Font& font, float startX, float startY, float panelWidth) {
    this->position = sf::Vector2f(startX, startY);
    this->panelWidth = panelWidth;
    this->currentFont = &font;

    tools.clear();

    // Calculate centered positions for 5 tools
    float buttonSize = 50.0f;
    float gap = 18.0f; // Slightly larger gap between buttons
    int numButtons = 5; // WALL, ERASE, START, END, SPECIAL
    float totalWidth = numButtons * buttonSize + (numButtons - 1) * gap;
    float startButtonX = startX + (panelWidth - totalWidth) / 2.0f;

    // ADD TOP MARGIN - Move buttons down by 15px
    float buttonY = startY + 15.0f;

    // WALL Button
    auto wallBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX, buttonY),
        "W", font, 22 // Slightly larger font
    );
    tools.push_back(std::move(wallBtn));

    // ERASE Button
    auto eraseBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + buttonSize + gap, buttonY),
        "E", font, 22
    );
    tools.push_back(std::move(eraseBtn));

    // START Button
    auto startBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + (buttonSize + gap) * 2, buttonY),
        "S", font, 22
    );
    tools.push_back(std::move(startBtn));

    // END Button
    auto endBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + (buttonSize + gap) * 3, buttonY),
        "F", font, 22
    );
    tools.push_back(std::move(endBtn));

    // SPECIAL Button
    auto specialBtn = std::make_unique<Button>(
        sf::Vector2f(buttonSize, buttonSize),
        sf::Vector2f(startButtonX + (buttonSize + gap) * 4, buttonY),
        "SP", font, 18 // Slightly smaller for "SP"
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
            selectionHighlight.setSize(btnSize + sf::Vector2f(10.0f, 10.0f)); // Slightly larger highlight
            selectionHighlight.setPosition(btnPos - sf::Vector2f(5.0f, 5.0f));
            window.draw(selectionHighlight);
        }

        // Draw the button
        tools[i]->draw(window);

        // Draw ENHANCED tooltip on hover
        if (tools[i]->isHoveredState()) {
            std::string tooltip;
            sf::Color tooltipColor;

            switch (static_cast<EditorTool>(i)) {
            case EditorTool::WALL:
                tooltip = "WALL - Place obstacles";
                tooltipColor = sf::Color(100, 100, 100);
                break;
            case EditorTool::ERASE:
                tooltip = "ERASE - Remove cells";
                tooltipColor = sf::Color(200, 200, 200);
                break;
            case EditorTool::START:
                tooltip = "START - Set starting point";
                tooltipColor = sf::Color(100, 220, 100);
                break;
            case EditorTool::END:
                tooltip = "END - Set goal point";
                tooltipColor = sf::Color(220, 100, 100);
                break;
            case EditorTool::SPECIAL:
                tooltip = "SPECIAL - Special cell";
                tooltipColor = sf::Color(255, 165, 0);
                break;
            }

            // Create tooltip text with larger font
            sf::Text tip(tooltip, *currentFont, 16); // Increased from 12 to 16
            tip.setFillColor(sf::Color::White);

            // Get button position
            sf::Vector2f btnPos = tools[i]->getPosition();
            sf::Vector2f btnSize = tools[i]->getSize();

            // Position tooltip ABOVE the button with more spacing
            tip.setPosition(btnPos.x + btnSize.x / 2.0f, btnPos.y - 45.0f); // Moved up from -25 to -45
            tip.setOrigin(tip.getLocalBounds().width / 2.0f, tip.getLocalBounds().height / 2.0f);

            // Draw ENHANCED background for tooltip (with padding)
            sf::FloatRect tipBounds = tip.getLocalBounds();
            float padding = 15.0f; // More padding
            float bgWidth = tipBounds.width + padding * 2;
            float bgHeight = tipBounds.height + padding;

            sf::RectangleShape tipBg(sf::Vector2f(bgWidth, bgHeight));
            tipBg.setPosition(btnPos.x + btnSize.x / 2.0f - bgWidth / 2.0f, btnPos.y - 55.0f); // Adjusted position

            // Stylish background with gradient effect
            tipBg.setFillColor(sf::Color(20, 20, 30, 240));
            tipBg.setOutlineColor(tooltipColor);
            tipBg.setOutlineThickness(2.5f);

            // Add rounded corners effect using multiple rectangles
            window.draw(tipBg);

            // Draw a small arrow pointing to the button
            sf::ConvexShape arrow;
            arrow.setPointCount(3);
            arrow.setPoint(0, sf::Vector2f(btnPos.x + btnSize.x / 2.0f - 8, btnPos.y - 15));
            arrow.setPoint(1, sf::Vector2f(btnPos.x + btnSize.x / 2.0f, btnPos.y - 5));
            arrow.setPoint(2, sf::Vector2f(btnPos.x + btnSize.x / 2.0f + 8, btnPos.y - 15));
            arrow.setFillColor(tooltipColor);
            window.draw(arrow);

            // Draw the tooltip text
            window.draw(tip);

            // Optional: Add shortcut hint
            std::string shortcut;
            switch (static_cast<EditorTool>(i)) {
            case EditorTool::WALL: shortcut = "[1]"; break;
            case EditorTool::ERASE: shortcut = "[2]"; break;
            case EditorTool::START: shortcut = "[3]"; break;
            case EditorTool::END: shortcut = "[4]"; break;
            case EditorTool::SPECIAL: shortcut = "[5]"; break;
            }

            sf::Text shortcutText(shortcut, *currentFont, 12);
            shortcutText.setFillColor(sf::Color::Cyan);
            shortcutText.setPosition(btnPos.x + btnSize.x / 2.0f, btnPos.y - 30.0f);
            shortcutText.setOrigin(shortcutText.getLocalBounds().width / 2.0f,
                shortcutText.getLocalBounds().height / 2.0f);
            window.draw(shortcutText);
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