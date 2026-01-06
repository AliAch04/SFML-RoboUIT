#pragma once
#include "NeuralNetwork.h"
#include "EvolutionaryAStar.h"
#include <vector>
#include <memory>
#include <unordered_map>

class MetaLearner {
private:
    std::unique_ptr<NeuralNetwork> metaNetwork;
    std::vector<std::vector<double>> experienceMemory;
    std::vector<std::vector<double>> adaptationPatterns;

    // Mémoire des stratégies réussies
    std::unordered_map<std::string, std::vector<double>> strategyLibrary;

    // Hyperparamètres du méta-apprentissage
    double metaLearningRate;
    double strategyExplorationRate;
    int experienceCapacity;

public:
    MetaLearner(int inputSize = 10, int hiddenSize = 20, int outputSize = 3);

    // Apprendre à partir d'une expérience
    void learnFromExperience(const std::vector<double>& mazeFeatures,
        const std::vector<double>& initialWeights,
        const std::vector<double>& finalWeights,
        double performanceScore);

    // Prédire les poids optimaux pour un nouveau labyrinthe
    std::vector<double> predictOptimalWeights(const std::vector<double>& mazeFeatures);

    // Générer une nouvelle stratégie d'adaptation
    std::vector<double> generateAdaptationStrategy(const std::vector<double>& currentState,
        double performanceTrend);

    // Évaluer une stratégie
    double evaluateStrategy(const std::vector<double>& strategy,
        const std::vector<double>& mazeFeatures) const;

    // Sélectionner la meilleure stratégie pour une situation
    std::vector<double> selectBestStrategy(const std::vector<double>& mazeFeatures,
        double currentPerformance);

    // Getters
    double getMetaLearningRate() const { return metaLearningRate; }
    int getExperienceCount() const { return experienceMemory.size(); }
    int getStrategyCount() const { return strategyLibrary.size(); }

    // Sauvegarde/chargement
    bool saveMetaModel(const std::string& filename) const;
    bool loadMetaModel(const std::string& filename);

private:
    std::vector<double> encodeExperience(const std::vector<double>& mazeFeatures,
        const std::vector<double>& weights,
        double performance);
    std::string generateStrategyKey(const std::vector<double>& features) const;
};