#include "QLearning.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <iostream>

QLearning::QLearning(double alpha, double gamma, double epsilon,
    double decay, double minEpsilon)
    : learningRate(alpha), discountFactor(gamma), explorationRate(epsilon),
    explorationDecay(decay), minExplorationRate(minEpsilon), rng(std::random_device{}()) {
}

int QLearning::chooseAction(const Point& state, const std::vector<int>& availableActions) {
    if (availableActions.empty()) return -1;

    // Exploration vs exploitation
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < explorationRate) {
        // Exploration: choisir une action aléatoire
        std::uniform_int_distribution<int> actionDist(0, static_cast<int>(availableActions.size()) - 1);
        return availableActions[actionDist(rng)];
    }
    else {
        // Exploitation: choisir l'action avec la plus grande Q-value
        int bestAction = availableActions[0];
        double bestValue = getQValue(state, bestAction);

        for (size_t i = 1; i < availableActions.size(); ++i) {
            double value = getQValue(state, availableActions[i]);
            if (value > bestValue) {
                bestValue = value;
                bestAction = availableActions[i];
            }
        }
        return bestAction;
    }
}

void QLearning::update(const Point& state, int action, double reward,
    const Point& nextState, const std::vector<int>& nextActions) {
    if (action < 0 || action > 3) return;

    // Initialiser si nécessaire
    initializeQValue(state, action);

    // Calcul de la Q-value cible
    double currentQ = getQValue(state, action);
    double maxNextQ = nextActions.empty() ? 0.0 : getMaxQValue(nextState, nextActions);

    // Formule Q-learning
    double newQ = currentQ + learningRate * (reward + discountFactor * maxNextQ - currentQ);

    // Mettre à jour la Q-table
    qTable[{state, action}] = newQ;

    // Décroissance de l'exploration
    explorationRate = std::max(minExplorationRate, explorationRate * explorationDecay);
}

double QLearning::getQValue(const Point& state, int action) const {
    auto it = qTable.find({ state, action });
    if (it != qTable.end()) {
        return it->second;
    }
    return 0.0; // Valeur par défaut
}

void QLearning::initializeQValue(const Point& state, int action) {
    StateAction key{ state, action };
    if (qTable.find(key) == qTable.end()) {
        qTable[key] = 0.0;
    }
}

double QLearning::getMaxQValue(const Point& state, const std::vector<int>& actions) {
    if (actions.empty()) return 0.0;

    double maxQ = getQValue(state, actions[0]);
    for (size_t i = 1; i < actions.size(); ++i) {
        double qValue = getQValue(state, actions[i]);
        if (qValue > maxQ) {
            maxQ = qValue;
        }
    }
    return maxQ;
}

double QLearning::getLearningScore() const {
    if (qTable.empty()) return 0.0;

    // Calculer un score basé sur la convergence des Q-values
    double totalPositive = 0.0;
    int count = 0;

    for (const auto& entry : qTable) {
        if (entry.second > 0) {
            totalPositive += entry.second;
            count++;
        }
    }

    if (count == 0) return 0.0;

    // Normaliser le score entre 0 et 100
    double avgPositive = totalPositive / count;
    double score = std::min(100.0, avgPositive * 10.0); // Facteur d'échelle
    return score;
}

bool QLearning::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    // Sauvegarder les hyperparamètres
    file << learningRate << " " << discountFactor << " "
        << explorationRate << " " << explorationDecay << " "
        << minExplorationRate << "\n";

    // Sauvegarder la Q-table
    file << qTable.size() << "\n";
    for (const auto& entry : qTable) {
        file << entry.first.state.x << " " << entry.first.state.y << " "
            << entry.first.action << " " << entry.second << "\n";
    }

    file.close();
    return true;
}

bool QLearning::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    // Charger les hyperparamètres
    file >> learningRate >> discountFactor >> explorationRate
        >> explorationDecay >> minExplorationRate;

    // Charger la Q-table
    size_t tableSize;
    file >> tableSize;

    qTable.clear();
    for (size_t i = 0; i < tableSize; ++i) {
        int x, y, action;
        double value;
        file >> x >> y >> action >> value;
        qTable[{Point{ x, y }, action}] = value;
    }

    file.close();
    return true;
}