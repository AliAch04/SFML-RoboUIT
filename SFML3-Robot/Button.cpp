#include "Button.h"
#include <string>

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position,
    const std::string& buttonText, sf::Font& font, unsigned int characterSize) {
    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(sf::Color(70, 70, 70));
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color::White);

    text.setFont(font);
    text.setString(buttonText);
    text.setCharacterSize(characterSize);
    text.setFillColor(sf::Color::White);

    // Center text in button
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::setHovered(bool hover) {
    isHovered = hover;
    shape.setFillColor(hover ? sf::Color(100, 100, 100) : sf::Color(70, 70, 70));
}

bool Button::contains(const sf::Vector2f& point) const {
    return shape.getGlobalBounds().contains(point);
}

void Button::draw(sf::RenderWindow& window) const {
    window.draw(shape);
    window.draw(text);
}

void Button::setText(const std::string& newText, sf::Font& font) {
    text.setString(newText);
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.width / 2.0f, textBounds.height / 2.0f);
    sf::Vector2f pos = shape.getPosition();
    sf::Vector2f size = shape.getSize();
    text.setPosition(pos.x + size.x / 2.0f, pos.y + size.y / 2.0f);
}

void Button::setPosition(const sf::Vector2f& position) {
    shape.setPosition(position);
    sf::Vector2f size = shape.getSize();
    text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}