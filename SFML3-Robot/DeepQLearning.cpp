#include "DeepQLearning.h"
#include <iostream>
#include <algorithm>

DeepQLearning::DeepQLearning(int stateSize, double alpha, double gamma,
    double epsilon, double decay, double minEpsilon,
    int replaySize, int batchSize, int targetUpdateFreq)
    : QLearning(alpha, gamma, epsilon, decay, minEpsilon),
    updateTargetFrequency(targetUpdateFreq),
    trainingSteps(0),
    batchSize(batchSize) {

    // Initialize replay buffer
    replayBuffer = std::make_unique<ExperienceReplay>(replaySize, batchSize);

    // Initialize networks (will be properly sized when we know the maze)
    // Input: state + action, Output: Q-value
    int inputSize = stateSize;
    int outputSize = 1;

    initializeNetworks(inputSize, outputSize);

    // Initialize normalization bounds
    inputMins = { 0, 0, 0, 0, 0 }; // x, y, goal_x, goal_y, action
    inputMaxs = { 30, 30, 30, 30, 3 }; // Assuming max maze size 30x30
}

void DeepQLearning::initializeNetworks(int inputSize, int outputSize) {
    // Network architecture: input -> 64 -> 64 -> output
    std::vector<int> networkSizes = { inputSize, 64, 64, outputSize };

    qNetwork = std::make_unique<NeuralNetwork>(networkSizes, learningRate);
    targetNetwork = std::make_unique<NeuralNetwork>(networkSizes, learningRate);

    // Copy weights from Q-network to target network
    *targetNetwork = *qNetwork;
}

int DeepQLearning::chooseAction(const Point& state, const std::vector<int>& availableActions) {
    if (availableActions.empty()) return -1;

    // Exploration vs exploitation
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    if (dist(rng) < explorationRate) {
        // Exploration: random action
        std::uniform_int_distribution<int> actionDist(0, static_cast<int>(availableActions.size()) - 1);
        return availableActions[actionDist(rng)];
    }
    else {
        // Exploitation: use neural network to choose best action
        // This requires knowing the goal position - will be handled by LearningRobot
        return availableActions[0]; // Placeholder - actual implementation needs goal position
    }
}

void DeepQLearning::update(const Point& state, int action, double reward,
    const Point& nextState, const std::vector<int>& nextActions) {
    // Store experience in replay buffer
    // Note: We need the goal position to properly preprocess states
    // This will be handled by LearningRobot

    // Train from replay buffer periodically
    if (replayBuffer->size() >= static_cast<size_t>(batchSize)) {
        trainFromReplay();
    }

    // Update target network periodically
    if (trainingSteps % updateTargetFrequency == 0) {
        updateTargetNetwork();
    }

    trainingSteps++;
}

void DeepQLearning::trainFromReplay() {
    auto batch = replayBuffer->sampleBatch();
    if (batch.empty()) return;

    double totalLoss = 0.0;
    int count = 0;

    for (const auto& experience : batch) {
        // Calculate target Q-value
        double targetQ = experience.reward;

        if (!experience.terminal) {
            // Use target network to predict next state Q-values
            // This is simplified - actual implementation needs proper state preprocessing
            std::vector<double> nextStateMax = targetNetwork->predict(experience.nextState);
            double maxNextQ = *std::max_element(nextStateMax.begin(), nextStateMax.end());
            targetQ += discountFactor * maxNextQ;
        }

        // Create target vector
        std::vector<double> target = { targetQ };

        // Train Q-network
        double loss = qNetwork->train(experience.state, target);
        totalLoss += loss;
        count++;
    }

    if (count > 0) {
        // Update exploration rate
        explorationRate = std::max(minExplorationRate, explorationRate * explorationDecay);
    }
}

void DeepQLearning::updateTargetNetwork() {
    // Soft update: target = tau * q + (1-tau) * target
    // For simplicity, we do hard update in this version
    *targetNetwork = *qNetwork;
}

std::vector<double> DeepQLearning::preprocessState(const Point& state, const Point& goal) const {
    std::vector<double> processed;

    // Normalize coordinates
    processed.push_back(static_cast<double>(state.x) / 30.0); // Assuming max maze size
    processed.push_back(static_cast<double>(state.y) / 30.0);
    processed.push_back(static_cast<double>(goal.x) / 30.0);
    processed.push_back(static_cast<double>(goal.y) / 30.0);

    // Add distance to goal (Manhattan)
    double distance = (std::abs(state.x - goal.x) + std::abs(state.y - goal.y)) / 60.0;
    processed.push_back(distance);

    return processed;
}

std::vector<double> DeepQLearning::preprocessStateAction(const Point& state,
    int action,
    const Point& goal) const {
    auto processedState = preprocessState(state, goal);

    // Add one-hot encoded action
    std::vector<double> stateAction = processedState;
    for (int i = 0; i < 4; ++i) {
        stateAction.push_back(i == action ? 1.0 : 0.0);
    }

    return stateAction;
}

double DeepQLearning::getQLoss() const {
    // This would track loss from training
    return 0.0; // Placeholder
}

bool DeepQLearning::saveModel(const std::string& filename) const {
    // Save Q-network
    if (!qNetwork->saveToFile(filename + "_qnetwork.dat")) {
        return false;
    }

    // Save target network
    if (!targetNetwork->saveToFile(filename + "_target.dat")) {
        return false;
    }

    // Save replay buffer
    if (!replayBuffer->saveToFile(filename + "_replay.dat")) {
        return false;
    }

    // Save hyperparameters
    std::ofstream file(filename + "_params.txt");
    if (!file.is_open()) return false;

    file << learningRate << "\n";
    file << discountFactor << "\n";
    file << explorationRate << "\n";
    file << explorationDecay << "\n";
    file << minExplorationRate << "\n";
    file << trainingSteps << "\n";

    file.close();
    return true;
}

bool DeepQLearning::loadModel(const std::string& filename) {
    // Load Q-network
    if (!qNetwork->loadFromFile(filename + "_qnetwork.dat")) {
        return false;
    }

    // Load target network
    if (!targetNetwork->loadFromFile(filename + "_target.dat")) {
        return false;
    }

    // Load replay buffer
    if (!replayBuffer->loadFromFile(filename + "_replay.dat")) {
        return false;
    }

    // Load hyperparameters
    std::ifstream file(filename + "_params.txt");
    if (!file.is_open()) return false;

    file >> learningRate;
    file >> discountFactor;
    file >> explorationRate;
    file >> explorationDecay;
    file >> minExplorationRate;
    file >> trainingSteps;

    file.close();
    return true;
}