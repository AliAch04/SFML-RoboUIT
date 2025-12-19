#pragma once

enum class AppState { MAIN_MENU, GAME, OPTIONS };

enum class MenuButton { START, OPTIONS, EXIT, NONE };

// AJOUT : Ajout de EDIT_MODE ici
enum class GameState {
    IDLE,
    SOLVING,
    COMPLETE,
    FAILED,
    EDIT_MODE
};

enum class CellType { EMPTY, WALL, START, END, SPECIAL };

enum class RobotState { IDLE, CALCULATING, MOVING, COMPLETED, PAUSED };

// AJOUT : Préparez ceci pour la tâche suivante (Barre d'outils)
enum class EditorTool {
    WALL,
    ERASE,
    START,
    END,
    SPECIAL,
    NONE
};