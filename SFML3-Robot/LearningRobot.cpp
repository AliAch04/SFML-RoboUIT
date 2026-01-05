#include "LearningRobot.h"
#include <algorithm>
#include <cmath>
#include <iostream>

LearningRobot::LearningRobot()
    : qLearning(std::make_unique<QLearning>()),
    previousState({ -1, -1 }),
    previousAction(-1),
    totalReward(0.0),
    successfulTrials(0),
    totalTrials(0) {
}

void LearningRobot::setMaze(Maze* maze) {
    currentMaze = maze;
    visitedStates.clear();
}

void LearningRobot::startNewTrial() {
    totalTrials++;
    totalReward = 0.0;
    visitedStates.clear();
    previousState = { -1, -1 };
    previousAction = -1;
}

void LearningRobot::moveTo(Point next) {
    if (!currentMaze) {
        Robot::moveTo(next);
        return;
    }

    // Apprentissage avant de bouger
    if (previousAction != -1) {
        Point current = getPosition();
        double reward = calculateReward(previousState, current);

        // Obtenir les actions disponibles pour l'état suivant
        std::vector<int> nextActions = getAvailableActions(current);

        // Mettre à jour Q-learning
        qLearning->update(previousState, previousAction, reward, current, nextActions);

        // Mettre à jour la récompense totale
        totalReward += reward;
        receiveReward(reward);
    }

    // Choisir la prochaine action
    Point currentPos = getPosition();
    std::vector<int> availableActions = getAvailableActions(currentPos);

    if (!availableActions.empty()) {
        previousAction = qLearning->chooseAction(currentPos, availableActions);
        previousState = currentPos;

        // Obtenir la prochaine position
        Point nextPos = getNextState(currentPos, previousAction);

        // Mémoriser l'état visité
        visitedStates.push_back(currentPos);
        if (visitedStates.size() > MAX_MEMORY) {
            visitedStates.erase(visitedStates.begin());
        }

        // Appeler la méthode parent pour le mouvement
        Robot::moveTo(nextPos);
    }
}

void LearningRobot::update(float dt) {
    Robot::update(dt);

    // Vérifier si le but est atteint
    if (currentMaze && getPosition() == currentMaze->endPos && getState() != RobotState::COMPLETED) {
        // Grande récompense pour avoir atteint le but
        receiveReward(REWARD_GOAL);
        successfulTrials++;
        std::cout << "Robot a atteint le but! Récompense: " << REWARD_GOAL
            << " Score d'apprentissage: " << getLearningScore() << "%" << std::endl;
    }
}

void LearningRobot::setPosition(Point p) {
    Robot::setPosition(p);
    visitedStates.clear();
    visitedStates.push_back(p);
    previousState = { -1, -1 };
    previousAction = -1;
}

std::vector<int> LearningRobot::getAvailableActions(const Point& state) {
    std::vector<int> actions;

    if (!currentMaze) return actions;

    // Directions: 0=up, 1=down, 2=left, 3=right
    Point directions[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    for (int i = 0; i < 4; ++i) {
        Point next = { state.x + directions[i].x, state.y + directions[i].y };
        if (currentMaze->isValid(next) && !currentMaze->isWall(next)) {
            actions.push_back(i);
        }
    }

    return actions;
}

Point LearningRobot::getNextState(const Point& state, int action) {
    Point directions[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    if (action >= 0 && action < 4) {
        return { state.x + directions[action].x, state.y + directions[action].y };
    }

    return state;
}

double LearningRobot::calculateReward(const Point& state, const Point& nextState) {
    if (!currentMaze) return REWARD_MOVE;

    // Pénalité pour heurter un mur
    if (!currentMaze->isValid(nextState) || currentMaze->isWall(nextState)) {
        return PENALTY_WALL;
    }

    // Récompense pour progresser vers le but
    double progressReward = 0.0;
    if (isMakingProgress(nextState, currentMaze->endPos)) {
        progressReward = REWARD_PROGRESS;
    }

    // Pénalité pour les boucles
    if (isLooping(nextState)) {
        return PENALTY_LOOP + progressReward + REWARD_MOVE;
    }

    // Récompense normale pour un mouvement valide
    return REWARD_MOVE + progressReward;
}

bool LearningRobot::isLooping(const Point& state) {
    // Vérifier si l'état a déjà été visité récemment
    return std::find(visitedStates.begin(), visitedStates.end(), state) != visitedStates.end();
}

bool LearningRobot::isMakingProgress(const Point& state, const Point& goal) {
    // Calcul de la distance de Manhattan
    int currentDist = std::abs(state.x - goal.x) + std::abs(state.y - goal.y);
    int prevDist = std::abs(previousState.x - goal.x) + std::abs(previousState.y - goal.y);

    return currentDist < prevDist;
}

void LearningRobot::receiveReward(double reward) {
    totalReward += reward;
}

double LearningRobot::getLearningScore() const {
    return qLearning->getLearningScore();
}

double LearningRobot::getSuccessRate() const {
    if (totalTrials == 0) return 0.0;
    return (static_cast<double>(successfulTrials) / totalTrials) * 100.0;
}

bool LearningRobot::saveModel(const std::string& filename) {
    return qLearning->saveToFile(filename);
}

bool LearningRobot::loadModel(const std::string& filename) {
    return qLearning->loadFromFile(filename);
}

void LearningRobot::resetLearning() {
    qLearning = std::make_unique<QLearning>();
    totalReward = 0.0;
    successfulTrials = 0;
    totalTrials = 0;
    visitedStates.clear();
}