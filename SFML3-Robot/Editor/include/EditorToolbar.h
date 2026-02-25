#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Button.h"
#include "Enums.h"

class EditorToolbar {
private:
    std::vector<std::unique_ptr<Button>> tools;
    sf::RectangleShape selectionHighlight; // Cadre pour l'outil actif
    sf::CircleShape glowEffect; // Effet de lueur
    EditorTool selectedTool = EditorTool::WALL;

    // Couleurs pour les icônes des outils
    sf::Color wallColor = sf::Color::Black;
    sf::Color eraseColor = sf::Color(200, 200, 200);
    sf::Color startColor = sf::Color(100, 220, 100);
    sf::Color endColor = sf::Color(220, 100, 100);
    sf::Color specialColor = sf::Color(255, 165, 0); // Orange for special

    // Position et dimensions
    sf::Vector2f position;
    float panelWidth;
    sf::Font* currentFont;

public:
    EditorToolbar();

    // Initialise la barre dans le panneau de droite avec centrage
    void init(sf::Font& font, float startX, float startY, float panelWidth = 270.0f);

    // Dessine les outils avec highlight
    void draw(sf::RenderWindow& window);

    // Gère le clic. Retourne TRUE si un outil a été changé.
    bool handleClick(sf::Vector2f mousePos);

    // Gère le survol (Hover)
    void handleHover(sf::Vector2f mousePos);

    EditorTool getSelectedTool() const;
    void setSelectedTool(EditorTool tool);
};