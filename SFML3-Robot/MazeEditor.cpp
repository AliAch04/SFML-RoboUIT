#include "MazeEditor.h"

MazeEditor::MazeEditor(Maze &mazeRef) : maze(mazeRef) {}

void MazeEditor::applyTool(Point p, EditorTool tool)
{
    // 1. Vérification des limites via ta méthode existante dans Maze.h
    if (!maze.isValid(p))
        return;

    // 2. Logique selon l'outil
    switch (tool)
    {
    case EditorTool::WALL:
        // On ne peut pas écraser le Départ ou l'Arrivée avec un mur sans réfléchir
        // Mais pour faire simple : si c'est Start/End, on invalide la pos correspondante
        if (p == maze.startPos)
            maze.startPos = {-1, -1};
        if (p == maze.endPos)
            maze.endPos = {-1, -1};
        maze.setCell(p.x, p.y, CellType::WALL);
        break;

    case EditorTool::ERASE:
        if (p == maze.startPos)
            maze.startPos = {-1, -1};
        if (p == maze.endPos)
            maze.endPos = {-1, -1};
        maze.setCell(p.x, p.y, CellType::EMPTY);
        break;

    case EditorTool::START:
        moveStartParams(p);
        break;

    case EditorTool::END:
        moveEndParams(p);
        break;

    case EditorTool::SPECIAL:
        maze.setCell(p.x, p.y, CellType::SPECIAL);
        break;

    default:
        break;
    }
}

void MazeEditor::moveStartParams(Point newStart)
{
    // 1. Si une position de départ existait déjà, on la vide
    // (Note : on vérifie que width > 0 pour être sûr que maze est init)
    if (maze.width > 0 && maze.isValid(maze.startPos))
    {
        maze.setCell(maze.startPos.x, maze.startPos.y, CellType::EMPTY);
    }

    // 2. Si on pose le départ sur l'arrivée, on supprime l'arrivée
    if (newStart == maze.endPos)
    {
        maze.endPos = {-1, -1}; // Invalide l'arrivée
    }

    // 3. On place le nouveau départ
    maze.setCell(newStart.x, newStart.y, CellType::START);
    maze.startPos = newStart; // Mise à jour de la variable publique de Maze
}

void MazeEditor::moveEndParams(Point newEnd)
{
    // 1. Nettoyage de l'ancienne arrivée
    if (maze.width > 0 && maze.isValid(maze.endPos))
    {
        maze.setCell(maze.endPos.x, maze.endPos.y, CellType::EMPTY);
    }

    // 2. Si on pose l'arrivée sur le départ
    if (newEnd == maze.startPos)
    {
        maze.startPos = {-1, -1};
    }

    // 3. Placement
    maze.setCell(newEnd.x, newEnd.y, CellType::END);
    maze.endPos = newEnd;
}

bool MazeEditor::isMazeReady() const
{
    // Le labyrinthe est valide si startPos et endPos sont dans les limites
    return maze.isValid(maze.startPos) && maze.isValid(maze.endPos) && (maze.startPos != maze.endPos);
}