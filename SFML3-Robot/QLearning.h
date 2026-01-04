#pragma once
#include "Point.h"
#include <unordered_map>
#include <vector>
#include <random>
#include <string>

struct StateAction {
    Point state;
    int action; // 0: up, 1: down, 2: left, 3: right

    bool operator==(const StateAction& other) const {
        return state == other.state && action == other.action;
    }
};

struct StateActionHash {
    std::size_t operator()(const StateAction& sa) const {
        return (std::hash<int>()(sa.state.x) << 1) ^
            (std::hash<int>()(sa.state.y) << 2) ^
            (std::hash<int>()(sa.action) << 3);
    }
};

class QLearning {
private:
    std::unordered_map<StateAction, double, StateActionHash> qTable;
    double learningRate;
    double discountFactor;
    double explorationRate;
    double explorationDecay;
    double minExplorationRate;
    std::mt19937 rng;

public:
    QLearning(double alpha = 0.1, double gamma = 0.9,
        double epsilon = 1.0, double decay = 0.995,
        double minEpsilon = 0.01);

    // Sélection d'action avec stratégie epsilon-greedy
    int chooseAction(const Point& state, const std::vector<int>& availableActions);

    // Mise à jour de la Q-value
    void update(const Point& state, int action, double reward,
        const Point& nextState, const std::vector<int>& nextActions);

    // Getters et setters
    double getQValue(const Point& state, int action) const;
    void setExplorationRate(double epsilon);
    double getExplorationRate() const { return explorationRate; }

    // Sauvegarde/chargement
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);

    // Statistiques
    double getLearningScore() const; // Score d'apprentissage (0-100%)
    int getQTableSize() const { return static_cast<int>(qTable.size()); }

private:
    void initializeQValue(const Point& state, int action);
    double getMaxQValue(const Point& state, const std::vector<int>& actions);
};