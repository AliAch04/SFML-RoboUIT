#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <deque>
#include <string>
#include <memory>

class EvolutionaryAStar;
class MetaLearner;

class PerformanceDashboard {
private:
    sf::RenderWindow& window;
    sf::Font font;

    // Composants du dashboard
    sf::RectangleShape mainPanel;
    sf::Text titleText;

    // Graphiques
    sf::VertexArray optimalityGraph;
    sf::VertexArray convergenceGraph;
    sf::VertexArray adaptabilityGraph;

    // Métriques en temps réel
    sf::Text optimalityText;
    sf::Text convergenceText;
    sf::Text adaptabilityText;
    sf::Text weightsText;
    sf::Text generationText;
    sf::Text strategyText;

    // Historique des performances
    std::deque<double> optimalityHistory;
    std::deque<double> convergenceHistory;
    std::deque<double> adaptabilityHistory;

    // Configuration
    int maxHistorySize;
    sf::Vector2f position;
    float width;
    float height;

    // Références aux algorithmes
    const EvolutionaryAStar* evolutionaryAStar;
    const MetaLearner* metaLearner;

public:
    PerformanceDashboard(sf::RenderWindow& win, const sf::Font& f);

    void setAlgorithms(const EvolutionaryAStar* evoAStar, const MetaLearner* meta);
    void updateMetrics();
    void draw();

    void setPosition(const sf::Vector2f& pos);
    void setSize(float w, float h);

    void addPerformanceData(double optimality, double convergence, double adaptability);
    void clearHistory();

    void setGenerationInfo(int currentGen, int totalGens, const std::string& strategy);

private:
    void updateGraphs();
    void updateTexts();
    sf::Color getPerformanceColor(double value) const;
    std::string formatDouble(double value, int precision = 2) const;
};