#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Button {
private:
    sf::ConvexShape shape;
    sf::Text text;
    bool isHovered = false;

    sf::Vector2f m_size;
    sf::Vector2f m_position;

    void updateRoundedShape(float cut);

    // Variables pour le feedback visuel
    sf::Clock clickTimer;
    bool isBeingClicked = false;

public:
    Button(sf::Vector2f size, sf::Vector2f position, std::string textStr, sf::Font& font, int fontSize = 14);

    // UNE SEULE FONCTION DRAW (non const)
    void draw(sf::RenderWindow& window);

    void triggerClickEffect();
    void setHovered(bool hover);
    bool contains(const sf::Vector2f& point) const;
    void setText(const std::string& newText, sf::Font& font);
    void setPosition(const sf::Vector2f& position);
    std::string getText() const { return text.getString(); }
};