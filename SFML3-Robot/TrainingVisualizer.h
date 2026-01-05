#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <deque>
#include <string>

class TrainingVisualizer {
private:
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

public:
    TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f);

    void update(double loss, double reward, double successRate, int trainingSteps);
    void draw();

    void setPosition(const sf::Vector2f& position);
    void setSize(float width, float height);

    void clearHistory();

private:
    void updateCurves();
    double getMaxValue(const std::deque<double>& history) const;
    double getMinValue(const std::deque<double>& history) const;
};