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

    // Add these members
    bool isPanelMode;
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    sf::RectangleShape panelBackground;
    sf::Text panelTitle;

    // Hide/Show button on the panel
    sf::RectangleShape hideShowButton;
    sf::Text hideShowButtonText;

    // Training curves (basic only)
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
    sf::Text infoText;

    // Title
    sf::Text titleText;

    // Configuration
    int maxHistorySize;
    float graphWidth;
    float graphHeight;
    sf::Vector2f graphPosition;

public:
    TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f);

    void setPanelMode(bool panelMode);
    void setPanelPosition(const sf::Vector2f& position);
    void setPanelSize(const sf::Vector2f& size);
    void handlePanelEvents(const sf::Vector2f& mousePos, bool mouseClicked);

    // Simplified update method
    void update(double loss, double reward, double successRate, int trainingSteps);
    void draw();

    // Visibility control
    void setVisible(bool visible);
    bool isDashboardVisible() const;
    void toggleVisibility();

    void setPosition(const sf::Vector2f& position);
    void setSize(float width, float height);
    void clearHistory();

private:
    void updateCurves();
    double getMaxValue(const std::deque<double>& history) const;
    double getMinValue(const std::deque<double>& history) const;
};