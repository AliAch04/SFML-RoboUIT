#include "Button.h"

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position,
    const std::string& textStr, sf::Font& font,
    unsigned int characterSize)
    : m_size(size), m_position(position), isHovered(false), isBeingClicked(false)
{
    float cutSize = 8.0f;

    // Initialize shape before using it
    shape = sf::ConvexShape();

    // Call updateRoundedShape AFTER setting m_size and m_position
    updateRoundedShape(cutSize);

    // Style par défaut
    shape.setFillColor(sf::Color(20, 30, 45, 220));
    shape.setOutlineColor(sf::Color(0, 200, 255, 150));
    shape.setOutlineThickness(2.0f);

    text.setFont(font);
    text.setString(textStr);
    text.setCharacterSize(characterSize);
    text.setFillColor(sf::Color::White);

    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width / 2.0f,
        textBounds.top + textBounds.height / 2.0f);
    text.setPosition(position.x + size.x / 2.0f,
        position.y + size.y / 2.0f);
}

void Button::draw(sf::RenderWindow& window) {
    // Gestion du feedback visuel (effet flash)
    if (isBeingClicked && clickTimer.getElapsedTime().asSeconds() < 0.1f) {
        shape.setFillColor(sf::Color(100, 255, 255, 200)); // Flash Cyan
    }
    else {
        isBeingClicked = false;
        if (isHovered) {
            shape.setFillColor(sf::Color(40, 60, 90, 250)); // Couleur Hover
        }
        else {
            shape.setFillColor(sf::Color(20, 30, 45, 220)); // Couleur Normale
        }
    }
    window.draw(shape);
    window.draw(text);
}

void Button::triggerClickEffect() {
    isBeingClicked = true;
    clickTimer.restart();
}

// Nouvelle logique : Forme à 8 points (Octogone étiré / Coins coupés)
void Button::updateRoundedShape(float cut) {
    shape.setPointCount(8);

    // On s'assure que la coupe n'est pas trop grande par rapport au bouton
    if (cut * 2 > m_size.y) cut = m_size.y / 2.0f;

    // Définition manuelle des 8 points pour des coupes nettes à 45 degrés
    // Haut-Gauche (coupé)
    shape.setPoint(0, sf::Vector2f(cut, 0));

    // Haut-Droit (coupé)
    shape.setPoint(1, sf::Vector2f(m_size.x - cut, 0));
    shape.setPoint(2, sf::Vector2f(m_size.x, cut));

    // Bas-Droit (coupé)
    shape.setPoint(3, sf::Vector2f(m_size.x, m_size.y - cut));
    shape.setPoint(4, sf::Vector2f(m_size.x - cut, m_size.y));

    // Bas-Gauche (coupé)
    shape.setPoint(5, sf::Vector2f(cut, m_size.y));
    shape.setPoint(6, sf::Vector2f(0, m_size.y - cut));

    // Retour Haut-Gauche
    shape.setPoint(7, sf::Vector2f(0, cut));

    shape.setPosition(m_position);
}

void Button::setHovered(bool hover) {
    isHovered = hover;
    if (hover) {
        // Effet "Activé" : Fond plus clair + Bordure Cyan pure et brillante
        shape.setFillColor(sf::Color(40, 60, 90, 250));
        shape.setOutlineColor(sf::Color::Cyan);
        shape.setOutlineThickness(3.0f);
    }
    else {
        // Effet "Repos" : Fond sombre + Bordure plus discrète
        shape.setFillColor(sf::Color(20, 30, 45, 220));
        shape.setOutlineColor(sf::Color(0, 200, 255, 150));
        shape.setOutlineThickness(2.0f);
    }
}

bool Button::contains(const sf::Vector2f& point) const {
    return shape.getGlobalBounds().contains(point);
}

void Button::setText(const std::string& newText, sf::Font& font) {
    text.setString(newText);
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    text.setPosition(m_position.x + m_size.x / 2.0f, m_position.y + m_size.y / 2.0f);
}

void Button::setPosition(const sf::Vector2f& position) {
    m_position = position;
    shape.setPosition(position);
    text.setPosition(m_position.x + m_size.x / 2.0f, m_position.y + m_size.y / 2.0f);
}

// GETTERS - IMPLEMENT ONLY ONCE
sf::Vector2f Button::getPosition() const {
    return m_position;
}

sf::Vector2f Button::getSize() const {
    return m_size;
}

std::string Button::getText() const {
    return text.getString();
}