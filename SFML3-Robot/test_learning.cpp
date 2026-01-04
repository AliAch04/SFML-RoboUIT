#include "LearningRobot.h"
#include "Maze.h"
#include <iostream>

void testQLearning() {
    std::cout << "=== Test Q-Learning Robot ===" << std::endl;

    // Créer un labyrinthe simple
    auto maze = std::make_shared<Maze>(5, 5);
    maze->setCell(1, 1, CellType::START);
    maze->setCell(3, 3, CellType::END);

    // Créer le robot apprenant
    LearningRobot robot;
    robot.setMaze(maze);
    robot.setPosition(maze->startPos);

    // Simuler quelques mouvements
    std::cout << "Début de l'apprentissage..." << std::endl;

    for (int trial = 0; trial < 10; ++trial) {
        robot.startNewTrial();
        std::cout << "Essai " << trial + 1
            << " - Score: " << robot.getLearningScore() << "%" << std::endl;
    }

    std::cout << "Taux de réussite final: " << robot.getSuccessRate() << "%" << std::endl;
    std::cout << "Taille Q-table: " << robot.getLearningScore() << " entrées" << std::endl;

    // Test sauvegarde/chargement
    if (robot.saveModel("robot_model.dat")) {
        std::cout << "Modèle sauvegardé avec succès!" << std::endl;
    }
}