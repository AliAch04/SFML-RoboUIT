#pragma once

enum class CellType { EMPTY, WALL, START, END, SPECIAL };
enum class GameState { IDLE, SOLVING, COMPLETE, FAILED };
enum class RobotState { IDLE, CALCULATING, MOVING, COMPLETED, PAUSED };
enum class AppState { MAIN_MENU, GAME, OPTIONS };
enum class MenuButton { START, OPTIONS, EXIT, NONE };

