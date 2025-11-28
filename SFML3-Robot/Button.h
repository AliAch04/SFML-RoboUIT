#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    bool isHovered = false;

public:
    Button(const sf::Vector2f& size, const sf::Vector2f& position,
        const std::string& buttonText, sf::Font& font, unsigned int characterSize = 24);

    void setHovered(bool hover);
    bool contains(const sf::Vector2f& point) const;
    void draw(sf::RenderWindow& window) const;
    void setText(const std::string& newText, sf::Font& font);
    void setPosition(const sf::Vector2f& position);
};