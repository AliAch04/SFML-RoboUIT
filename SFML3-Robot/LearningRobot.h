#pragma once
#include "Robot.h"
#include "LearningRobot.h"
#include "QLearning.h"
#include "Maze.h"
#include <memory>

class LearningRobot : public Robot {
private:
    std::unique_ptr<QLearning> qLearning;
    std::shared_ptr<Maze> currentMaze;
    Point previousState;
    int previousAction;
    double totalReward;
    int successfulTrials;
    int totalTrials;

    // Récompenses
    const double REWARD_GOAL = 100.0;
    const double REWARD_MOVE = -0.1;
    const double PENALTY_WALL = -5.0;
    const double PENALTY_LOOP = -2.0;
    const double REWARD_PROGRESS = 0.5;

    // Mémoire pour détecter les boucles
    std::vector<Point> visitedStates;
    static const int MAX_MEMORY = 100;

public:
    LearningRobot();
    ~LearningRobot() = default;

    void setMaze(std::shared_ptr<Maze> maze);

    // Surcharge des méthodes de Robot
    void moveTo(Point next) override;
    void update(float dt) override;
    void setPosition(Point p) override;

    // Méthodes d'apprentissage
    void startNewTrial();
    void receiveReward(double reward);
    void learnFromExperience(const Point& currentState, int action,
        double reward, const Point& nextState);

    // Gestion des actions disponibles
    std::vector<int> getAvailableActions(const Point& state);
    Point getNextState(const Point& state, int action);

    // Getters
    double getLearningScore() const;
    double getTotalReward() const { return totalReward; }
    int getSuccessfulTrials() const { return successfulTrials; }
    int getTotalTrials() const { return totalTrials; }
    double getSuccessRate() const;

    // Sauvegarde/chargement du modèle
    bool saveModel(const std::string& filename);
    bool loadModel(const std::string& filename);

    // Réinitialisation
    void resetLearning();

private:
    double calculateReward(const Point& state, const Point& nextState);
    bool isLooping(const Point& state);
    bool isMakingProgress(const Point& state, const Point& goal);
};