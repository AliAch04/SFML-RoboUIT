#include "TextInput.h"
#include <iostream>

TextInput::TextInput(sf::Vector2f size, sf::Vector2f position, std::string labelText, sf::Font& font, int fontSize)
{
    // Configuration Boîte
    box.setSize(size);
    box.setPosition(position);
    box.setFillColor(sf::Color(50, 50, 60));
    box.setOutlineThickness(1.0f);
    box.setOutlineColor(sf::Color(100, 100, 100));

    // Configuration Label
    label.setFont(font);
    label.setString(labelText);
    label.setCharacterSize(12);
    label.setFillColor(sf::Color::White);
    label.setPosition(position.x, position.y - 20.0f);

    // Configuration Texte
    text.setFont(font);
    text.setString("");
    text.setCharacterSize(fontSize);
    text.setFillColor(sf::Color::Cyan);
    text.setPosition(position.x + 5.0f, position.y + 5.0f);

    rawText = "";

    // CORRECTION : Utilisation de m_isFocused
    m_isFocused = false;
}

void TextInput::draw(sf::RenderWindow& window)
{
    // CORRECTION : Utilisation de m_isFocused
    if (m_isFocused) {
        box.setOutlineColor(sf::Color::Cyan);
        box.setOutlineThickness(2.0f);
    }
    else {
        box.setOutlineColor(sf::Color(100, 100, 100));
        box.setOutlineThickness(1.0f);
    }

    window.draw(box);
    window.draw(label);
    window.draw(text);
}

void TextInput::setPosition(const sf::Vector2f& position) {
    box.setPosition(position);
    // Réduisez à -18.0f au lieu de -25.0f pour coller un peu plus à la boite
    label.setPosition(position.x, position.y - 18.0f);
    text.setPosition(position.x + 5.0f, position.y + 5.0f);
}

void TextInput::setFocused(bool focused) {
    // CORRECTION : Utilisation de m_isFocused
    m_isFocused = focused;
}

bool TextInput::contains(sf::Vector2f point) const {
    return box.getGlobalBounds().contains(point);
}

void TextInput::setText(const std::string& str) {
    rawText = str;
    text.setString(rawText);
}

std::string TextInput::getText() const {
    return rawText;
}

void TextInput::handleTextEntered(sf::Uint32 unicode) {
    if (!m_isFocused) return;

    // 1. Gestion de la touche Effacer (Backspace = 8)
    if (unicode == 8) {
        if (!rawText.empty()) {
            rawText.pop_back();
        }
    }
    // 2. Gestion des caractères imprimables (Unicode > 31 et < 128 pour ASCII standard)
    else if (unicode > 31 && unicode < 128) {
        // On définit une limite de caractères pour ne pas que le texte dépasse de la boîte
        if (rawText.size() < 20) {
            rawText += static_cast<char>(unicode);
        }
    }

    // Mettre à jour l'affichage
    text.setString(rawText);
}