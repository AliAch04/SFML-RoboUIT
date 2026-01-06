#include "MetaLearner.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

MetaLearner::MetaLearner(int inputSize, int hiddenSize, int outputSize)
    : metaLearningRate(0.001), strategyExplorationRate(0.3), experienceCapacity(1000) {

    // Architecture: input -> hidden -> output
    std::vector<int> sizes = { inputSize, hiddenSize, outputSize };
    metaNetwork = std::make_unique<NeuralNetwork>(sizes, metaLearningRate);
}

void MetaLearner::learnFromExperience(const std::vector<double>& mazeFeatures,
    const std::vector<double>& initialWeights,
    const std::vector<double>& finalWeights,
    double performanceScore) {
    // Encoder l'expérience
    auto encoded = encodeExperience(mazeFeatures, finalWeights, performanceScore);
    experienceMemory.push_back(encoded);

    // Garder la mémoire limitée
    if (experienceMemory.size() > experienceCapacity) {
        experienceMemory.erase(experienceMemory.begin());
    }

    // Créer un motif d'adaptation (différence entre poids initiaux et finaux)
    std::vector<double> adaptationPattern;
    for (size_t i = 0; i < initialWeights.size() && i < finalWeights.size(); ++i) {
        adaptationPattern.push_back(finalWeights[i] - initialWeights[i]);
    }
    adaptationPatterns.push_back(adaptationPattern);

    // Entraîner le réseau méta sur le lot d'expériences
    if (experienceMemory.size() >= 10) {
        std::vector<std::vector<double>> batchInputs;
        std::vector<std::vector<double>> batchTargets;

        // Préparer un batch d'entraînement
        for (size_t i = 0; i < std::min(size_t(10), experienceMemory.size()); ++i) {
            // Les premières N caractéristiques sont les features du labyrinthe
            // Les suivantes sont les poids initiaux
            std::vector<double> input;

            // Features du labyrinthe (premières mazeFeatures.size() éléments)
            size_t featureSize = mazeFeatures.size();
            for (size_t j = 0; j < featureSize && j < experienceMemory[i].size(); ++j) {
                input.push_back(experienceMemory[i][j]);
            }

            // Performance précédente (dernier élément)
            if (!experienceMemory[i].empty()) {
                input.push_back(experienceMemory[i].back());
            }

            // Cible: les poids finaux (éléments entre features et performance)
            std::vector<double> target;
            for (size_t j = featureSize; j < experienceMemory[i].size() - 1 && j < featureSize + 3; ++j) {
                target.push_back(experienceMemory[i][j]);
            }

            if (!input.empty() && !target.empty()) {
                batchInputs.push_back(input);
                batchTargets.push_back(target);
            }
        }

        // Entraîner sur le batch
        if (!batchInputs.empty()) {
            double loss = metaNetwork->trainBatch(batchInputs, batchTargets);

            // Stocker la stratégie si elle est efficace
            if (performanceScore > 0.7) {
                std::string key = generateStrategyKey(mazeFeatures);
                strategyLibrary[key] = finalWeights;
            }
        }
    }
}

std::vector<double> MetaLearner::predictOptimalWeights(const std::vector<double>& mazeFeatures) {
    // Construire l'entrée pour le réseau méta
    std::vector<double> input = mazeFeatures;

    // Ajouter des poids neutres initiaux (0.33, 0.33, 0.33)
    input.push_back(0.33);
    input.push_back(0.33);
    input.push_back(0.33);

    // Ajouter une performance moyenne (0.5)
    input.push_back(0.5);

    // Prédiction du réseau méta
    auto prediction = metaNetwork->predict(input);

    // Normaliser les poids prédits pour qu'ils somment à 1
    double sum = 0.0;
    for (double val : prediction) {
        sum += val;
    }

    if (sum > 0) {
        for (double& val : prediction) {
            val /= sum;
        }
    }

    return prediction;
}

std::vector<double> MetaLearner::generateAdaptationStrategy(const std::vector<double>& currentState,
    double performanceTrend) {
    std::vector<double> strategy;

    // Exploration vs exploitation des stratégies
    if (strategyLibrary.empty() || ((double)rand() / RAND_MAX) < strategyExplorationRate) {
        // Exploration: générer une nouvelle stratégie
        double total = 0.0;
        for (int i = 0; i < 3; ++i) {
            double weight = 0.2 + 0.6 * ((double)rand() / RAND_MAX);
            strategy.push_back(weight);
            total += weight;
        }

        // Normaliser
        for (double& w : strategy) {
            w /= total;
        }
    }
    else {
        // Exploitation: utiliser une stratégie similaire de la bibliothèque
        std::string currentKey = generateStrategyKey(currentState);

        // Trouver la stratégie la plus similaire
        double bestSimilarity = -1.0;
        std::vector<double> bestStrategy;

        for (const auto& entry : strategyLibrary) {
            // Calculer la similarité (simplifié)
            double similarity = 0.0;
            for (size_t i = 0; i < std::min(currentState.size(), entry.second.size()); ++i) {
                similarity += 1.0 - std::abs(currentState[i] - entry.second[i]);
            }
            similarity /= currentState.size();

            if (similarity > bestSimilarity) {
                bestSimilarity = similarity;
                bestStrategy = entry.second;
            }
        }

        if (!bestStrategy.empty()) {
            strategy = bestStrategy;

            // Ajouter une petite perturbation basée sur la tendance de performance
            if (performanceTrend < 0) {
                // Performance en baisse: augmenter l'exploration
                for (double& w : strategy) {
                    w += 0.1 * ((double)rand() / RAND_MAX - 0.5);
                }
            }
        }
    }

    // Assurer que les poids sont valides
    for (double& w : strategy) {
        w = std::max(0.1, std::min(0.8, w));
    }

    // Normaliser
    double total = 0.0;
    for (double w : strategy) total += w;
    for (double& w : strategy) w /= total;

    return strategy;
}

double MetaLearner::evaluateStrategy(const std::vector<double>& strategy,
    const std::vector<double>& mazeFeatures) const {
    if (strategy.size() != 3) return 0.0;

    // Score basé sur la cohérence avec les expériences passées
    double score = 0.0;
    int count = 0;

    for (const auto& experience : experienceMemory) {
        if (experience.size() >= mazeFeatures.size() + 3 + 1) {
            // Extraire les poids de l'expérience
            std::vector<double> expWeights;
            for (size_t i = mazeFeatures.size(); i < mazeFeatures.size() + 3; ++i) {
                expWeights.push_back(experience[i]);
            }

            // Calculer la similarité
            double similarity = 0.0;
            for (size_t i = 0; i < 3; ++i) {
                similarity += 1.0 - std::abs(strategy[i] - expWeights[i]);
            }
            similarity /= 3.0;

            // Ponderer par la performance de l'expérience
            double expPerformance = experience.back();
            score += similarity * expPerformance;
            count++;
        }
    }

    return count > 0 ? score / count : 0.5; // Score par défaut
}

std::vector<double> MetaLearner::selectBestStrategy(const std::vector<double>& mazeFeatures,
    double currentPerformance) {
    // Si pas de stratégies stockées, prédire une nouvelle
    if (strategyLibrary.empty()) {
        return predictOptimalWeights(mazeFeatures);
    }

    // Évaluer les stratégies existantes
    double bestScore = -1.0;
    std::vector<double> bestStrategy;

    for (const auto& entry : strategyLibrary) {
        double score = evaluateStrategy(entry.second, mazeFeatures);

        // Ajuster le score basé sur la performance actuelle
        if (currentPerformance < 0.5) {
            // Mauvaise performance: favoriser les stratégies différentes
            score *= 0.8;
        }

        if (score > bestScore) {
            bestScore = score;
            bestStrategy = entry.second;
        }
    }

    return bestStrategy;
}

std::vector<double> MetaLearner::encodeExperience(const std::vector<double>& mazeFeatures,
    const std::vector<double>& weights,
    double performance) {
    std::vector<double> encoded;

    // Features du labyrinthe
    encoded.insert(encoded.end(), mazeFeatures.begin(), mazeFeatures.end());

    // Poids
    encoded.insert(encoded.end(), weights.begin(), weights.end());

    // Performance
    encoded.push_back(performance);

    return encoded;
}

std::string MetaLearner::generateStrategyKey(const std::vector<double>& features) const {
    std::stringstream key;
    key << std::fixed << std::setprecision(2);

    for (size_t i = 0; i < std::min(size_t(3), features.size()); ++i) {
        key << static_cast<int>(features[i] * 100) << "_";
    }

    return key.str();
}

bool MetaLearner::saveMetaModel(const std::string& filename) const {
    return metaNetwork->saveToFile(filename + "_metanetwork.dat");
}

bool MetaLearner::loadMetaModel(const std::string& filename) {
    return metaNetwork->loadFromFile(filename + "_metanetwork.dat");
}