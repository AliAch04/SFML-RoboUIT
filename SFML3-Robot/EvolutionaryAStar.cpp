#include "EvolutionaryAStar.h"
#include "NeuralNetwork.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <queue>
#include <fstream>

EvolutionaryAStar::EvolutionaryAStar()
    : weightG(0.4), weightH(0.4), weightQ(0.2),
    adaptationRate(0.1), minWeight(0.1), maxWeight(0.8),
    iterationsWithoutImprovement(0), maxIterationsWithoutImprovement(10),
    convergenceSpeed(0.0), optimalityRate(0.0), adaptabilityScore(0.5) {

    deepLearner = std::make_unique<DeepQLearning>();
    traditionalHeuristic = std::make_unique<ManhattanHeuristic>();
}

std::vector<Point> EvolutionaryAStar::findPath(Maze* maze) {
    // Utiliser la version évolutive par défaut
    return findPathWithEvolution(maze, 30);
}

std::vector<Point> EvolutionaryAStar::findPathWithEvolution(Maze* maze, int maxGenerations) {
    if (!maze) return {};
    if (!maze->isValid(maze->startPos) || !maze->isValid(maze->endPos)) return {};
    if (maze->startPos == maze->endPos) return { maze->startPos };

    std::cout << "=== Début de la recherche évolutive A* ===" << std::endl;
    std::cout << "Poids initiaux: G=" << weightG << " H=" << weightH << " Q=" << weightQ << std::endl;

    // Génération initiale de chemins
    auto initialPaths = generateAlternativePaths(maze, 5);
    if (initialPaths.empty()) return {};

    // Sélection du meilleur chemin initial
    std::vector<Point> bestPath = initialPaths[0];
    double bestLength = bestPath.size();

    // Évolution sur plusieurs générations
    for (int generation = 0; generation < maxGenerations; ++generation) {
        std::cout << "Génération " << generation + 1 << "/" << maxGenerations << std::endl;

        // Croisement et mutation
        std::vector<std::vector<Point>> newPopulation;

        for (size_t i = 0; i < initialPaths.size(); ++i) {
            for (size_t j = i + 1; j < initialPaths.size(); ++j) {
                // Croisement
                auto child = crossoverPaths(initialPaths[i], initialPaths[j]);

                // Mutation
                if (static_cast<double>(rand()) / RAND_MAX < 0.3) {
                    child = mutatePath(child, 0.15);
                }

                // Validation du chemin
                if (!child.empty() && child[0] == maze->startPos && child.back() == maze->endPos) {
                    newPopulation.push_back(child);

                    // Mettre à jour le meilleur chemin
                    if (child.size() < bestLength) {
                        bestLength = child.size();
                        bestPath = child;
                        iterationsWithoutImprovement = 0;

                        std::cout << "  Nouveau meilleur chemin trouvé: " << bestLength << " étapes" << std::endl;
                    }
                }
            }
        }

        // Adaptation des poids basée sur la performance
        adaptWeightsBasedOnPerformance(!bestPath.empty(), bestLength,
            std::abs(maze->endPos.x - maze->startPos.x) +
            std::abs(maze->endPos.y - maze->startPos.y));

        // Mise à jour de la population
        if (!newPopulation.empty()) {
            initialPaths = newPopulation;

            // Garder une diversité (élitisme + diversité)
            std::sort(initialPaths.begin(), initialPaths.end(),
                [](const std::vector<Point>& a, const std::vector<Point>& b) {
                    return a.size() < b.size();
                });

            // Garder les 5 meilleurs
            if (initialPaths.size() > 5) {
                initialPaths.resize(5);
            }
        }

        iterationsWithoutImprovement++;
        if (iterationsWithoutImprovement >= maxIterationsWithoutImprovement) {
            std::cout << "Arrêt précoce: pas d'amélioration depuis "
                << iterationsWithoutImprovement << " générations" << std::endl;
            break;
        }
    }

    // Mettre à jour les métriques de performance
    double optimalLength = std::abs(maze->endPos.x - maze->startPos.x) +
        std::abs(maze->endPos.y - maze->startPos.y);
    updatePerformanceMetrics(!bestPath.empty(), bestLength, optimalLength, maxGenerations);

    // Méta-apprentissage à partir de cette expérience
    metaLearnFromExperience();

    std::cout << "=== Fin de la recherche évolutive ===" << std::endl;
    std::cout << "Longueur du chemin: " << bestPath.size() << " étapes" << std::endl;
    std::cout << "Poids finaux: G=" << weightG << " H=" << weightH << " Q=" << weightQ << std::endl;
    std::cout << "Score d'adaptabilité: " << adaptabilityScore << std::endl;

    return bestPath;
}

void EvolutionaryAStar::adaptWeightsBasedOnPerformance(bool pathFound, double pathLength, double optimalLength) {
    if (!pathFound) {
        // Échec: augmenter l'exploration (poids Q)
        weightQ = std::min(maxWeight, weightQ + adaptationRate * 0.5);
        weightG = std::max(minWeight, weightG - adaptationRate * 0.25);
        weightH = std::max(minWeight, weightH - adaptationRate * 0.25);

        std::cout << "  Adaptation (échec): Augmentation de Q à " << weightQ << std::endl;
        return;
    }

    // Calcul du ratio d'optimalité
    double optimalityRatio = optimalLength / pathLength;

    if (optimalityRatio < 0.8) {
        // Performance médiocre: rééquilibrer les poids
        weightG = std::min(maxWeight, weightG + adaptationRate * 0.3);
        weightH = std::max(minWeight, weightH - adaptationRate * 0.15);
        weightQ = std::max(minWeight, weightQ - adaptationRate * 0.15);

        std::cout << "  Adaptation (médiocre): G=" << weightG << " H=" << weightH
            << " Q=" << weightQ << " Ratio=" << optimalityRatio << std::endl;
    }
    else if (optimalityRatio > 0.95) {
        // Performance excellente: augmenter la confiance dans Q
        weightQ = std::min(maxWeight, weightQ + adaptationRate * 0.2);
        weightG = std::max(minWeight, weightG - adaptationRate * 0.1);

        std::cout << "  Adaptation (excellente): Confiance en Q augmentée à " << weightQ
            << " Ratio=" << optimalityRatio << std::endl;
    }

    // Normalisation pour que la somme = 1
    double total = weightG + weightH + weightQ;
    weightG /= total;
    weightH /= total;
    weightQ /= total;

    // Mettre à jour le score d'adaptabilité
    adaptabilityScore = 0.7 * adaptabilityScore + 0.3 * optimalityRatio;
    adaptabilityScore = std::min(1.0, std::max(0.0, adaptabilityScore));
}

void EvolutionaryAStar::updateConfidenceMap(const Point& pos, double confidence) {
    confidenceMap[pos] = 0.9 * confidenceMap[pos] + 0.1 * confidence;

    // Propagation locale de la confiance
    Point directions[4] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };
    for (const auto& dir : directions) {
        Point neighbor = { pos.x + dir.x, pos.y + dir.y };
        confidenceMap[neighbor] = 0.95 * confidenceMap[neighbor] + 0.05 * confidence;
    }
}

std::vector<std::vector<Point>> EvolutionaryAStar::generateAlternativePaths(Maze* maze, int numPaths) {
    std::vector<std::vector<Point>> paths;

    // Chemin 1: A* traditionnel
    PathFinder traditionalAStar;
    auto path1 = traditionalAStar.findPath(maze);
    if (!path1.empty()) paths.push_back(path1);

    // Chemin 2: A* avec heuristique apprise
    std::vector<Point> path2;
    if (deepLearner) {
        // Implémenter une recherche guidée par le modèle appris
        std::priority_queue<HybridNode, std::vector<HybridNode>, std::greater<HybridNode>> open;
        std::unordered_map<Point, Point, PointHash> cameFrom;
        std::unordered_map<Point, double, PointHash> gScores;

        gScores[maze->startPos] = 0;
        double qValue = deepLearner->getQValue(maze->startPos, 0); // Approximation
        double hValue = traditionalHeuristic->calculate(maze->startPos, maze->endPos);

        HybridNode startNode{ maze->startPos, 0, hValue, qValue, calculateHybridF(0, hValue, qValue) };
        open.push(startNode);

        Point directions[4] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

        while (!open.empty() && path2.size() < 100) { // Limite de sécurité
            HybridNode current = open.top();
            open.pop();

            if (current.pos == maze->endPos) {
                // Reconstruction du chemin
                Point p = current.pos;
                path2.push_back(p);
                while (cameFrom.find(p) != cameFrom.end()) {
                    p = cameFrom[p];
                    path2.push_back(p);
                }
                std::reverse(path2.begin(), path2.end());
                break;
            }

            for (const auto& dir : directions) {
                Point neighbor = { current.pos.x + dir.x, current.pos.y + dir.y };

                if (!maze->isValid(neighbor) || maze->isWall(neighbor)) continue;

                double tentativeG = current.g + 1.0;

                if (gScores.find(neighbor) == gScores.end() || tentativeG < gScores[neighbor]) {
                    cameFrom[neighbor] = current.pos;
                    gScores[neighbor] = tentativeG;

                    double h = traditionalHeuristic->calculate(neighbor, maze->endPos);
                    double q = getLearnedHeuristic(neighbor, maze->endPos, maze);
                    double f = calculateHybridF(tentativeG, h, q);

                    open.push({ neighbor, tentativeG, h, q, f });
                }
            }
        }

        if (!path2.empty()) paths.push_back(path2);
    }

    // Chemins 3-N: Variations aléatoires guidées
    for (int i = 2; i < numPaths; ++i) {
        if (!path1.empty()) {
            auto mutated = mutatePath(path1, 0.2 + 0.1 * i);
            if (!mutated.empty()) paths.push_back(mutated);
        }
    }

    return paths;
}

std::vector<Point> EvolutionaryAStar::crossoverPaths(const std::vector<Point>& path1,
    const std::vector<Point>& path2) {
    if (path1.empty() || path2.empty() || path1[0] != path2[0] || path1.back() != path2.back()) {
        return {};
    }

    // Trouver un point de croisement commun
    std::vector<Point> commonPoints;
    for (const auto& p1 : path1) {
        if (std::find(path2.begin(), path2.end(), p1) != path2.end()) {
            commonPoints.push_back(p1);
        }
    }

    if (commonPoints.size() < 3) return {}; // Pas assez de points communs

    // Choisir un point de croisement aléatoire (pas le début ni la fin)
    int crossoverIndex = rand() % (commonPoints.size() - 2) + 1;
    Point crossoverPoint = commonPoints[crossoverIndex];

    // Trouver les indices dans les deux chemins
    auto it1 = std::find(path1.begin(), path1.end(), crossoverPoint);
    auto it2 = std::find(path2.begin(), path2.end(), crossoverPoint);

    if (it1 == path1.end() || it2 == path2.end()) return {};

    int idx1 = std::distance(path1.begin(), it1);
    int idx2 = std::distance(path2.begin(), it2);

    // Créer le chemin enfant: début de path1 jusqu'au point de croisement, puis fin de path2
    std::vector<Point> child(path1.begin(), path1.begin() + idx1 + 1);
    child.insert(child.end(), path2.begin() + idx2 + 1, path2.end());

    // Éliminer les boucles potentielles
    std::vector<Point> cleaned;
    std::unordered_set<Point, PointHash> visited;

    for (const auto& p : child) {
        if (visited.find(p) == visited.end()) {
            cleaned.push_back(p);
            visited.insert(p);
        }
    }

    return cleaned;
}

std::vector<Point> EvolutionaryAStar::mutatePath(const std::vector<Point>& path, double mutationRate) {
    if (path.empty() || mutationRate <= 0) return path;

    std::vector<Point> mutated = path;

    // Types de mutation possibles
    int mutationType = rand() % 3;

    switch (mutationType) {
    case 0: { // Mutation par déplacement local
        if (mutated.size() > 3) {
            int mutateIndex = rand() % (mutated.size() - 2) + 1;

            // Essayer de trouver un chemin alternatif local
            Point prev = mutated[mutateIndex - 1];
            Point next = mutated[mutateIndex + 1];

            // Points adjacents possibles
            std::vector<Point> candidates = {
                {prev.x + 1, prev.y}, {prev.x - 1, prev.y},
                {prev.x, prev.y + 1}, {prev.x, prev.y - 1}
            };

            for (const auto& candidate : candidates) {
                // Vérifier si le candidat mène à next en 2 étapes
                if ((candidate.x == next.x && std::abs(candidate.y - next.y) == 1) ||
                    (candidate.y == next.y && std::abs(candidate.x - next.x) == 1)) {
                    mutated[mutateIndex] = candidate;
                    break;
                }
            }
        }
        break;
    }

    case 1: { // Mutation par inversion de segment
        if (mutated.size() > 4) {
            int start = rand() % (mutated.size() - 3);
            int end = start + rand() % (mutated.size() - start - 2) + 2;

            std::reverse(mutated.begin() + start, mutated.begin() + end);
        }
        break;
    }

    case 2: { // Mutation par ajout/détour
        if (mutated.size() > 2 && rand() % 100 < mutationRate * 100) {
            int insertIndex = rand() % (mutated.size() - 1) + 1;
            Point before = mutated[insertIndex - 1];
            Point after = mutated[insertIndex];

            // Créer un petit détour
            if (before.x == after.x) { // Vertical
                int midX = before.x + (rand() % 3 - 1); // -1, 0, ou 1
                mutated.insert(mutated.begin() + insertIndex, Point{ midX, before.y });
                mutated.insert(mutated.begin() + insertIndex + 1, Point{ midX, after.y });
            }
            else if (before.y == after.y) { // Horizontal
                int midY = before.y + (rand() % 3 - 1);
                mutated.insert(mutated.begin() + insertIndex, Point{ before.x, midY });
                mutated.insert(mutated.begin() + insertIndex + 1, Point{ after.x, midY });
            }
        }
        break;
    }
    }

    return mutated;
}

void EvolutionaryAStar::metaLearnFromExperience() {
    if (performanceHistory.empty()) return;

    // Analyser l'historique des performances
    double avgSuccessRate = 0.0;
    double avgAdaptationSpeed = 0.0;

    for (const auto& perf : performanceHistory) {
        avgSuccessRate += perf.first;
        avgAdaptationSpeed += perf.second;
    }

    avgSuccessRate /= performanceHistory.size();
    avgAdaptationSpeed /= performanceHistory.size();

    // Ajuster le taux d'adaptation en fonction des performances
    if (avgSuccessRate > 0.8 && avgAdaptationSpeed > 0.7) {
        // Bonnes performances: augmenter l'agressivité de l'adaptation
        adaptationRate = std::min(0.5, adaptationRate * 1.1);
        std::cout << "Meta-learning: Augmentation du taux d'adaptation à " << adaptationRate << std::endl;
    }
    else if (avgSuccessRate < 0.5 || avgAdaptationSpeed < 0.3) {
        // Mauvaises performances: être plus conservateur
        adaptationRate = std::max(0.05, adaptationRate * 0.9);
        std::cout << "Meta-learning: Réduction du taux d'adaptation à " << adaptationRate << std::endl;
    }

    // Mettre à jour les limites des poids
    maxWeight = std::min(0.9, 0.7 + adaptabilityScore * 0.2);
    minWeight = std::max(0.05, 0.1 - adaptabilityScore * 0.05);
}

double EvolutionaryAStar::predictOptimalWeights(const std::vector<double>& mazeFeatures) {
    // Cette méthode utiliserait un modèle de méta-apprentissage pour prédire
    // les poids optimaux en fonction des caractéristiques du labyrinthe
    // Pour l'instant, retourne une prédiction basée sur l'adaptabilité

    return adaptabilityScore;
}

double EvolutionaryAStar::calculateHybridF(double g, double h, double q) const {
    return weightG * g + weightH * h + weightQ * (1.0 - q); // Inverser q car valeur Q élevée = bonne
}

double EvolutionaryAStar::getLearnedHeuristic(const Point& current, const Point& goal, Maze* maze) {
    if (!deepLearner || !maze) return 0.5; // Valeur par défaut

    // Utiliser la confiance mappée si disponible
    auto it = confidenceMap.find(current);
    if (it != confidenceMap.end()) {
        return it->second;
    }

    // Sinon, utiliser une estimation basée sur la distance et l'apprentissage
    double baseConfidence = deepLearner->getLearningScore() / 100.0;
    double distanceRatio = traditionalHeuristic->calculate(current, goal) /
        (maze->width + maze->height);

    return baseConfidence * (1.0 - distanceRatio * 0.5);
}

void EvolutionaryAStar::updatePerformanceMetrics(bool success, double actualLength,
    double optimalLength, int iterations) {
    if (success && optimalLength > 0) {
        double optimality = optimalLength / actualLength;
        optimalityRate = 0.9 * optimalityRate + 0.1 * optimality;

        double speed = 1.0 / (iterations + 1); // Plus d'itérations = plus lent
        convergenceSpeed = 0.9 * convergenceSpeed + 0.1 * speed;

        performanceHistory.push_back({ optimality, speed });
        if (performanceHistory.size() > 100) {
            performanceHistory.erase(performanceHistory.begin());
        }
    }
}

std::vector<double> EvolutionaryAStar::extractMazeFeatures(Maze* maze) const {
    std::vector<double> features;

    if (!maze) return features;

    // 1. Taille du labyrinthe
    features.push_back(maze->width / 30.0);  // Normalisé
    features.push_back(maze->height / 30.0);

    // 2. Densité des murs
    int wallCount = 0;
    for (int y = 0; y < maze->height; ++y) {
        for (int x = 0; x < maze->width; ++x) {
            if (maze->isWall({ x, y })) wallCount++;
        }
    }
    features.push_back(static_cast<double>(wallCount) / (maze->width * maze->height));

    // 3. Distance entre départ et arrivée
    double distance = std::abs(maze->endPos.x - maze->startPos.x) +
        std::abs(maze->endPos.y - maze->startPos.y);
    features.push_back(distance / (maze->width + maze->height));

    // 4. Complexité estimée (nombre de branches)
    // Simplification: utiliser la densité des murs comme proxy

    return features;
}

void EvolutionaryAStar::printStatistics() const {
    std::cout << "\n=== Statistiques A* Évolutif ===" << std::endl;
    std::cout << "Score d'adaptabilité: " << adaptabilityScore << std::endl;
    std::cout << "Taux d'optimalité: " << optimalityRate << std::endl;
    std::cout << "Vitesse de convergence: " << convergenceSpeed << std::endl;
    std::cout << "Poids actuels: G=" << weightG << " H=" << weightH << " Q=" << weightQ << std::endl;
    std::cout << "Taille de la carte de confiance: " << confidenceMap.size() << std::endl;
    std::cout << "Historique des performances: " << performanceHistory.size() << " entrées" << std::endl;
}

bool EvolutionaryAStar::saveEvolutionaryModel(const std::string& filename) const {
    std::ofstream file(filename + "_evolutionary.dat", std::ios::binary);
    if (!file.is_open()) return false;

    // Sauvegarder les poids et paramètres
    file.write(reinterpret_cast<const char*>(&weightG), sizeof(weightG));
    file.write(reinterpret_cast<const char*>(&weightH), sizeof(weightH));
    file.write(reinterpret_cast<const char*>(&weightQ), sizeof(weightQ));
    file.write(reinterpret_cast<const char*>(&adaptationRate), sizeof(adaptationRate));
    file.write(reinterpret_cast<const char*>(&adaptabilityScore), sizeof(adaptabilityScore));
    file.write(reinterpret_cast<const char*>(&optimalityRate), sizeof(optimalityRate));
    file.write(reinterpret_cast<const char*>(&convergenceSpeed), sizeof(convergenceSpeed));

    // Sauvegarder la carte de confiance
    size_t mapSize = confidenceMap.size();
    file.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    for (const auto& entry : confidenceMap) {
        file.write(reinterpret_cast<const char*>(&entry.first.x), sizeof(entry.first.x));
        file.write(reinterpret_cast<const char*>(&entry.first.y), sizeof(entry.first.y));
        file.write(reinterpret_cast<const char*>(&entry.second), sizeof(entry.second));
    }

    // Sauvegarder l'historique des performances
    size_t historySize = performanceHistory.size();
    file.write(reinterpret_cast<const char*>(&historySize), sizeof(historySize));

    for (const auto& perf : performanceHistory) {
        file.write(reinterpret_cast<const char*>(&perf.first), sizeof(perf.first));
        file.write(reinterpret_cast<const char*>(&perf.second), sizeof(perf.second));
    }

    file.close();

    // Sauvegarder également le modèle d'apprentissage profond
    if (deepLearner) {
        deepLearner->saveModel(filename + "_deepq");
    }

    return true;
}

bool EvolutionaryAStar::loadEvolutionaryModel(const std::string& filename) {
    std::ifstream file(filename + "_evolutionary.dat", std::ios::binary);
    if (!file.is_open()) return false;

    // Charger les poids et paramètres
    file.read(reinterpret_cast<char*>(&weightG), sizeof(weightG));
    file.read(reinterpret_cast<char*>(&weightH), sizeof(weightH));
    file.read(reinterpret_cast<char*>(&weightQ), sizeof(weightQ));
    file.read(reinterpret_cast<char*>(&adaptationRate), sizeof(adaptationRate));
    file.read(reinterpret_cast<char*>(&adaptabilityScore), sizeof(adaptabilityScore));
    file.read(reinterpret_cast<char*>(&optimalityRate), sizeof(optimalityRate));
    file.read(reinterpret_cast<char*>(&convergenceSpeed), sizeof(convergenceSpeed));

    // Charger la carte de confiance
    size_t mapSize;
    file.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));

    confidenceMap.clear();
    for (size_t i = 0; i < mapSize; ++i) {
        Point p;
        double confidence;
        file.read(reinterpret_cast<char*>(&p.x), sizeof(p.x));
        file.read(reinterpret_cast<char*>(&p.y), sizeof(p.y));
        file.read(reinterpret_cast<char*>(&confidence), sizeof(confidence));
        confidenceMap[p] = confidence;
    }

    // Charger l'historique des performances
    size_t historySize;
    file.read(reinterpret_cast<char*>(&historySize), sizeof(historySize));

    performanceHistory.clear();
    for (size_t i = 0; i < historySize; ++i) {
        double success, speed;
        file.read(reinterpret_cast<char*>(&success), sizeof(success));
        file.read(reinterpret_cast<char*>(&speed), sizeof(speed));
        performanceHistory.push_back({ success, speed });
    }

    file.close();

    // Charger le modèle d'apprentissage profond
    if (deepLearner) {
        deepLearner->loadModel(filename + "_deepq");
    }

    return true;
}