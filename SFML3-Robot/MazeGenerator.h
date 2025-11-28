#pragma once
#include <vector>
#include <random>

class Maze;

class MazeGenerator {
public:
    static void generateSolvableMaze(Maze* maze);

private:
    static void initializeWithWalls(Maze* maze);
    static void carvePaths(Maze* maze);
};