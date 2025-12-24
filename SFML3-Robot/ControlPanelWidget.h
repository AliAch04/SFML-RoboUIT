#pragma once
#include <SFML/Graphics.hpp>

class ControlPanelWidget {
public:
    ControlPanelWidget();
    void draw(sf::RenderWindow& window);

private:
    sf::RectangleShape panel;
};
