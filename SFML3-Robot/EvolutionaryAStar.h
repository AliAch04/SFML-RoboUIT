#pragma once
#include "AStar.h"
#include "DeepQLearning.h"
#include <vector>
#include <memory>
#include <functional>

struct HybridNode {
    Point pos;
    double g; // Coût réel depuis le départ
    double h; // Heuristique traditionnelle
    double q; // Valeur Q apprise (confiance du robot)
    double f; // Score hybride: f = w1*g + w2*h + w3*q

    // Surcharge pour la file de priorité
    bool operator>(const HybridNode& other) const {
        return f > other.f;
    }

    bool operator<(const HybridNode& other) const {
        return f < other.f;
    }
};

class EvolutionaryAStar : public PathFinder {
private:
    std::unique_ptr<DeepQLearning> deepLearner;
    std::unique_ptr<ManhattanHeuristic> traditionalHeuristic;

    // Poids adaptatifs pour la fonction de coût hybride
    double weightG; // Poids pour le coût réel
    double weightH; // Poids pour l'heuristique traditionnelle
    double weightQ; // Poids pour la valeur Q apprise

    // Mémoire à long terme pour l'adaptation
    std::unordered_map<Point, double, PointHash> confidenceMap; // Confiance par position
    std::vector<std::pair<double, double>> performanceHistory; // (success_rate, adaptation_speed)

    // Paramètres d'adaptation
    double adaptationRate;
    double minWeight;
    double maxWeight;
    int iterationsWithoutImprovement;
    int maxIterationsWithoutImprovement;

    // Métriques de performance
    double convergenceSpeed;
    double optimalityRate;
    double adaptabilityScore;

public:
    EvolutionaryAStar();

    // Surcharge de la méthode findPath avec apprentissage évolutif
    std::vector<Point> findPath(Maze* maze) override;

    // Méthodes évolutives
    std::vector<Point> findPathWithEvolution(Maze* maze, int maxGenerations = 50);
    void adaptWeightsBasedOnPerformance(bool pathFound, double pathLength, double optimalLength);
    void updateConfidenceMap(const Point& pos, double confidence);

    // Génération de chemins alternatifs (diversité)
    std::vector<std::vector<Point>> generateAlternativePaths(Maze* maze, int numPaths = 5);
    std::vector<Point> crossoverPaths(const std::vector<Point>& path1, const std::vector<Point>& path2);
    std::vector<Point> mutatePath(const std::vector<Point>& path, double mutationRate = 0.1);

    // Méta-apprentissage (apprendre à apprendre)
    void metaLearnFromExperience();
    double predictOptimalWeights(const std::vector<double>& mazeFeatures);

    // Getters pour les métriques
    double getConvergenceSpeed() const { return convergenceSpeed; }
    double getOptimalityRate() const { return optimalityRate; }
    double getAdaptabilityScore() const { return adaptabilityScore; }
    double getWeightG() const { return weightG; }
    double getWeightH() const { return weightH; }
    double getWeightQ() const { return weightQ; }

    // Affichage des statistiques
    void printStatistics() const;

    // Sauvegarde/chargement du modèle évolutif
    bool saveEvolutionaryModel(const std::string& filename) const;
    bool loadEvolutionaryModel(const std::string& filename);

private:
    double calculateHybridF(double g, double h, double q) const;
    double getLearnedHeuristic(const Point& current, const Point& goal, Maze* maze);
    void updatePerformanceMetrics(bool success, double actualLength, double optimalLength, int iterations);
    std::vector<double> extractMazeFeatures(Maze* maze) const;
};