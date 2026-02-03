#include "TrainingVisualizer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

TrainingVisualizer::TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f)
    : window(win), font(f), maxHistorySize(100),
    graphWidth(350.0f), graphHeight(150.0f), graphPosition(20.0f, 80.0f) {

    // Initialize title
    titleText.setFont(font);
    titleText.setString("TRAINING DASHBOARD");
    titleText.setCharacterSize(16);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(graphPosition.x, graphPosition.y - 40);

    // Initialize graph background
    graphBackground.setSize(sf::Vector2f(graphWidth, graphHeight));
    graphBackground.setPosition(graphPosition);
    graphBackground.setFillColor(sf::Color(20, 20, 30, 220));
    graphBackground.setOutlineThickness(2);
    graphBackground.setOutlineColor(sf::Color(100, 100, 150));

    // Initialize curves
    lossCurve.setPrimitiveType(sf::LineStrip);
    rewardCurve.setPrimitiveType(sf::LineStrip);
    successCurve.setPrimitiveType(sf::LineStrip);

    // Initialize text displays
    float textY = graphPosition.y + graphHeight + 5;

    lossText.setFont(font);
    lossText.setCharacterSize(12);
    lossText.setFillColor(sf::Color::Red);
    lossText.setPosition(graphPosition.x + 5, textY);

    rewardText.setFont(font);
    rewardText.setCharacterSize(12);
    rewardText.setFillColor(sf::Color::Green);
    rewardText.setPosition(graphPosition.x + 120, textY);

    successText.setFont(font);
    successText.setCharacterSize(12);
    successText.setFillColor(sf::Color::Cyan);
    successText.setPosition(graphPosition.x + 240, textY);

    trainingStepsText.setFont(font);
    trainingStepsText.setCharacterSize(14);
    trainingStepsText.setFillColor(sf::Color::Yellow);
    trainingStepsText.setPosition(graphPosition.x, graphPosition.y - 20);
}

void TrainingVisualizer::createLegend() {
    std::vector<std::pair<std::string, sf::Color>> legendItemsData = {
        {"Loss", sf::Color::Red},
        {"Reward", sf::Color::Green},
        {"Success", sf::Color::Cyan},

    };

    for (size_t i = 0; i < legendItemsData.size(); ++i) {
        sf::Text item;
        item.setFont(font);
        item.setString(legendItemsData[i].first);
        item.setCharacterSize(10);
        item.setFillColor(legendItemsData[i].second);
        item.setPosition(graphPosition.x + 5 + (i % 3) * 120, 
                        graphPosition.y + graphHeight - 70 + (i / 3) * 15);
        legendItems.push_back(item);
    }
}

void TrainingVisualizer::updateCurves() {
    // Clear all curves
    lossCurve.clear();
    rewardCurve.clear();
    successCurve.clear();

    if (lossHistory.empty()) return;

    // Calculate x step
    float xStep = graphWidth / std::max(1.0f, static_cast<float>(lossHistory.size() - 1));

    for (size_t i = 0; i < lossHistory.size(); ++i) {
        float x = graphPosition.x + static_cast<float>(i) * xStep;

        // Normalize each value for display
        if (i < lossHistory.size()) {
            float y = normalizeValue(lossHistory[i], lossHistory, graphPosition.y, graphHeight);
            lossCurve.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Red));
        }

        if (i < rewardHistory.size()) {
            float y = normalizeValue(rewardHistory[i], rewardHistory, graphPosition.y, graphHeight);
            rewardCurve.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Green));
        }

        if (i < successRateHistory.size()) {
            float y = graphPosition.y + graphHeight -
                static_cast<float>(successRateHistory[i] / 100.0 * graphHeight);
            successCurve.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Cyan));
        }

    }
}

void TrainingVisualizer::update(double loss, double reward, double successRate, int trainingSteps) {
    // Update histories
    lossHistory.push_back(loss);
    rewardHistory.push_back(reward);
    successRateHistory.push_back(successRate);

    // Keep history size limited
    if (static_cast<int>(lossHistory.size()) > maxHistorySize) lossHistory.pop_front();
    if (static_cast<int>(rewardHistory.size()) > maxHistorySize) rewardHistory.pop_front();
    if (static_cast<int>(successRateHistory.size()) > maxHistorySize) successRateHistory.pop_front();

    // Update curves
    updateCurves();

    // Update text displays
    std::stringstream ss;

    ss << "Steps: " << trainingSteps;
    trainingStepsText.setString(ss.str());

    ss.str("");
    ss << "Loss: " << std::fixed << std::setprecision(4) << loss;
    lossText.setString(ss.str());

    ss.str("");
    ss << "Reward: " << std::fixed << std::setprecision(2) << reward;
    rewardText.setString(ss.str());

    ss.str("");
    ss << "Success: " << std::fixed << std::setprecision(1) << successRate << "%";
    successText.setString(ss.str());
}


double TrainingVisualizer::getMaxValue(const std::deque<double>& history) const {
    if (history.empty()) return 1.0;
    return *std::max_element(history.begin(), history.end());
}

double TrainingVisualizer::getMinValue(const std::deque<double>& history) const {
    if (history.empty()) return 0.0;
    return *std::min_element(history.begin(), history.end());
}

float TrainingVisualizer::normalizeValue(double value, const std::deque<double>& history,
    float minY, float maxHeight, bool isStepCount) {
    if (history.empty()) return minY;

    double minVal = getMinValue(history);
    double maxVal = getMaxValue(history);

    // For step counts, cap the max for better visualization
    if (isStepCount && maxVal > 50) {
        maxVal = 50; // Cap at 50 steps for visualization
    }

    double range = std::max(0.1, maxVal - minVal);
    float normalized = static_cast<float>((value - minVal) / range * maxHeight);

    return graphPosition.y + graphHeight - normalized;
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

void TrainingVisualizer::drawGrid() {
    // Draw horizontal grid lines
    for (int i = 0; i <= 4; ++i) {
        float y = graphPosition.y + (graphHeight / 4.0f) * i;

        // Create a line array
        sf::Vertex line[2] = {
            sf::Vertex(sf::Vector2f(graphPosition.x, y), sf::Color(100, 100, 100, 100)),
            sf::Vertex(sf::Vector2f(graphPosition.x + graphWidth, y), sf::Color(100, 100, 100, 100))
        };
        window.draw(line, 2, sf::Lines);
    }

    // Draw vertical grid lines (fewer)
    for (int i = 0; i <= 2; ++i) {
        float x = graphPosition.x + (graphWidth / 2.0f) * i;

        // Create a line array
        sf::Vertex line[2] = {
            sf::Vertex(sf::Vector2f(x, graphPosition.y), sf::Color(100, 100, 100, 100)),
            sf::Vertex(sf::Vector2f(x, graphPosition.y + graphHeight), sf::Color(100, 100, 100, 100))
        };
        window.draw(line, 2, sf::Lines);
    }
}

// Visibility control methods
void TrainingVisualizer::setVisible(bool visible) {
    isVisible = visible;
}

bool TrainingVisualizer::isDashboardVisible() const {
    return isVisible;
}

void TrainingVisualizer::toggleVisibility() {
    isVisible = !isVisible;
}