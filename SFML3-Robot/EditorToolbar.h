class EditorToolbar {
private:
    std::vector<Button> toolButtons;
    EditorTool selectedTool = EditorTool::WALL;
    sf::Vector2f position;
    float width;
    sf::Font* font;

public:
    void init(sf::Font& f, float x, float y, float panelWidth) {
        font = &f;
        position = sf::Vector2f(x, y);
        width = panelWidth;

        toolButtons.clear();

        // Calculate button sizes and positions - CENTERED
        float buttonSize = 50.0f;
        float gap = 15.0f;
        int numButtons = 5; // WALL, ERASE, START, END, SPECIAL
        float totalWidth = numButtons * buttonSize + (numButtons - 1) * gap;
        float startX = x + (panelWidth - totalWidth) / 2.0f;

        // WALL
        toolButtons.emplace_back(sf::Vector2f(buttonSize, buttonSize),
            sf::Vector2f(startX, y),
            "W", f, 20);

        // ERASE
        toolButtons.emplace_back(sf::Vector2f(buttonSize, buttonSize),
            sf::Vector2f(startX + buttonSize + gap, y),
            "E", f, 20);

        // START
        toolButtons.emplace_back(sf::Vector2f(buttonSize, buttonSize),
            sf::Vector2f(startX + (buttonSize + gap) * 2, y),
            "S", f, 20);

        // END
        toolButtons.emplace_back(sf::Vector2f(buttonSize, buttonSize),
            sf::Vector2f(startX + (buttonSize + gap) * 3, y),
            "F", f, 20);

        // SPECIAL
        toolButtons.emplace_back(sf::Vector2f(buttonSize, buttonSize),
            sf::Vector2f(startX + (buttonSize + gap) * 4, y),
            "SP", f, 16);
    }

    void draw(sf::RenderWindow& window) {
        for (size_t i = 0; i < toolButtons.size(); ++i) {
            // Draw selection indicator for selected tool
            if (i == static_cast<size_t>(selectedTool)) {
                // Draw highlight background
                sf::RectangleShape highlight(toolButtons[i].getSize() + sf::Vector2f(8, 8));
                highlight.setPosition(toolButtons[i].getPosition() - sf::Vector2f(4, 4));
                highlight.setFillColor(sf::Color(100, 150, 255, 100));
                highlight.setOutlineColor(sf::Color::Cyan);
                highlight.setOutlineThickness(2.0f);
                window.draw(highlight);

                // Draw glow effect
                sf::CircleShape glow(35.0f);
                glow.setPosition(toolButtons[i].getPosition() +
                    toolButtons[i].getSize() / 2.0f - sf::Vector2f(35, 35));
                glow.setFillColor(sf::Color(100, 150, 255, 30));
                window.draw(glow);
            }

            // Draw button
            toolButtons[i].draw(window);

            // Draw tooltip text
            if (toolButtons[i].isHovered()) {
                std::string tooltip;
                switch (static_cast<EditorTool>(i)) {
                case EditorTool::WALL: tooltip = "Wall"; break;
                case EditorTool::ERASE: tooltip = "Erase"; break;
                case EditorTool::START: tooltip = "Start Position"; break;
                case EditorTool::END: tooltip = "End Position"; break;
                case EditorTool::SPECIAL: tooltip = "Special Cell"; break;
                }

                sf::Text tip(tooltip, *font, 12);
                tip.setPosition(toolButtons[i].getPosition().x,
                    toolButtons[i].getPosition().y - 25);
                tip.setFillColor(sf::Color::White);
                window.draw(tip);
            }
        }
    }

    bool handleClick(sf::Vector2f mousePos) {
        for (size_t i = 0; i < toolButtons.size(); ++i) {
            if (toolButtons[i].contains(mousePos)) {
                selectedTool = static_cast<EditorTool>(i);
                return true;
            }
        }
        return false;
    }

    void handleHover(sf::Vector2f mousePos) {
        for (auto& btn : toolButtons) {
            btn.setHovered(btn.contains(mousePos));
        }
    }

    EditorTool getSelectedTool() const { return selectedTool; }
    void setSelectedTool(EditorTool tool) { selectedTool = tool; }
};