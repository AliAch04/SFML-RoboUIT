#include "EditorToolbar.h"

EditorToolbar::EditorToolbar() {}

void EditorToolbar::init(sf::Font& font, float startX, float startY) {
    tools.clear();

    float btnWidth = 120.0f;
    float btnHeight = 40.0f;
    float gap = 15.0f;
    float currentY = startY;

    // Création des 4 boutons requis
    // 1. WALL
    tools.push_back(std::make_unique<Button>(sf::Vector2f(btnWidth, btnHeight), sf::Vector2f(startX, currentY), "WALL", font, 18));
    currentY += btnHeight + gap;

    // 2. ERASE
    tools.push_back(std::make_unique<Button>(sf::Vector2f(btnWidth, btnHeight), sf::Vector2f(startX, currentY), "ERASE", font, 18));
    currentY += btnHeight + gap;

    // 3. START
    tools.push_back(std::make_unique<Button>(sf::Vector2f(btnWidth, btnHeight), sf::Vector2f(startX, currentY), "START", font, 18));
    currentY += btnHeight + gap;

    // 4. END
    tools.push_back(std::make_unique<Button>(sf::Vector2f(btnWidth, btnHeight), sf::Vector2f(startX, currentY), "END", font, 18));
    currentY += btnHeight + gap;

    // 5. SPECIAL
    tools.push_back(std::make_unique<Button>(sf::Vector2f(btnWidth, btnHeight), sf::Vector2f(startX, currentY), "SPECIAL", font, 18));


    // Initialiser le cadre de sélection (Highlight)
    selectionHighlight.setSize(sf::Vector2f(btnWidth + 6, btnHeight + 6));
    selectionHighlight.setFillColor(sf::Color::Transparent);
    selectionHighlight.setOutlineColor(sf::Color::Yellow);
    selectionHighlight.setOutlineThickness(3);

    // Positionner le highlight sur le premier outil (Mur) par défaut
    selectionHighlight.setPosition(startX - 3, startY - 3);
}

void EditorToolbar::draw(sf::RenderWindow& window) {
    // Dessiner le highlight SOUS les boutons ou AUTOUR
    window.draw(selectionHighlight);

    for (const auto& btn : tools) {
        btn->draw(window);
    }
}

void EditorToolbar::handleHover(sf::Vector2f mousePos) {
    for (const auto& btn : tools) {
        btn->setHovered(btn->contains(mousePos));
    }
}

bool EditorToolbar::handleClick(sf::Vector2f mousePos) {
    for (size_t i = 0; i < tools.size(); ++i) {
        if (tools[i]->contains(mousePos)) {
            // Mise à jour de l'outil sélectionné
            if (i == 0) selectedTool = EditorTool::WALL;
            else if (i == 1) selectedTool = EditorTool::ERASE;
            else if (i == 2) selectedTool = EditorTool::START;
            else if (i == 3) selectedTool = EditorTool::END;
            else if (i == 4) selectedTool = EditorTool::SPECIAL;


            // Déplacer le cadre jaune sur le bouton cliqué
            // On triche un peu : on sait que tools[i] est un Button, 
            // mais Button n'a pas getPosition(). On recupère la position via le calcul d'init ou 
            // on ajoute une méthode getPosition() à Button.h si besoin.
            // ICI: Supposons que nous devons le calculer basiquement pour cet exemple:
            float startX = 640.0f; // Doit correspondre au init
            float startY = 180.0f; // Doit correspondre au init (sous le Zoom)
            float btnHeight = 40.0f;
            float gap = 15.0f;
            float newY = startY + i * (btnHeight + gap);

            selectionHighlight.setPosition(startX - 3, newY - 3);

            return true;
        }
    }
    return false;
}

EditorTool EditorToolbar::getSelectedTool() const {
    return selectedTool;
}