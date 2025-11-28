#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class TextInput {
private:
    sf::RectangleShape box;
    sf::Text text;
    sf::Text label;
    std::string inputText;
    bool focused = false;

public:
    TextInput(const sf::Vector2f& position, float width, const std::string& labelText, sf::Font& font);

    bool contains(const sf::Vector2f& point) const;
    void setFocused(bool focus);
    bool isFocused() const { return focused; }
    void handleTextEntered(sf::Uint32 unicode);
    std::string getText() const { return inputText; }
    void setText(const std::string& newText);
    void draw(sf::RenderWindow& window) const;
};