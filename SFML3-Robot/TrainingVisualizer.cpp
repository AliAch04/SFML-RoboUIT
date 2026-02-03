#include "TrainingVisualizer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

TrainingVisualizer::TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f)
    : window(win), font(f), maxHistorySize(50),
    graphWidth(300.0f), graphHeight(80.0f),
    isPanelMode(true), isVisible(true) {

    // Initialize panel with smaller size
    panelPosition = sf::Vector2f(620.0f, 80.0f);
    panelSize = sf::Vector2f(350.0f, 300.0f);

    // Panel background
    panelBackground.setSize(panelSize);
    panelBackground.setPosition(panelPosition);
    panelBackground.setFillColor(sf::Color(30, 30, 40, 240));
    panelBackground.setOutlineThickness(2);
    panelBackground.setOutlineColor(sf::Color(100, 100, 150));

    // Panel title
    panelTitle.setFont(font);
    panelTitle.setString("TRAINING DASHBOARD");
    panelTitle.setCharacterSize(16);
    panelTitle.setFillColor(sf::Color::Cyan);
    panelTitle.setStyle(sf::Text::Bold);
    panelTitle.setPosition(panelPosition.x + 10, panelPosition.y + 10);

    // Graph position inside panel
    graphPosition = sf::Vector2f(panelPosition.x + 15, panelPosition.y + 40);

    // Initialize graph background
    graphBackground.setSize(sf::Vector2f(graphWidth, graphHeight));
    graphBackground.setPosition(graphPosition);
    graphBackground.setFillColor(sf::Color(20, 20, 30, 220));
    graphBackground.setOutlineThickness(1);
    graphBackground.setOutlineColor(sf::Color(80, 80, 100));

    // Initialize curves
    lossCurve.setPrimitiveType(sf::LineStrip);
    rewardCurve.setPrimitiveType(sf::LineStrip);
    successCurve.setPrimitiveType(sf::LineStrip);

    // Initialize text displays
    float textY = graphPosition.y + graphHeight + 10;

    lossText.setFont(font);
    lossText.setCharacterSize(12);
    lossText.setFillColor(sf::Color::Red);
    lossText.setPosition(graphPosition.x + 5, textY);

    rewardText.setFont(font);
    rewardText.setCharacterSize(12);
    rewardText.setFillColor(sf::Color::Green);
    rewardText.setPosition(graphPosition.x + 110, textY);

    textY += 18;

    successText.setFont(font);
    successText.setCharacterSize(12);
    successText.setFillColor(sf::Color::Cyan);
    successText.setPosition(graphPosition.x + 5, textY);

    trainingStepsText.setFont(font);
    trainingStepsText.setCharacterSize(12);
    trainingStepsText.setFillColor(sf::Color::Yellow);
    trainingStepsText.setPosition(graphPosition.x + 110, textY);
}

void TrainingVisualizer::updateCurves() {
    // Clear all curves
    lossCurve.clear();
    rewardCurve.clear();
    successCurve.clear();

    if (lossHistory.empty()) return;

    // Calculate x step
    float xStep = graphWidth / std::max(1.0f, static_cast<float>(lossHistory.size() - 1));

    // Find max values for normalization
    double maxLoss = getMaxValue(lossHistory);
    double minLoss = getMinValue(lossHistory);
    double lossRange = std::max(0.1, maxLoss - minLoss);

    double maxReward = getMaxValue(rewardHistory);
    double minReward = getMinValue(rewardHistory);
    double rewardRange = std::max(0.1, maxReward - minReward);

    for (size_t i = 0; i < lossHistory.size(); ++i) {
        float x = graphPosition.x + static_cast<float>(i) * xStep;

        // Loss curve (simple normalization)
        if (i < lossHistory.size()) {
            float lossY = graphPosition.y + graphHeight -
                static_cast<float>((lossHistory[i] - minLoss) / lossRange * graphHeight);
            lossCurve.append(sf::Vertex(sf::Vector2f(x, lossY), sf::Color::Red));
        }

        // Reward curve
        if (i < rewardHistory.size()) {
            float rewardY = graphPosition.y + graphHeight -
                static_cast<float>((rewardHistory[i] - minReward) / rewardRange * graphHeight);
            rewardCurve.append(sf::Vertex(sf::Vector2f(x, rewardY), sf::Color::Green));
        }

        // Success rate curve
        if (i < successRateHistory.size()) {
            float successY = graphPosition.y + graphHeight -
                static_cast<float>(successRateHistory[i] / 100.0 * graphHeight);
            successCurve.append(sf::Vertex(sf::Vector2f(x, successY), sf::Color::Cyan));
        }
    }
}

void TrainingVisualizer::update(double loss, double reward, double successRate, int trainingSteps) {
    // Update histories
    lossHistory.push_back(loss);
    rewardHistory.push_back(reward);
    successRateHistory.push_back(successRate);

    // Keep history size limited to 50 (shows last 50 data points)
    if (static_cast<int>(lossHistory.size()) > maxHistorySize) lossHistory.pop_front();
    if (static_cast<int>(rewardHistory.size()) > maxHistorySize) rewardHistory.pop_front();
    if (static_cast<int>(successRateHistory.size()) > maxHistorySize) successRateHistory.pop_front();

    // Update curves
    updateCurves();

    // Update text displays - simplified format
    std::stringstream ss;

    // Loss (shorter format)
    ss << "Loss: " << std::fixed << std::setprecision(3) << loss;
    lossText.setString(ss.str());

    // Reward
    ss.str("");
    ss << "Reward: " << std::fixed << std::setprecision(1) << reward;
    rewardText.setString(ss.str());

    // Success rate
    ss.str("");
    ss << "Success: " << std::fixed << std::setprecision(1) << successRate << "%";
    successText.setString(ss.str());

    // Training steps
    ss.str("");
    ss << "Steps: " << trainingSteps;
    trainingStepsText.setString(ss.str());
}

void TrainingVisualizer::draw() {
    if (!isVisible) return;

    if (isPanelMode) {
        // Draw panel background
        window.draw(panelBackground);

        // Draw panel title
        window.draw(panelTitle);
    }

    // Draw graph and content
    window.draw(graphBackground);

    // Draw curves
    if (lossCurve.getVertexCount() > 0) window.draw(lossCurve);
    if (rewardCurve.getVertexCount() > 0) window.draw(rewardCurve);
    if (successCurve.getVertexCount() > 0) window.draw(successCurve);

    // Draw text elements
    window.draw(trainingStepsText);
    window.draw(lossText);
    window.draw(rewardText);
    window.draw(successText);

    // Add grid lines
    if (isPanelMode) {
        // Vertical grid lines
        for (int i = 1; i < 4; i++) {
            float x = graphPosition.x + (graphWidth / 4) * i;
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, graphPosition.y), sf::Color(100, 100, 100, 50)),
                sf::Vertex(sf::Vector2f(x, graphPosition.y + graphHeight), sf::Color(100, 100, 100, 50))
            };
            window.draw(line, 2, sf::Lines);
        }

        // Horizontal grid lines
        for (int i = 1; i < 3; i++) {
            float y = graphPosition.y + (graphHeight / 3) * i;
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(graphPosition.x, y), sf::Color(100, 100, 100, 50)),
                sf::Vertex(sf::Vector2f(graphPosition.x + graphWidth, y), sf::Color(100, 100, 100, 50))
            };
            window.draw(line, 2, sf::Lines);
        }
    }
}


void TrainingVisualizer::setPanelMode(bool panelMode) {
    isPanelMode = panelMode;

    if (panelMode) {
        // Adjust sizes for panel mode
        graphWidth = 320.0f;
        graphHeight = 140.0f;
        graphBackground.setSize(sf::Vector2f(graphWidth, graphHeight));
    }
}

// Remove button-related code from setPanelPosition:
void TrainingVisualizer::setPanelPosition(const sf::Vector2f& position) {
    panelPosition = position;
    panelBackground.setPosition(position);
    panelTitle.setPosition(position.x + 10, position.y + 10);

    // Update graph position
    graphPosition = sf::Vector2f(position.x + 15, position.y + 40);
    graphBackground.setPosition(graphPosition);

    // Update text positions
    float textY = graphPosition.y + graphHeight + 10;
    lossText.setPosition(graphPosition.x + 5, textY);
    rewardText.setPosition(graphPosition.x + 110, textY);

    textY += 18;
    successText.setPosition(graphPosition.x + 5, textY);
    trainingStepsText.setPosition(graphPosition.x + 110, textY);
}

void TrainingVisualizer::setPanelSize(const sf::Vector2f& size) {
    panelSize = size;
    panelBackground.setSize(size);

    // Adjust graph size proportionally
    graphWidth = size.x - 30;
    graphHeight = size.y * 0.4f;

    graphBackground.setSize(sf::Vector2f(graphWidth, graphHeight));
}

double TrainingVisualizer::getMaxValue(const std::deque<double>& history) const {
    if (history.empty()) return 1.0;
    return *std::max_element(history.begin(), history.end());
}

double TrainingVisualizer::getMinValue(const std::deque<double>& history) const {
    if (history.empty()) return 0.0;
    return *std::min_element(history.begin(), history.end());
}

void TrainingVisualizer::setPosition(const sf::Vector2f& position) {
    graphPosition = position;
    graphBackground.setPosition(position);
    trainingStepsText.setPosition(position.x, position.y - 25);
}

void TrainingVisualizer::setSize(float width, float height) {
    graphWidth = width;
    graphHeight = height;
    graphBackground.setSize(sf::Vector2f(width, height));
}

void TrainingVisualizer::clearHistory() {
    lossHistory.clear();
    rewardHistory.clear();
    successRateHistory.clear();
    lossCurve.clear();
    rewardCurve.clear();
    successCurve.clear();
}


bool TrainingVisualizer::isDashboardVisible() const {
    return isVisible;
}

void TrainingVisualizer::toggleVisibility() {
    isVisible = !isVisible;
}