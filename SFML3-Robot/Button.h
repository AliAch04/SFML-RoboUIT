#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Button {
private:
    // On utilise ConvexShape au lieu de RectangleShape pour faire des arrondis
    sf::ConvexShape shape;
    sf::Text text;
    bool isHovered = false;

    // Variables pour mémoriser la taille
    sf::Vector2f m_size;
    sf::Vector2f m_position;

    // Fonction interne pour redessiner la forme arrondie
    void updateRoundedShape(float radius);

public:
    Button(const sf::Vector2f& size, const sf::Vector2f& position,
        const std::string& buttonText, sf::Font& font, unsigned int characterSize = 24);

    void setHovered(bool hover);
    bool contains(const sf::Vector2f& point) const;
    void draw(sf::RenderWindow& window) const;
    void setText(const std::string& newText, sf::Font& font);
    void setPosition(const sf::Vector2f& position);
};