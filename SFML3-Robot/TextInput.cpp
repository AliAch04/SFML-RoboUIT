#include "TextInput.h"
#include <string>

TextInput::TextInput(const sf::Vector2f& position, float width, const std::string& labelText, sf::Font& font) {
    box.setSize(sf::Vector2f(width, 30));
    box.setPosition(position);
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(2);
    box.setOutlineColor(sf::Color(150, 150, 150));

    label.setFont(font);
    label.setString(labelText);
    label.setCharacterSize(18);
    label.setFillColor(sf::Color::White);
    label.setPosition(position.x, position.y - 25);

    text.setFont(font);
    text.setString("");
    text.setCharacterSize(18);
    text.setFillColor(sf::Color::Black);
    text.setPosition(position.x + 5, position.y + 5);
}

bool TextInput::contains(const sf::Vector2f& point) const {
    return box.getGlobalBounds().contains(point);
}

void TextInput::setFocused(bool focus) {
    focused = focus;
    box.setOutlineColor(focus ? sf::Color::Blue : sf::Color(150, 150, 150));
}

void TextInput::handleTextEntered(sf::Uint32 unicode) {
    if (!focused) return;

    if (unicode == 8) { // Backspace
        if (!inputText.empty()) {
            inputText.pop_back();
        }
    }
    else if (unicode == 13) { // Enter
        focused = false;
        box.setOutlineColor(sf::Color(150, 150, 150));
    }
    else if (unicode >= 32 && unicode < 128) { // Printable characters
        inputText += static_cast<char>(unicode);
    }

    text.setString(inputText);
}

void TextInput::setText(const std::string& newText) {
    inputText = newText;
    text.setString(inputText);
}

void TextInput::draw(sf::RenderWindow& window) const {
    window.draw(box);
    window.draw(text);
    window.draw(label);
}