#pragma once
#include "QLearning.h"
#include "NeuralNetwork.h"
#include "ExperienceReplay.h"
#include <vector>
#include <memory>

class DeepQLearning : public QLearning {
private:
    std::unique_ptr<NeuralNetwork> qNetwork;
    std::unique_ptr<NeuralNetwork> targetNetwork;
    std::unique_ptr<ExperienceReplay> replayBuffer;

    int updateTargetFrequency;
    int trainingSteps;
    int batchSize;

    // Normalization bounds
    std::vector<double> inputMins;
    std::vector<double> inputMaxs;

public:
    DeepQLearning(int stateSize = 5, // x, y, goal_x, goal_y, action
        double alpha = 0.001,
        double gamma = 0.99,
        double epsilon = 1.0,
        double decay = 0.995,
        double minEpsilon = 0.01,
        int replaySize = 10000,
        int batchSize = 32,
        int targetUpdateFreq = 100);

    // Override QLearning methods
    int chooseAction(const Point& state, const std::vector<int>& availableActions) override;
    void update(const Point& state, int action, double reward,
        const Point& nextState, const std::vector<int>& nextActions) override;

    // Deep Q-Learning specific methods
    void trainFromReplay();
    void updateTargetNetwork();

    // State preprocessing
    std::vector<double> preprocessState(const Point& state, const Point& goal) const;
    std::vector<double> preprocessStateAction(const Point& state, int action, const Point& goal) const;

    // Getters
    double getQLoss() const;
    int getTrainingSteps() const { return trainingSteps; }

    // Save/load
    bool saveModel(const std::string& filename) const;
    bool loadModel(const std::string& filename);

private:
    void initializeNetworks(int inputSize, int outputSize);
    double predictQValue(const std::vector<double>& stateAction);
    std::vector<double> predictQValues(const std::vector<double>& state);
};