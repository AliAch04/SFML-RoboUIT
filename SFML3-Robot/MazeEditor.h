#pragma once
#include "Maze.h"
#include "Enums.h"
#include "Point.h"

class MazeEditor
{
private:
    Maze &maze; // Référence vers ton instance de Maze

public:
    // Constructeur prenant le labyrinthe par référence
    MazeEditor(Maze &mazeRef);

    // Applique l'outil sélectionné sur une case spécifique
    void applyTool(Point p, EditorTool tool);

    // Vérifie si le labyrinthe est prêt à être résolu (Start + End existent)
    bool isMazeReady() const;

private:
    // Méthodes internes pour gérer l'unicité
    void moveStartParams(Point newStart);
    void moveEndParams(Point newEnd);
};