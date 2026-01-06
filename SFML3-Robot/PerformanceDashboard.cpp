#include "PerformanceDashboard.h"
#include "EvolutionaryAStar.h"
#include "MetaLearner.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

PerformanceDashboard::PerformanceDashboard(sf::RenderWindow& win, const sf::Font& f)
    : window(win), font(f), maxHistorySize(50), position(20.0f, 20.0f),
    width(350.0f), height(250.0f), evolutionaryAStar(nullptr), metaLearner(nullptr) {

    // Initialiser le panneau principal
    mainPanel.setSize(sf::Vector2f(width, height));
    mainPanel.setPosition(position);
    mainPanel.setFillColor(sf::Color(20, 20, 20, 220));
    mainPanel.setOutlineThickness(2);
    mainPanel.setOutlineColor(sf::Color::Cyan);

    // Titre
    titleText.setFont(font);
    titleText.setString("Dashboard A* Evolutif");
    titleText.setCharacterSize(18);
    titleText.setFillColor(sf::Color::Cyan);
    titleText.setPosition(position.x + 10, position.y + 10);

    // Initialiser les graphiques
    optimalityGraph.setPrimitiveType(sf::LineStrip);
    convergenceGraph.setPrimitiveType(sf::LineStrip);
    adaptabilityGraph.setPrimitiveType(sf::LineStrip);

    // Initialiser les textes
    optimalityText.setFont(font);
    optimalityText.setCharacterSize(12);
    optimalityText.setFillColor(sf::Color::Green);
    optimalityText.setPosition(position.x + 10, position.y + 40);

    convergenceText.setFont(font);
    convergenceText.setCharacterSize(12);
    convergenceText.setFillColor(sf::Color::Yellow);
    convergenceText.setPosition(position.x + 10, position.y + 60);

    adaptabilityText.setFont(font);
    adaptabilityText.setCharacterSize(12);
    adaptabilityText.setFillColor(sf::Color::Magenta);
    adaptabilityText.setPosition(position.x + 10, position.y + 80);

    weightsText.setFont(font);
    weightsText.setCharacterSize(12);
    weightsText.setFillColor(sf::Color::White);
    weightsText.setPosition(position.x + 10, position.y + 100);

    generationText.setFont(font);
    generationText.setCharacterSize(12);
    generationText.setFillColor(sf::Color::White);
    generationText.setPosition(position.x + 10, position.y + 120);

    strategyText.setFont(font);
    strategyText.setCharacterSize(10);
    strategyText.setFillColor(sf::Color::White);
    strategyText.setPosition(position.x + 10, position.y + 140);
}

void PerformanceDashboard::setAlgorithms(const EvolutionaryAStar* evoAStar, const MetaLearner* meta) {
    evolutionaryAStar = evoAStar;
    metaLearner = meta;
}

void PerformanceDashboard::updateMetrics() {
    if (evolutionaryAStar) {
        // Ajouter les métriques actuelles à l'historique
        optimalityHistory.push_back(evolutionaryAStar->getOptimalityRate());
        convergenceHistory.push_back(evolutionaryAStar->getConvergenceSpeed());
        adaptabilityHistory.push_back(evolutionaryAStar->getAdaptabilityScore());

        // Garder la taille limitée
        if (optimalityHistory.size() > static_cast<size_t>(maxHistorySize)) {
            optimalityHistory.pop_front();
            convergenceHistory.pop_front();
            adaptabilityHistory.pop_front();
        }

        updateGraphs();
    }

    updateTexts();
}

void PerformanceDashboard::updateGraphs() {
    optimalityGraph.clear();
    convergenceGraph.clear();
    adaptabilityGraph.clear();

    if (optimalityHistory.empty()) return;

    // Calculer les plages pour la normalisation
    float graphWidth = width - 150.0f;
    float graphHeight = 60.0f;
    float graphX = position.x + 180.0f;
    float graphY = position.y + 40.0f;

    float xStep = graphWidth / std::max(1.0f, static_cast<float>(optimalityHistory.size() - 1));

    for (size_t i = 0; i < optimalityHistory.size(); ++i) {
        float x = graphX + static_cast<float>(i) * xStep;

        // Graphique d'optimalité (vert) 
        float optY = graphY + graphHeight - static_cast<float>(optimalityHistory[i] * graphHeight);
        optimalityGraph.append(sf::Vertex(sf::Vector2f(x, optY), sf::Color::Green));

        // Graphique de convergence (jaune) 
        float convY = graphY + graphHeight - static_cast<float>(convergenceHistory[i] * graphHeight);
        convergenceGraph.append(sf::Vertex(sf::Vector2f(x, convY), sf::Color::Yellow));

        // Graphique d'adaptabilité (magenta) 
        float adaptY = graphY + graphHeight - static_cast<float>(adaptabilityHistory[i] * graphHeight);
        adaptabilityGraph.append(sf::Vertex(sf::Vector2f(x, adaptY), sf::Color::Magenta));
    }
}

void PerformanceDashboard::updateTexts() {
    std::stringstream ss;

    if (evolutionaryAStar) {
        // Optimalité
        ss << "Optimalite: " << formatDouble(evolutionaryAStar->getOptimalityRate() * 100) << "%";
        optimalityText.setString(ss.str());

        // Convergence
        ss.str("");
        ss << "Convergence: " << formatDouble(evolutionaryAStar->getConvergenceSpeed() * 100) << "%";
        convergenceText.setString(ss.str());

        // Adaptabilité
        ss.str("");
        ss << "Adaptabilite: " << formatDouble(evolutionaryAStar->getAdaptabilityScore() * 100) << "%";
        adaptabilityText.setString(ss.str());

        // Poids
        ss.str("");
        ss << "Poids: G=" << formatDouble(evolutionaryAStar->getWeightG(), 2)
            << " H=" << formatDouble(evolutionaryAStar->getWeightH(), 2)
            << " Q=" << formatDouble(evolutionaryAStar->getWeightQ(), 2);
        weightsText.setString(ss.str());
    }

    if (metaLearner) {
        ss.str("");
        ss << "Strategies: " << metaLearner->getStrategyCount()
            << "  Experiences: " << metaLearner->getExperienceCount();
        strategyText.setString(ss.str());
    }
}

void PerformanceDashboard::draw() {
    window.draw(mainPanel);
    window.draw(titleText);

    window.draw(optimalityGraph);
    window.draw(convergenceGraph);
    window.draw(adaptabilityGraph);

    window.draw(optimalityText);
    window.draw(convergenceText);
    window.draw(adaptabilityText);
    window.draw(weightsText);
    window.draw(generationText);
    window.draw(strategyText);
}

void PerformanceDashboard::setPosition(const sf::Vector2f& pos) {
    position = pos;
    mainPanel.setPosition(pos);
    titleText.setPosition(pos.x + 10, pos.y + 10);
    updateTexts();
    updateGraphs();
}

void PerformanceDashboard::setSize(float w, float h) {
    width = w;
    height = h;
    mainPanel.setSize(sf::Vector2f(w, h));
    updateGraphs();
}

void PerformanceDashboard::addPerformanceData(double optimality, double convergence, double adaptability) {
    optimalityHistory.push_back(optimality);
    convergenceHistory.push_back(convergence);
    adaptabilityHistory.push_back(adaptability);

    if (optimalityHistory.size() > static_cast<size_t>(maxHistorySize)) {
        optimalityHistory.pop_front();
        convergenceHistory.pop_front();
        adaptabilityHistory.pop_front();
    }

    updateGraphs();
}

void PerformanceDashboard::clearHistory() {
    optimalityHistory.clear();
    convergenceHistory.clear();
    adaptabilityHistory.clear();
    optimalityGraph.clear();
    convergenceGraph.clear();
    adaptabilityGraph.clear();
}

void PerformanceDashboard::setGenerationInfo(int currentGen, int totalGens, const std::string& strategy) {
    std::stringstream ss;
    ss << "Generation: " << currentGen << "/" << totalGens;
    generationText.setString(ss.str());

    ss.str("");
    ss << "Strategie: " << strategy;
    strategyText.setString(ss.str());
}

sf::Color PerformanceDashboard::getPerformanceColor(double value) const {
    if (value >= 0.8) return sf::Color::Green;
    if (value >= 0.6) return sf::Color::Yellow;
    if (value >= 0.4) return sf::Color(255, 165, 0); // Orange
    return sf::Color::Red;
}

std::string PerformanceDashboard::formatDouble(double value, int precision) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}