#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Button {
private:
    sf::ConvexShape shape;
    sf::Text text;
    

    sf::Vector2f m_size;
    sf::Vector2f m_position;

    void updateRoundedShape(float cut);

    // Variables pour le feedback visuel
    sf::Clock clickTimer;
    bool isBeingClicked = false;

public:

    // Default constructor
    Button() = default;
    
    // Main constructor
    Button(const sf::Vector2f& size, const sf::Vector2f& position,
           const std::string& buttonText, sf::Font& font, 
           unsigned int characterSize = 24);

    bool isHovered = false;

    // UNE SEULE FONCTION DRAW (non const)
    void draw(sf::RenderWindow& window);

    void triggerClickEffect();
    void setHovered(bool hover);
    bool contains(const sf::Vector2f& point) const;
    void setText(const std::string& newText, sf::Font& font);
    void setPosition(const sf::Vector2f& position);
    sf::Vector2f getPosition() const { return m_position; }
    sf::Vector2f getSize() const { return m_size; }
    bool isHovered() const { return isHovered; }
    std::string getText() const { return text.getString(); }
};