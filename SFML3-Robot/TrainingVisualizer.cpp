#include "TrainingVisualizer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

TrainingVisualizer::TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f)
    : window(win), font(f), maxHistorySize(100),
    graphWidth(300.0f), graphHeight(150.0f), graphPosition(20.0f, 400.0f) {

    // Initialize graph background
    graphBackground.setSize(sf::Vector2f(graphWidth, graphHeight));
    graphBackground.setPosition(graphPosition);
    graphBackground.setFillColor(sf::Color(30, 30, 30, 200));
    graphBackground.setOutlineThickness(1);
    graphBackground.setOutlineColor(sf::Color::White);

    // Initialize curves
    lossCurve.setPrimitiveType(sf::LineStrip);
    rewardCurve.setPrimitiveType(sf::LineStrip);
    successCurve.setPrimitiveType(sf::LineStrip);

    // Initialize text
    lossText.setFont(font);
    lossText.setCharacterSize(12);
    lossText.setFillColor(sf::Color::Red);

    rewardText.setFont(font);
    rewardText.setCharacterSize(12);
    rewardText.setFillColor(sf::Color::Green);

    successText.setFont(font);
    successText.setCharacterSize(12);
    successText.setFillColor(sf::Color::Cyan);

    trainingStepsText.setFont(font);
    trainingStepsText.setCharacterSize(14);
    trainingStepsText.setFillColor(sf::Color::Yellow);
    trainingStepsText.setPosition(graphPosition.x, graphPosition.y - 25);
}

void TrainingVisualizer::update(double loss, double reward, double successRate, int trainingSteps) {
    // Update histories
    lossHistory.push_back(loss);
    rewardHistory.push_back(reward);
    successRateHistory.push_back(successRate);

    // Keep history size limited
    if (lossHistory.size() > maxHistorySize) lossHistory.pop_front();
    if (rewardHistory.size() > maxHistorySize) rewardHistory.pop_front();
    if (successRateHistory.size() > maxHistorySize) successRateHistory.pop_front();

    // Update curves
    updateCurves();

    // Update text
    std::stringstream ss;
    ss << "Loss: " << std::fixed << std::setprecision(4) << loss;
    lossText.setString(ss.str());
    lossText.setPosition(graphPosition.x + 5, graphPosition.y + 5);

    ss.str("");
    ss << "Reward: " << std::fixed << std::setprecision(2) << reward;
    rewardText.setString(ss.str());
    rewardText.setPosition(graphPosition.x + 5, graphPosition.y + 25);

    ss.str("");
    ss << "Success: " << std::fixed << std::setprecision(1) << successRate << "%";
    successText.setString(ss.str());
    successText.setPosition(graphPosition.x + 5, graphPosition.y + 45);

    ss.str("");
    ss << "Training Steps: " << trainingSteps;
    trainingStepsText.setString(ss.str());
}

void TrainingVisualizer::updateCurves() {
    lossCurve.clear();
    rewardCurve.clear();
    successCurve.clear();

    if (lossHistory.empty()) return;

    // Find value ranges for normalization
    double maxLoss = getMaxValue(lossHistory);
    double minLoss = getMinValue(lossHistory);
    double lossRange = std::max(0.1, maxLoss - minLoss);

    double maxReward = getMaxValue(rewardHistory);
    double minReward = getMinValue(rewardHistory);
    double rewardRange = std::max(0.1, maxReward - minReward);

    // Draw curves
    float xStep = graphWidth / std::max(1.0f, static_cast<float>(lossHistory.size() - 1));

    // ← CORRECTION ICI : ajout de static_cast
    for (size_t i = 0; i < lossHistory.size(); ++i) {
        float x = graphPosition.x + static_cast<float>(i) * xStep;

        // Loss curve (red)
        if (static_cast<int>(i) < static_cast<int>(lossHistory.size())) {  // Cast pour éviter warning
            float lossY = graphPosition.y + graphHeight -
                static_cast<float>((lossHistory[i] - minLoss) / lossRange * graphHeight);
            lossCurve.append(sf::Vertex(sf::Vector2f(x, lossY), sf::Color::Red));
        }

        // Reward curve (green)
        if (static_cast<int>(i) < static_cast<int>(rewardHistory.size())) {
            float rewardY = graphPosition.y + graphHeight -
                static_cast<float>((rewardHistory[i] - minReward) / rewardRange * graphHeight);
            rewardCurve.append(sf::Vertex(sf::Vector2f(x, rewardY), sf::Color::Green));
        }

        // Success rate curve (cyan)
        if (static_cast<int>(i) < static_cast<int>(successRateHistory.size())) {
            float successY = graphPosition.y + graphHeight -
                static_cast<float>(successRateHistory[i] / 100.0 * graphHeight);
            successCurve.append(sf::Vertex(sf::Vector2f(x, successY), sf::Color::Cyan));
        }
    }
}

double TrainingVisualizer::getMaxValue(const std::deque<double>& history) const {
    if (history.empty()) return 1.0;
    return *std::max_element(history.begin(), history.end());
}

double TrainingVisualizer::getMinValue(const std::deque<double>& history) const {
    if (history.empty()) return 0.0;
    return *std::min_element(history.begin(), history.end());
}

void TrainingVisualizer::draw() {
    window.draw(graphBackground);
    window.draw(lossCurve);
    window.draw(rewardCurve);
    window.draw(successCurve);
    window.draw(lossText);
    window.draw(rewardText);
    window.draw(successText);
    window.draw(trainingStepsText);
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