#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <deque>
#include <string>

class TrainingVisualizer {
private:
    bool isVisible = true;
    sf::RenderWindow& window;
    sf::Font font;

    // Training curves
    std::deque<double> lossHistory;
    std::deque<double> rewardHistory;
    std::deque<double> successRateHistory;

    // Display elements
    sf::RectangleShape graphBackground;
    sf::VertexArray lossCurve;
    sf::VertexArray rewardCurve;
    sf::VertexArray successCurve;

    sf::Text lossText;
    sf::Text rewardText;
    sf::Text successText;
    sf::Text trainingStepsText;

    // Configuration
    int maxHistorySize;
    float graphWidth;
    float graphHeight;
    sf::Vector2f graphPosition;

    // Add new metrics
    std::deque<double> qValueHistory;
    std::deque<double> explorationRateHistory;
    std::deque<int> stepsPerEpisodeHistory;

    // Add new curves
    sf::VertexArray qValueCurve;
    sf::VertexArray explorationCurve;
    sf::VertexArray stepsCurve;

    // Add new text displays
    sf::Text qValueText;
    sf::Text explorationText;
    sf::Text stepsText;
    sf::Text episodeText;
    sf::Text learningRateText;
    sf::Text epsilonText;

    // Add title
    sf::Text titleText;

    // Add legends
    std::vector<sf::Text> legendItems;

public:
    TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f);

    void update(double loss, double reward, double successRate,
        int trainingSteps, int currentEpisode = 0,
        double avgQValue = 0.0, double explorationRate = 0.0,
        double learningRate = 0.0, double epsilon = 0.0,
        int stepsThisEpisode = 0);
    void draw();

    // Visibility control
    void setVisible(bool visible);
    bool isDashboardVisible() const;
    void toggleVisibility();

    void createLegend();

    void setPosition(const sf::Vector2f& position);
    void setSize(float width, float height);

    void clearHistory();

private:
    void updateCurves();
    double getMaxValue(const std::deque<double>& history) const;
    double getMinValue(const std::deque<double>& history) const;
};