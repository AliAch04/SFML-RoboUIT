#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class TextInput {
private:
    sf::RectangleShape box;
    sf::Text label;
    sf::Text text;
    std::string rawText;

    // Variable membre renommée pour éviter les conflits
    bool m_isFocused;

public:
    // --- C'EST CETTE LIGNE QUI CORRIGE VOTRE ERREUR ---
    // Le constructeur doit accepter exactement ces 5 types
    TextInput(sf::Vector2f size, sf::Vector2f position, std::string labelText, sf::Font& font, int fontSize);
    // --------------------------------------------------

    // Méthodes
    void draw(sf::RenderWindow& window);
    void handleTextEntered(sf::Uint32 unicode);

    // Setters & Getters
    void setPosition(const sf::Vector2f& position);
    void setFocused(bool focused);

    bool isFocused() const { return m_isFocused; }

    void setText(const std::string& str);
    std::string getText() const;

    bool contains(sf::Vector2f point) const;
};