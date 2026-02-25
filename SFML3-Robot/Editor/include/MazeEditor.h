#pragma once
#include <SFML/Graphics.hpp>
#include <vector> // <--- AJOUT INDISPENSABLE POUR LES PILES
#include "Maze.h"
#include "Enums.h"
#include "Point.h"

// Structure pour enregistrer une action (pour le Undo/Redo)
struct EditAction
{
    int x;
    int y;
    CellType prevType; // Ce qu'il y avait avant
    CellType newType;  // Ce qu'on a mis
};

class MazeEditor
{
private:
    Maze& maze;              // Référence vers ton instance de Maze

    // --- MEMBRES POUR LA PRÉVISUALISATION ---
    sf::RectangleShape ghostRect; // Le carré visuel (le fantôme)
    EditorTool currentTool;       // L'outil actuellement sélectionné (pour la couleur)
    bool isGhostVisible;          // Doit-on afficher le fantôme ?
    sf::Vector2i ghostPos;        // La position (x,y) sur la grille visée

    // --- GESTION UNDO / REDO ---
    std::vector<EditAction> undoStack;
    std::vector<EditAction> redoStack;

public:
    // Constructeur prenant le labyrinthe par référence
    MazeEditor(Maze& mazeRef);

    // Définit l'outil actif (Mur, Gomme, Départ, Arrivée)
    void setTool(EditorTool tool);

    // Applique l'outil sélectionné sur la case visée par le fantôme
    void applyTool();

    // --- NOUVELLES FONCTIONS UNDO / REDO ---
    void undo();
    void redo();

    // --- FONCTIONS D'AFFICHAGE ---

    // Met à jour la position et la couleur du fantôme selon la souris
    void updateGhost(const sf::RenderWindow& window, float cellSize, sf::Vector2f mazeOffset);

    // Affiche le fantôme
    void draw(sf::RenderWindow& window);

    // Vérifie si le labyrinthe est prêt à être résolu (Start + End existent)
    bool isMazeReady() const;

private:
    // Méthodes internes pour gérer l'unicité
    void moveStartParams(Point newStart);
    void moveEndParams(Point newEnd);
};