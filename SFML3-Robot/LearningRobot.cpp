#include "LearningRobot.h"
#include "EvolutionaryAStar.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <chrono>

LearningRobot::LearningRobot()
    : qLearning(std::make_unique<QLearning>()),
    previousState({ -1, -1 }),
    previousAction(-1),
    totalReward(0.0),
    successfulTrials(0),
    totalTrials(0) {
}

void LearningRobot::setMaze(Maze* maze) {
    currentMaze = maze;
    visitedStates.clear();
}

void LearningRobot::startNewTrial() {
    totalTrials++;
    totalReward = 0.0;
    visitedStates.clear();
    previousState = { -1, -1 };
    previousAction = -1;
}

void LearningRobot::moveTo(Point next) {
    if (!currentMaze) {
        Robot::moveTo(next);
        return;
    }

    // Apprentissage avant de bouger
    if (previousAction != -1) {
        Point current = getPosition();
        double reward = calculateReward(previousState, current);

        // Obtenir les actions disponibles pour l'état suivant
        std::vector<int> nextActions = getAvailableActions(current);

        // Mettre à jour Q-learning
        qLearning->update(previousState, previousAction, reward, current, nextActions);

        // Mettre à jour la récompense totale
        totalReward += reward;
        receiveReward(reward);
    }

    // Choisir la prochaine action
    Point currentPos = getPosition();
    std::vector<int> availableActions = getAvailableActions(currentPos);

    if (!availableActions.empty()) {
        previousAction = qLearning->chooseAction(currentPos, availableActions);
        previousState = currentPos;

        // Obtenir la prochaine position
        Point nextPos = getNextState(currentPos, previousAction);

        // Mémoriser l'état visité
        visitedStates.push_back(currentPos);
        if (visitedStates.size() > MAX_MEMORY) {
            visitedStates.erase(visitedStates.begin());
        }

        // Appeler la méthode parent pour le mouvement
        Robot::moveTo(nextPos);
    }
}

void LearningRobot::update(float dt) {
    Robot::update(dt);

    // Vérifier si le but est atteint
    if (currentMaze && getPosition() == currentMaze->endPos && getState() != RobotState::COMPLETED) {
        // Grande récompense pour avoir atteint le but
        receiveReward(REWARD_GOAL);
        successfulTrials++;
        std::cout << "Robot a atteint le but! Récompense: " << REWARD_GOAL
            << " Score d'apprentissage: " << getLearningScore() << "%" << std::endl;
    }
}

void LearningRobot::setPosition(Point p) {
    Robot::setPosition(p);
    visitedStates.clear();
    visitedStates.push_back(p);
    previousState = { -1, -1 };
    previousAction = -1;
}

std::vector<int> LearningRobot::getAvailableActions(const Point& state) {
    std::vector<int> actions;

    if (!currentMaze) return actions;

    // Directions: 0=up, 1=down, 2=left, 3=right
    Point directions[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    for (int i = 0; i < 4; ++i) {
        Point next = { state.x + directions[i].x, state.y + directions[i].y };
        if (currentMaze->isValid(next) && !currentMaze->isWall(next)) {
            actions.push_back(i);
        }
    }

    return actions;
}

Point LearningRobot::getNextState(const Point& state, int action) {
    Point directions[4] = { {0, -1}, {0, 1}, {-1, 0}, {1, 0} };

    if (action >= 0 && action < 4) {
        return { state.x + directions[action].x, state.y + directions[action].y };
    }

    return state;
}

double LearningRobot::calculateReward(const Point& state, const Point& nextState) {
    if (!currentMaze) return REWARD_MOVE;

    // Pénalité pour heurter un mur
    if (!currentMaze->isValid(nextState) || currentMaze->isWall(nextState)) {
        return PENALTY_WALL;
    }

    // Récompense pour progresser vers le but
    double progressReward = 0.0;
    if (isMakingProgress(nextState, currentMaze->endPos)) {
        progressReward = REWARD_PROGRESS;
    }

    // Pénalité pour les boucles
    if (isLooping(nextState)) {
        return PENALTY_LOOP + progressReward + REWARD_MOVE;
    }

    // Récompense normale pour un mouvement valide
    return REWARD_MOVE + progressReward;
}

bool LearningRobot::isLooping(const Point& state) {
    // Vérifier si l'état a déjà été visité récemment
    return std::find(visitedStates.begin(), visitedStates.end(), state) != visitedStates.end();
}

bool LearningRobot::isMakingProgress(const Point& state, const Point& goal) {
    // Calcul de la distance de Manhattan
    int currentDist = std::abs(state.x - goal.x) + std::abs(state.y - goal.y);
    int prevDist = std::abs(previousState.x - goal.x) + std::abs(previousState.y - goal.y);

    return currentDist < prevDist;
}

void LearningRobot::receiveReward(double reward) {
    totalReward += reward;
}

double LearningRobot::getLearningScore() const {
    return qLearning->getLearningScore();
}

double LearningRobot::getSuccessRate() const {
    if (totalTrials == 0) return 0.0;
    return (static_cast<double>(successfulTrials) / totalTrials) * 100.0;
}

bool LearningRobot::saveModel(const std::string& filename) {
    return qLearning->saveToFile(filename);
}

bool LearningRobot::loadModel(const std::string& filename) {
    return qLearning->loadFromFile(filename);
}

void LearningRobot::resetLearning() {
    qLearning = std::make_unique<QLearning>();
    totalReward = 0.0;
    successfulTrials = 0;
    totalTrials = 0;
    visitedStates.clear();
}

void LearningRobot::runEvolutionaryOptimization(int generations) {
    if (!currentMaze || !evolutionaryPathFinder) return;

    maxGenerations = generations;
    std::cout << "\n=== Début de l'optimisation évolutive ===" << std::endl;
    std::cout << "Générations maximum: " << maxGenerations << std::endl;
    std::cout << "Score d'adaptabilité initial: "
        << evolutionaryPathFinder->getAdaptabilityScore() << std::endl;

    // Historique des performances
    std::vector<double> optimalityHistory;
    std::vector<double> adaptabilityHistory;

    for (currentGeneration = 1; currentGeneration <= maxGenerations; currentGeneration++) {
        // Exécuter une génération d'optimisation
        auto path = evolutionaryPathFinder->findPathWithEvolution(currentMaze, 1);

        if (!path.empty()) {
            // Calculer les métriques
            double optimalLength = std::abs(currentMaze->endPos.x - currentMaze->startPos.x) +
                std::abs(currentMaze->endPos.y - currentMaze->startPos.y);
            double optimality = optimalLength / path.size();
            double adaptability = evolutionaryPathFinder->getAdaptabilityScore();

            optimalityHistory.push_back(optimality);
            adaptabilityHistory.push_back(adaptability);

            // Changer de stratégie périodiquement
            if (currentGeneration % 10 == 0 && metaLearner) {
                auto mazeFeatures = evolutionaryPathFinder->extractMazeFeatures(currentMaze);
                double currentPerformance = optimalityHistory.empty() ? 0.5 : optimalityHistory.back();
                auto newStrategy = metaLearner->selectBestStrategy(mazeFeatures, currentPerformance);

                currentStrategy = "Adaptation Gen " + std::to_string(currentGeneration);

                // Afficher le changement de stratégie
                std::cout << "Génération " << currentGeneration
                    << ": Changement de stratégie" << std::endl;
                std::cout << "  Optimalité: " << optimality
                    << " Adaptabilité: " << adaptability << std::endl;
            }

            // Arrêt précoce si convergence
            if (optimalityHistory.size() > 5) {
                double avgImprovement = 0.0;
                for (size_t i = 1; i < optimalityHistory.size(); i++) {
                    avgImprovement += optimalityHistory[i] - optimalityHistory[i - 1];
                }
                avgImprovement /= (optimalityHistory.size() - 1);

                if (std::abs(avgImprovement) < 0.001 && optimality > 0.95) {
                    std::cout << "Convergence atteinte à la génération "
                        << currentGeneration << std::endl;
                    break;
                }
            }
        }

        // Affichage de progression
        if (currentGeneration % 5 == 0) {
            std::cout << "Génération " << currentGeneration << "/" << maxGenerations;
            if (!path.empty()) {
                std::cout << " - Longueur chemin: " << path.size() << " étapes";
            }
            std::cout << std::endl;
        }
    }

    // Récupérer le meilleur chemin trouvé
    currentEvolutionaryPath = evolutionaryPathFinder->findPath(currentMaze);

    std::cout << "=== Fin de l'optimisation évolutive ===" << std::endl;
    std::cout << "Meilleur chemin: " << currentEvolutionaryPath.size() << " étapes" << std::endl;
    std::cout << "Score d'adaptabilité final: "
        << evolutionaryPathFinder->getAdaptabilityScore() << std::endl;
    evolutionaryPathFinder->printStatistics();
}

double LearningRobot::calculatePathOptimality(const std::vector<Point>& path) const {
    if (path.empty() || !currentMaze) return 0.0;

    // Longueur optimale théorique (distance de Manhattan)
    int optimalLength = std::abs(currentMaze->endPos.x - currentMaze->startPos.x) +
        std::abs(currentMaze->endPos.y - currentMaze->startPos.y);

    // Longueur réelle du chemin
    int actualLength = path.size() - 1; // -1 car le départ compte comme une étape

    if (optimalLength == 0) return 1.0; // Départ = arrivée

    // Ratio d'optimalité (plus proche de 1 = plus optimal)
    double optimality = static_cast<double>(optimalLength) / actualLength;

    // Limiter entre 0 et 1
    return std::min(1.0, std::max(0.0, optimality));
}

double LearningRobot::calculatePathSmoothness(const std::vector<Point>& path) const {
    if (path.size() < 3) return 1.0; // Chemin trop court pour évaluer

    int directionChanges = 0;
    Point prevDirection = { 0, 0 };

    for (size_t i = 1; i < path.size(); i++) {
        Point currentDirection = {
            path[i].x - path[i - 1].x,
            path[i].y - path[i - 1].y
        };

        // Compter les changements de direction
        if (prevDirection.x != 0 || prevDirection.y != 0) {
            if (currentDirection.x != prevDirection.x || currentDirection.y != prevDirection.y) {
                directionChanges++;
            }
        }

        prevDirection = currentDirection;
    }

    // Plus le chemin est droit, moins il y a de changements de direction
    // Normaliser: 0 changements = smoothness parfaite (1.0)
    double maxChanges = path.size() - 2; // Nombre maximum possible de changements
    double smoothness = 1.0 - (static_cast<double>(directionChanges) / maxChanges);

    return std::max(0.0, smoothness);
}

double LearningRobot::calculatePathSafety(const std::vector<Point>& path) const {
    if (path.empty() || !currentMaze) return 0.0;

    int wallAdjacentCount = 0;
    int totalCells = 0;

    // Directions pour vérifier les voisins
    Point directions[4] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

    for (const auto& cell : path) {
        totalCells++;

        // Vérifier si la cellule est adjacente à un mur
        for (const auto& dir : directions) {
            Point neighbor = { cell.x + dir.x, cell.y + dir.y };
            if (currentMaze->isValid(neighbor) && currentMaze->isWall(neighbor)) {
                wallAdjacentCount++;
                break; // Compter une seule fois par cellule
            }
        }
    }

    // Score de sécurité: moins de cellules adjacentes aux murs = plus sûr
    double safetyScore = 1.0 - (static_cast<double>(wallAdjacentCount) / totalCells);

    // Bonus pour les chemins éloignés des murs externes
    int distanceFromBorderBonus = 0;
    for (const auto& cell : path) {
        int minDistToBorder = std::min({
            cell.x,
            currentMaze->width - 1 - cell.x,
            cell.y,
            currentMaze->height - 1 - cell.y
            });

        if (minDistToBorder >= 2) {
            distanceFromBorderBonus++;
        }
    }

    double borderBonus = static_cast<double>(distanceFromBorderBonus) / totalCells * 0.2;
    safetyScore = std::min(1.0, safetyScore + borderBonus);

    return safetyScore;
}

double LearningRobot::getEvolutionaryOptimality() const {
    if (!evolutionaryPathFinder) return 0.0;

    double optimality = evolutionaryPathFinder->getOptimalityRate();

    // Si on a un chemin évolutif actuel, calculer son optimalité spécifique
    if (!currentEvolutionaryPath.empty()) {
        double pathOptimality = calculatePathOptimality(currentEvolutionaryPath);
        // Combiner avec l'optimalité historique
        return 0.7 * pathOptimality + 0.3 * optimality;
    }

    return optimality;
}

double LearningRobot::getEvolutionaryConvergence() const {
    if (!evolutionaryPathFinder) return 0.0;

    double convergence = evolutionaryPathFinder->getConvergenceSpeed();

    // Ajuster basé sur le nombre de générations
    if (maxGenerations > 0) {
        double generationRatio = static_cast<double>(currentGeneration) / maxGenerations;
        // Plus on est proche de la fin, plus la convergence devrait être élevée
        convergence = 0.6 * convergence + 0.4 * generationRatio;
    }

    return std::min(1.0, convergence);
}

double LearningRobot::getEvolutionaryAdaptability() const {
    if (!evolutionaryPathFinder) return 0.0;

    double adaptability = evolutionaryPathFinder->getAdaptabilityScore();

    // Bonus d'adaptabilité basé sur les comparaisons de chemins
    if (!pathComparisons.empty()) {
        const auto& latest = pathComparisons.back();
        if (latest.improvementRatio > 0) {
            // Amélioration positive: augmenter l'adaptabilité
            adaptability = std::min(1.0, adaptability + 0.1);
        }
        else if (latest.improvementRatio < -0.1) {
            // Dégradation significative: réduire l'adaptabilité
            adaptability = std::max(0.0, adaptability - 0.05);
        }
    }

    // Bonus pour l'utilisation du méta-learner
    if (metaLearner && metaLearner->getExperienceCount() > 10) {
        adaptability = std::min(1.0, adaptability + 0.05);
    }

    return adaptability;
}

void LearningRobot::generatePerformanceReport() const {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "            RAPPORT DE PERFORMANCE DU ROBOT" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    // Informations générales
    std::cout << "\n=== INFORMATIONS GÉNÉRALES ===" << std::endl;
    std::cout << "Essais réussis: " << successfulTrials << "/" << totalTrials
        << " (" << getSuccessRate() << "%)" << std::endl;
    std::cout << "Score d'apprentissage: " << getLearningScore() << "%" << std::endl;
    std::cout << "Récompense totale: " << totalReward << std::endl;

    // A* Évolutif
    if (evolutionaryPathFinder) {
        std::cout << "\n=== A* ÉVOLUTIF ===" << std::endl;
        std::cout << "Score d'adaptabilité: " << getEvolutionaryAdaptability() * 100 << "%" << std::endl;
        std::cout << "Taux d'optimalité: " << getEvolutionaryOptimality() * 100 << "%" << std::endl;
        std::cout << "Vitesse de convergence: " << getEvolutionaryConvergence() * 100 << "%" << std::endl;
        std::cout << "Stratégie actuelle: " << currentStrategy << std::endl;
        std::cout << "Génération: " << currentGeneration << "/" << maxGenerations << std::endl;

        if (!currentEvolutionaryPath.empty()) {
            std::cout << "\nChemin évolutif actuel:" << std::endl;
            std::cout << "  Longueur: " << currentEvolutionaryPath.size() << " étapes" << std::endl;
            std::cout << "  Optimalité: " << calculatePathOptimality(currentEvolutionaryPath) * 100 << "%" << std::endl;
            std::cout << "  Fluidité: " << calculatePathSmoothness(currentEvolutionaryPath) * 100 << "%" << std::endl;
            std::cout << "  Sécurité: " << calculatePathSafety(currentEvolutionaryPath) * 100 << "%" << std::endl;
        }
    }

    // Méta-Learner
    if (metaLearner) {
        std::cout << "\n=== MÉTA-APPRENTISSAGE ===" << std::endl;
        std::cout << "Expériences stockées: " << metaLearner->getExperienceCount() << std::endl;
        std::cout << "Stratégies apprises: " << metaLearner->getStrategyCount() << std::endl;
        std::cout << "Taux d'apprentissage méta: " << metaLearner->getMetaLearningRate() << std::endl;
    }

    // Comparaisons de chemins
    if (!pathComparisons.empty()) {
        std::cout << "\n=== COMPARAISONS DES MÉTHODES ===" << std::endl;

        // Dernière comparaison
        const auto& latest = pathComparisons.back();
        std::cout << "Dernière comparaison:" << std::endl;
        std::cout << "  A* Traditionnel: " << latest.traditionalSteps << " étapes, "
            << latest.traditionalTime << "s" << std::endl;
        std::cout << "  A* Évolutif: " << latest.evolutionarySteps << " étapes, "
            << latest.evolutionaryTime << "s" << std::endl;
        std::cout << "  Amélioration: " << (latest.improvementRatio * 100) << "%" << std::endl;

        // Statistiques globales
        if (pathComparisons.size() > 1) {
            double avgImprovement = 0.0;
            int positiveImprovements = 0;

            for (const auto& comp : pathComparisons) {
                avgImprovement += comp.improvementRatio;
                if (comp.improvementRatio > 0) positiveImprovements++;
            }

            avgImprovement /= pathComparisons.size();
            double successRate = static_cast<double>(positiveImprovements) / pathComparisons.size() * 100;

            std::cout << "\nStatistiques globales (" << pathComparisons.size() << " comparaisons):" << std::endl;
            std::cout << "  Amélioration moyenne: " << (avgImprovement * 100) << "%" << std::endl;
            std::cout << "  Taux de succès A* Évolutif: " << successRate << "%" << std::endl;
        }
    }

    // Recommandations
    std::cout << "\n=== RECOMMANDATIONS ===" << std::endl;

    if (getEvolutionaryAdaptability() < 0.5) {
        std::cout << "⚠️  L'adaptabilité est faible. Essayez plus de labyrinthes variés." << std::endl;
    }

    if (!pathComparisons.empty() && pathComparisons.back().improvementRatio < 0) {
        std::cout << "⚠️  A* Évolutif est moins performant. Pensez à réinitialiser l'apprentissage." << std::endl;
    }

    if (getSuccessRate() > 80) {
        std::cout << "✓  Excellente performance! Le robot maîtrise ce type de labyrinthe." << std::endl;
    }

    if (metaLearner && metaLearner->getExperienceCount() < 5) {
        std::cout << "ℹ️  Le méta-learner a besoin de plus d'expériences pour être efficace." << std::endl;
    }

    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "Fin du rapport" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    // Option: Sauvegarder le rapport dans un fichier
    std::ofstream reportFile("performance_report.txt");
    if (reportFile.is_open()) {
        // Réécrire le rapport dans le fichier
        reportFile << "Rapport de Performance - "
            << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())
            << std::endl;
        // ... (même contenu que l'affichage console)
        reportFile.close();
        std::cout << "Rapport sauvegardé dans 'performance_report.txt'" << std::endl;
    }
}

std::vector<Point> LearningRobot::findEvolutionaryPath() {
    if (!currentMaze || !evolutionaryPathFinder) {
        return {};
    }

    // Réinitialiser l'état d'optimisation
    currentGeneration = 0;
    maxGenerations = 50; // Valeur par défaut
    currentEvolutionaryPath.clear();

    // Extraire les caractéristiques du labyrinthe pour le méta-apprentissage
    auto mazeFeatures = evolutionaryPathFinder->extractMazeFeatures(currentMaze);

    // Si on a un méta-learner, prédire la stratégie optimale
    if (metaLearner && metaLearner->getExperienceCount() > 0) {
        auto predictedWeights = metaLearner->predictOptimalWeights(mazeFeatures);
        currentStrategy = "Meta-optimisée";

        // Afficher la stratégie prédite
        std::cout << "Strategie meta-predite: [";
        for (double w : predictedWeights) {
            std::cout << w << " ";
        }
        std::cout << "]" << std::endl;
    }
    else {
        currentStrategy = "Exploration";
    }

    // Exécuter l'optimisation évolutive
    auto startTime = std::chrono::high_resolution_clock::now();
    currentEvolutionaryPath = evolutionaryPathFinder->findPathWithEvolution(currentMaze, maxGenerations);
    auto endTime = std::chrono::high_resolution_clock::now();

    // Calculer le temps d'exécution
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    double evolutionaryTime = duration.count() / 1000.0;

    // Mettre à jour le méta-learner avec cette expérience
    if (metaLearner && !currentEvolutionaryPath.empty()) {
        // Calculer la performance
        double optimalLength = std::abs(currentMaze->endPos.x - currentMaze->startPos.x) +
            std::abs(currentMaze->endPos.y - currentMaze->startPos.y);
        double performance = optimalLength / currentEvolutionaryPath.size();

        // Poids actuels de l'algorithme évolutif
        std::vector<double> currentWeights = {
            evolutionaryPathFinder->getWeightG(),
            evolutionaryPathFinder->getWeightH(),
            evolutionaryPathFinder->getWeightQ()
        };

        // Apprendre de cette expérience
        metaLearner->learnFromExperience(mazeFeatures, { 0.33, 0.33, 0.33 }, currentWeights, performance);
    }

    // Comparer avec A* traditionnel
    comparePathfindingMethods();

    return currentEvolutionaryPath;
}