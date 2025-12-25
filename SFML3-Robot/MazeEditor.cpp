#include "MazeEditor.h"
#include <iostream>

// --- Constructeur ---
MazeEditor::MazeEditor(Maze& mazeRef)
    : maze(mazeRef), currentTool(EditorTool::WALL), isGhostVisible(false)
{
    // Configuration du carré visuel (le fantôme)
    ghostRect.setOutlineThickness(2.0f);
    ghostRect.setOutlineColor(sf::Color::Yellow);
    ghostRect.setFillColor(sf::Color(255, 255, 255, 100));
}

// --- Changer l'outil actif ---
void MazeEditor::setTool(EditorTool tool) {
    currentTool = tool;
}

// --- Calculer la position du carré fantôme (Visual Preview) ---
void MazeEditor::updateGhost(const sf::RenderWindow& window, float cellSize, sf::Vector2f mazeOffset) {
    sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePixel);

    // Calcul de la position relative au labyrinthe
    float relativeX = mouseWorld.x - mazeOffset.x;
    float relativeY = mouseWorld.y - mazeOffset.y;

    // Vérifier si la souris est DANS la zone du labyrinthe
    if (relativeX >= 0 && relativeY >= 0 &&
        relativeX < maze.width * cellSize &&
        relativeY < maze.height * cellSize)
    {
        isGhostVisible = true;

        // Conversion en coordonnées de grille
        int gridX = static_cast<int>(relativeX / cellSize);
        int gridY = static_cast<int>(relativeY / cellSize);

        ghostPos = { gridX, gridY };

        // Mise à jour visuelle (position et taille)
        ghostRect.setSize(sf::Vector2f(cellSize, cellSize));
        ghostRect.setPosition(
            gridX * cellSize + mazeOffset.x,
            gridY * cellSize + mazeOffset.y
        );

        // Couleur selon l'outil sélectionné
        sf::Color c;
        switch (currentTool) {
        case EditorTool::WALL:    c = sf::Color(80, 80, 80, 180); break;    // Gris foncé
        case EditorTool::ERASE:   c = sf::Color(255, 255, 255, 150); break; // Blanc
        case EditorTool::START:   c = sf::Color(0, 255, 0, 150); break;     // Vert
        case EditorTool::END:     c = sf::Color(255, 0, 0, 150); break;     // Rouge
        case EditorTool::SPECIAL: c = sf::Color(0, 0, 255, 150); break;     // Bleu
        default:                  c = sf::Color(255, 255, 255, 100); break;
        }
        ghostRect.setFillColor(c);
    }
    else {
        isGhostVisible = false;
    }
}

// --- Afficher le fantôme ---
void MazeEditor::draw(sf::RenderWindow& window) {
    if (isGhostVisible) {
        window.draw(ghostRect);
    }
}

// --- Appliquer l'outil (Clic Souris) avec Historique ---
void MazeEditor::applyTool()
{
    // Sécurité : si le fantôme n'est pas visible, on ne fait rien
    if (!isGhostVisible) return;

    Point p = { ghostPos.x, ghostPos.y }; // On utilise la position du fantôme

    // 1. Vérification des limites
    if (!maze.isValid(p)) return;

    // 2. Capture de l'état AVANT modif (pour Undo)
    CellType prevType = maze.grid[p.y][p.x]->getType();
    CellType newType = CellType::EMPTY;

    // Détermine le type cible
    switch (currentTool) {
    case EditorTool::WALL:    newType = CellType::WALL; break;
    case EditorTool::ERASE:   newType = CellType::EMPTY; break;
    case EditorTool::START:   newType = CellType::START; break;
    case EditorTool::END:     newType = CellType::END; break;
    case EditorTool::SPECIAL: newType = CellType::SPECIAL; break;
    }

    // Si on ne change rien, on sort (pas d'historique inutile)
    if (prevType == newType) return;

    // 3. Application de la modification
    switch (currentTool)
    {
    case EditorTool::WALL:
        if (p == maze.startPos) maze.startPos = { -1, -1 };
        if (p == maze.endPos)   maze.endPos = { -1, -1 };
        maze.setCell(p.x, p.y, CellType::WALL);
        break;

    case EditorTool::ERASE:
        if (p == maze.startPos) maze.startPos = { -1, -1 };
        if (p == maze.endPos)   maze.endPos = { -1, -1 };
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
    }

    // 4. Enregistrement dans l'historique
    EditAction action;
    action.x = p.x;
    action.y = p.y;
    action.prevType = prevType;
    action.newType = newType;

    undoStack.push_back(action);

    // IMPORTANT : Une nouvelle action écrase le futur
    redoStack.clear();
}

// --- UNDO (Annuler) ---
void MazeEditor::undo()
{
    if (undoStack.empty()) return;

    // 1. Récupérer la dernière action
    EditAction lastAction = undoStack.back();
    undoStack.pop_back();

    // 2. Restaurer l'ancien type
    // Note : on remet directement le type sans passer par moveStartParams pour éviter les effets de bord
    maze.setCell(lastAction.x, lastAction.y, lastAction.prevType);

    // 3. Mettre à jour les pointeurs Start/End pour la logique
    Point p = { lastAction.x, lastAction.y };

    // Si on remet un START, on met à jour startPos
    if (lastAction.prevType == CellType::START) {
        maze.startPos = p;
    }
    // Si on remet un END, on met à jour endPos
    else if (lastAction.prevType == CellType::END) {
        maze.endPos = p;
    }

    // Cas spécial : Si on vient d'annuler la pose d'un START (on revient à EMPTY par exemple),
    // il faut être sûr que maze.startPos ne pointe plus vers cette case.
    if (lastAction.newType == CellType::START && maze.startPos == p) {
        maze.startPos = { -1, -1 };
    }
    if (lastAction.newType == CellType::END && maze.endPos == p) {
        maze.endPos = { -1, -1 };
    }

    // 4. Ajouter à la pile Redo
    redoStack.push_back(lastAction);

    std::cout << "Undo: (" << p.x << "," << p.y << ")" << std::endl;
}

// --- REDO (Rétablir) ---
void MazeEditor::redo()
{
    if (redoStack.empty()) return;

    // 1. Récupérer l'action
    EditAction action = redoStack.back();
    redoStack.pop_back();

    // 2. Ré-appliquer le nouveau type
    maze.setCell(action.x, action.y, action.newType);

    // 3. Mettre à jour les pointeurs Start/End
    Point p = { action.x, action.y };

    if (action.newType == CellType::START) {
        maze.startPos = p;
    }
    else if (action.newType == CellType::END) {
        maze.endPos = p;
    }

    // Si on rétablit un Mur sur une case qui était Start, on perd le Start
    if (action.prevType == CellType::START && maze.startPos == p) {
        maze.startPos = { -1, -1 };
    }
    if (action.prevType == CellType::END && maze.endPos == p) {
        maze.endPos = { -1, -1 };
    }

    // 4. Remettre dans Undo
    undoStack.push_back(action);

    std::cout << "Redo: (" << p.x << "," << p.y << ")" << std::endl;
}

// --- Fonctions utilitaires privées ---

void MazeEditor::moveStartParams(Point newStart)
{
    // 1. Si une position de départ existait déjà, on la vide
    if (maze.width > 0 && maze.isValid(maze.startPos))
    {
        maze.setCell(maze.startPos.x, maze.startPos.y, CellType::EMPTY);
    }

    // 2. Si on pose le départ sur l'arrivée, on supprime l'arrivée
    if (newStart == maze.endPos)
    {
        maze.endPos = { -1, -1 };
    }

    // 3. On place le nouveau départ
    maze.setCell(newStart.x, newStart.y, CellType::START);
    maze.startPos = newStart;
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
        maze.startPos = { -1, -1 };
    }

    // 3. Placement
    maze.setCell(newEnd.x, newEnd.y, CellType::END);
    maze.endPos = newEnd;
}

bool MazeEditor::isMazeReady() const
{
    return maze.isValid(maze.startPos) && maze.isValid(maze.endPos) && (maze.startPos != maze.endPos);
}