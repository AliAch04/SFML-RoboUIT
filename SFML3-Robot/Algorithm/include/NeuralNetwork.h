#pragma once
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <fstream>
#include <string>

class NeuralNetwork {
private:
    std::vector<std::vector<std::vector<double>>> weights; // weights[layer][neuron][input]
    std::vector<std::vector<double>> biases; // biases[layer][neuron]
    std::vector<int> layerSizes;
    double learningRate;

    // Activation functions
    double sigmoid(double x) const {
        return 1.0 / (1.0 + exp(-x));
    }

    double sigmoidDerivative(double x) const {
        return x * (1.0 - x);
    }

    double relu(double x) const {
        return std::max(0.0, x);
    }

    double reluDerivative(double x) const {
        return x > 0 ? 1.0 : 0.01; // Leaky ReLU
    }

    // Initialization
    void initializeWeights();

public:
    NeuralNetwork(const std::vector<int>& sizes, double lr = 0.01);

    // Forward propagation
    std::vector<double> predict(const std::vector<double>& input);

    // Training
    double train(const std::vector<double>& input, const std::vector<double>& target);

    // Batch training
    double trainBatch(const std::vector<std::vector<double>>& inputs,
        const std::vector<std::vector<double>>& targets);

    // Save/load
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);

    // Getters
    const std::vector<int>& getLayerSizes() const { return layerSizes; }
    double getLearningRate() const { return learningRate; }
    void setLearningRate(double lr) { learningRate = lr; }

    // Utility
    static std::vector<double> flattenState(const std::vector<double>& state, int action);
    static std::vector<double> normalizeInput(const std::vector<double>& input,
        const std::vector<double>& mins,
        const std::vector<double>& maxs);
};