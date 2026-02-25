#include "NeuralNetwork.h"
#include <iostream>

NeuralNetwork::NeuralNetwork(const std::vector<int>& sizes, double lr)
    : layerSizes(sizes), learningRate(lr) {

    // Initialize weights and biases
    weights.resize(layerSizes.size() - 1);
    biases.resize(layerSizes.size() - 1);

    std::random_device rd;
    std::mt19937 gen(rd());

    for (size_t i = 0; i < weights.size(); ++i) {
        int inputSize = layerSizes[i];
        int outputSize = layerSizes[i + 1];

        // Xavier/Glorot initialization
        double stddev = sqrt(2.0 / (inputSize + outputSize));
        std::normal_distribution<double> dist(0.0, stddev);

        weights[i].resize(outputSize);
        biases[i].resize(outputSize);

        for (int j = 0; j < outputSize; ++j) {
            weights[i][j].resize(inputSize);
            for (int k = 0; k < inputSize; ++k) {
                weights[i][j][k] = dist(gen);
            }
            biases[i][j] = dist(gen) * 0.1;
        }
    }
}

std::vector<double> NeuralNetwork::predict(const std::vector<double>& input) {
    if (layerSizes.empty()) {
        std::cerr << "Erreur: Réseau de neurones non initialisé!" << std::endl;
        return std::vector<double>(1, 0.0); // Valeur par défaut
    }

    if (input.size() != static_cast<size_t>(layerSizes[0])) {
        std::cerr << "Erreur: Taille d'entrée incorrecte! "
            << "Attendu: " << layerSizes[0]
            << ", Reçu: " << input.size() << std::endl;

        // OPTION 1: Redimensionner l'entrée (padding avec des zéros)
        std::vector<double> paddedInput = input;
        if (paddedInput.size() < static_cast<size_t>(layerSizes[0])) {
            paddedInput.resize(layerSizes[0], 0.0);
        }
        else if (paddedInput.size() > static_cast<size_t>(layerSizes[0])) {
            paddedInput.resize(layerSizes[0]);
        }

        // Continuer avec l'entrée ajustée
        std::vector<double> current = paddedInput;

        // ... reste du code de prédiction
        for (size_t layer = 0; layer < weights.size(); ++layer) {
            std::vector<double> next(layerSizes[layer + 1], 0.0);

            for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
                double sum = biases[layer][neuron];

                for (int inputIdx = 0; inputIdx < layerSizes[layer]; ++inputIdx) {
                    sum += weights[layer][neuron][inputIdx] * current[inputIdx];
                }

                // Activation function (ReLU for hidden, linear for output)
                if (layer < weights.size() - 1) {
                    next[neuron] = relu(sum);
                }
                else {
                    next[neuron] = sum; // Linear activation for output
                }
            }

            current = next;
        }

        return current;
    }

    // Code original si la taille est correcte
    std::vector<double> current = input;

    for (size_t layer = 0; layer < weights.size(); ++layer) {
        std::vector<double> next(layerSizes[layer + 1], 0.0);

        for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
            double sum = biases[layer][neuron];

            for (int inputIdx = 0; inputIdx < layerSizes[layer]; ++inputIdx) {
                sum += weights[layer][neuron][inputIdx] * current[inputIdx];
            }

            // Activation function (ReLU for hidden, linear for output)
            if (layer < weights.size() - 1) {
                next[neuron] = relu(sum);
            }
            else {
                next[neuron] = sum; // Linear activation for output
            }
        }

        current = next;
    }

    return current;
}

double NeuralNetwork::train(const std::vector<double>& input, const std::vector<double>& target) {
    // Forward pass
    std::vector<std::vector<double>> activations;
    std::vector<std::vector<double>> zs; // weighted sums

    activations.push_back(input);

    std::vector<double> current = input;
    for (size_t layer = 0; layer < weights.size(); ++layer) {
        std::vector<double> z(layerSizes[layer + 1], 0.0);
        std::vector<double> activation(layerSizes[layer + 1], 0.0);

        for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
            z[neuron] = biases[layer][neuron];

            for (int inputIdx = 0; inputIdx < layerSizes[layer]; ++inputIdx) {
                z[neuron] += weights[layer][neuron][inputIdx] * current[inputIdx];
            }

            if (layer < weights.size() - 1) {
                activation[neuron] = relu(z[neuron]);
            }
            else {
                activation[neuron] = z[neuron]; // Linear output
            }
        }

        zs.push_back(z);
        activations.push_back(activation);
        current = activation;
    }

    // Backward pass
    std::vector<std::vector<double>> deltas(weights.size());

    // Output layer delta
    size_t lastLayer = weights.size() - 1;
    deltas[lastLayer].resize(layerSizes[lastLayer + 1]);

    for (int neuron = 0; neuron < layerSizes[lastLayer + 1]; ++neuron) {
        double error = activations.back()[neuron] - target[neuron];
        deltas[lastLayer][neuron] = error; // Linear activation derivative = 1
    }

    // Hidden layers delta
    for (int layer = static_cast<int>(weights.size()) - 2; layer >= 0; --layer) {
        deltas[layer].resize(layerSizes[layer + 1]);

        for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
            double sum = 0.0;
            for (int nextNeuron = 0; nextNeuron < layerSizes[layer + 2]; ++nextNeuron) {
                sum += weights[layer + 1][nextNeuron][neuron] * deltas[layer + 1][nextNeuron];
            }

            double activation = activations[layer + 1][neuron];
            deltas[layer][neuron] = sum * reluDerivative(activation);
        }
    }

    // Update weights and biases
    for (size_t layer = 0; layer < weights.size(); ++layer) {
        for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
            // Update bias
            biases[layer][neuron] -= learningRate * deltas[layer][neuron];

            // Update weights
            for (int inputIdx = 0; inputIdx < layerSizes[layer]; ++inputIdx) {
                weights[layer][neuron][inputIdx] -= learningRate *
                    deltas[layer][neuron] * activations[layer][inputIdx];
            }
        }
    }

    // Calculate MSE loss
    double loss = 0.0;
    for (size_t i = 0; i < target.size(); ++i) {
        double error = activations.back()[i] - target[i];
        loss += error * error;
    }
    loss /= target.size();

    return loss;
}

double NeuralNetwork::trainBatch(const std::vector<std::vector<double>>& inputs,
    const std::vector<std::vector<double>>& targets) {
    if (inputs.size() != targets.size() || inputs.empty()) {
        return 0.0;
    }

    double totalLoss = 0.0;
    int batchSize = static_cast<int>(inputs.size());

    // Accumulate gradients
    std::vector<std::vector<std::vector<double>>> weightGradients = weights;
    std::vector<std::vector<double>> biasGradients = biases;

    // Initialize gradients to zero
    for (size_t layer = 0; layer < weightGradients.size(); ++layer) {
        for (size_t neuron = 0; neuron < weightGradients[layer].size(); ++neuron) {
            for (size_t w = 0; w < weightGradients[layer][neuron].size(); ++w) {
                weightGradients[layer][neuron][w] = 0.0;
            }
            biasGradients[layer][neuron] = 0.0;
        }
    }

    // Process each sample in batch
    for (int sample = 0; sample < batchSize; ++sample) {
        // Forward pass
        std::vector<std::vector<double>> activations;
        std::vector<std::vector<double>> zs;

        activations.push_back(inputs[sample]);
        std::vector<double> current = inputs[sample];

        for (size_t layer = 0; layer < weights.size(); ++layer) {
            std::vector<double> z(layerSizes[layer + 1], 0.0);
            std::vector<double> activation(layerSizes[layer + 1], 0.0);

            for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
                z[neuron] = biases[layer][neuron];
                for (int inputIdx = 0; inputIdx < layerSizes[layer]; ++inputIdx) {
                    z[neuron] += weights[layer][neuron][inputIdx] * current[inputIdx];
                }

                if (layer < weights.size() - 1) {
                    activation[neuron] = relu(z[neuron]);
                }
                else {
                    activation[neuron] = z[neuron];
                }
            }

            zs.push_back(z);
            activations.push_back(activation);
            current = activation;
        }

        // Backward pass
        std::vector<std::vector<double>> deltas(weights.size());

        // Output layer
        size_t lastLayer = weights.size() - 1;
        deltas[lastLayer].resize(layerSizes[lastLayer + 1]);
        for (int neuron = 0; neuron < layerSizes[lastLayer + 1]; ++neuron) {
            double error = activations.back()[neuron] - targets[sample][neuron];
            deltas[lastLayer][neuron] = error;
            totalLoss += error * error;
        }

        // Hidden layers
        for (int layer = static_cast<int>(weights.size()) - 2; layer >= 0; --layer) {
            deltas[layer].resize(layerSizes[layer + 1]);
            for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
                double sum = 0.0;
                for (int nextNeuron = 0; nextNeuron < layerSizes[layer + 2]; ++nextNeuron) {
                    sum += weights[layer + 1][nextNeuron][neuron] * deltas[layer + 1][nextNeuron];
                }
                double activation = activations[layer + 1][neuron];
                deltas[layer][neuron] = sum * reluDerivative(activation);
            }
        }

        // Accumulate gradients
        for (size_t layer = 0; layer < weights.size(); ++layer) {
            for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
                biasGradients[layer][neuron] += deltas[layer][neuron];

                for (int inputIdx = 0; inputIdx < layerSizes[layer]; ++inputIdx) {
                    weightGradients[layer][neuron][inputIdx] +=
                        deltas[layer][neuron] * activations[layer][inputIdx];
                }
            }
        }
    }

    // Average gradients and update
    for (size_t layer = 0; layer < weights.size(); ++layer) {
        for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
            biases[layer][neuron] -= learningRate * (biasGradients[layer][neuron] / batchSize);

            for (int inputIdx = 0; inputIdx < layerSizes[layer]; ++inputIdx) {
                weights[layer][neuron][inputIdx] -= learningRate *
                    (weightGradients[layer][neuron][inputIdx] / batchSize);
            }
        }
    }

    return totalLoss / (batchSize * targets[0].size());
}

bool NeuralNetwork::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Save layer sizes
    size_t numLayers = layerSizes.size();
    file.write(reinterpret_cast<const char*>(&numLayers), sizeof(numLayers));
    for (int size : layerSizes) {
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    }

    // Save learning rate
    file.write(reinterpret_cast<const char*>(&learningRate), sizeof(learningRate));

    // Save weights and biases
    for (size_t layer = 0; layer < weights.size(); ++layer) {
        for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
            // Save bias
            file.write(reinterpret_cast<const char*>(&biases[layer][neuron]), sizeof(double));

            // Save weights
            for (int w = 0; w < layerSizes[layer]; ++w) {
                file.write(reinterpret_cast<const char*>(&weights[layer][neuron][w]), sizeof(double));
            }
        }
    }

    file.close();
    return true;
}

bool NeuralNetwork::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Load layer sizes
    size_t numLayers;
    file.read(reinterpret_cast<char*>(&numLayers), sizeof(numLayers));

    layerSizes.resize(numLayers);
    for (size_t i = 0; i < numLayers; ++i) {
        file.read(reinterpret_cast<char*>(&layerSizes[i]), sizeof(int));
    }

    // Load learning rate
    file.read(reinterpret_cast<char*>(&learningRate), sizeof(learningRate));

    // Initialize network structure
    weights.resize(layerSizes.size() - 1);
    biases.resize(layerSizes.size() - 1);

    for (size_t layer = 0; layer < weights.size(); ++layer) {
        weights[layer].resize(layerSizes[layer + 1]);
        biases[layer].resize(layerSizes[layer + 1]);

        for (int neuron = 0; neuron < layerSizes[layer + 1]; ++neuron) {
            weights[layer][neuron].resize(layerSizes[layer]);

            // Load bias
            file.read(reinterpret_cast<char*>(&biases[layer][neuron]), sizeof(double));

            // Load weights
            for (int w = 0; w < layerSizes[layer]; ++w) {
                file.read(reinterpret_cast<char*>(&weights[layer][neuron][w]), sizeof(double));
            }
        }
    }

    file.close();
    return true;
}

std::vector<double> NeuralNetwork::flattenState(const std::vector<double>& state, int action) {
    std::vector<double> flattened = state;
    flattened.push_back(static_cast<double>(action));
    return flattened;
}

std::vector<double> NeuralNetwork::normalizeInput(const std::vector<double>& input,
    const std::vector<double>& mins,
    const std::vector<double>& maxs) {
    std::vector<double> normalized(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        double range = maxs[i] - mins[i];
        if (range > 0) {
            normalized[i] = (input[i] - mins[i]) / range * 2.0 - 1.0; // Normalize to [-1, 1]
        }
        else {
            normalized[i] = 0.0;
        }
    }

    return normalized;
}