#include "TrainingVisualizer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

TrainingVisualizer::TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f)
    : window(win), font(f), maxHistorySize(100),
    graphWidth(350.0f), graphHeight(180.0f), graphPosition(20.0f, 80.0f) {

    // Initialize title
    titleText.setFont(font);
    titleText.setString("TRAINING DASHBOARD");
    titleText.setCharacterSize(16);
    titleText.setFillColor(sf::Color::White);
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(graphPosition.x, graphPosition.y - 40);

    // Initialize graph background - slightly larger
    graphBackground.setSize(sf::Vector2f(graphWidth, graphHeight));
    graphBackground.setPosition(graphPosition);
    graphBackground.setFillColor(sf::Color(20, 20, 30, 220));  // Darker blue tint
    graphBackground.setOutlineThickness(2);
    graphBackground.setOutlineColor(sf::Color(100, 100, 150));

    // Initialize all curves
    lossCurve.setPrimitiveType(sf::LineStrip);
    rewardCurve.setPrimitiveType(sf::LineStrip);
    successCurve.setPrimitiveType(sf::LineStrip);
    qValueCurve.setPrimitiveType(sf::LineStrip);
    explorationCurve.setPrimitiveType(sf::LineStrip);
    stepsCurve.setPrimitiveType(sf::LineStrip);

    // Initialize all text displays with better positioning
    float textY = graphPosition.y + graphHeight + 5;

    lossText.setFont(font);
    lossText.setCharacterSize(11);
    lossText.setFillColor(sf::Color::Red);
    lossText.setPosition(graphPosition.x + 5, textY);

    rewardText.setFont(font);
    rewardText.setCharacterSize(11);
    rewardText.setFillColor(sf::Color::Green);
    rewardText.setPosition(graphPosition.x + 120, textY);

    successText.setFont(font);
    successText.setCharacterSize(11);
    successText.setFillColor(sf::Color::Cyan);
    successText.setPosition(graphPosition.x + 240, textY);

    // New text displays
    qValueText.setFont(font);
    qValueText.setCharacterSize(11);
    qValueText.setFillColor(sf::Color::Yellow);
    qValueText.setPosition(graphPosition.x + 5, textY + 20);

    explorationText.setFont(font);
    explorationText.setCharacterSize(11);
    explorationText.setFillColor(sf::Color::Magenta);
    explorationText.setPosition(graphPosition.x + 120, textY + 20);

    stepsText.setFont(font);
    stepsText.setCharacterSize(11);
    stepsText.setFillColor(sf::Color(255, 165, 0));  // Orange
    stepsText.setPosition(graphPosition.x + 240, textY + 20);

    // Episode and training info
    episodeText.setFont(font);
    episodeText.setCharacterSize(12);
    episodeText.setFillColor(sf::Color::White);
    episodeText.setPosition(graphPosition.x, graphPosition.y - 20);

    learningRateText.setFont(font);
    learningRateText.setCharacterSize(11);
    learningRateText.setFillColor(sf::Color(200, 200, 200));
    learningRateText.setPosition(graphPosition.x + graphWidth - 100, graphPosition.y + 5);

    epsilonText.setFont(font);
    epsilonText.setCharacterSize(11);
    epsilonText.setFillColor(sf::Color(200, 200, 200));
    epsilonText.setPosition(graphPosition.x + graphWidth - 100, graphPosition.y + 25);

    // Create legend
    createLegend();
}

void TrainingVisualizer::createLegend() {
    std::vector<std::pair<std::string, sf::Color>> legendItemsData = {
        {"Loss", sf::Color::Red},
        {"Reward", sf::Color::Green},
        {"Success", sf::Color::Cyan},
        {"Q-Value", sf::Color::Yellow},
        {"Exploration", sf::Color::Magenta},
        {"Steps/Episode", sf::Color(255, 165, 0)}
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
    qValueCurve.clear();
    explorationCurve.clear();
    stepsCurve.clear();

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

        if (i < qValueHistory.size()) {
            float y = normalizeValue(qValueHistory[i], qValueHistory, graphPosition.y, graphHeight);
            qValueCurve.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Yellow));
        }

        if (i < explorationRateHistory.size()) {
            float y = graphPosition.y + graphHeight -
                static_cast<float>(explorationRateHistory[i] * graphHeight);
            explorationCurve.append(sf::Vertex(sf::Vector2f(x, y), sf::Color::Magenta));
        }

        if (i < stepsPerEpisodeHistory.size()) {
            float y = normalizeValue(static_cast<double>(stepsPerEpisodeHistory[i]),
                stepsPerEpisodeHistory, graphPosition.y, graphHeight, true);
            stepsCurve.append(sf::Vertex(sf::Vector2f(x, y), sf::Color(255, 165, 0)));
        }
    }
}
void TrainingVisualizer::update(double loss, double reward, double successRate,
    int trainingSteps, int currentEpisode,
    double avgQValue, double explorationRate,
    double learningRate, double epsilon,
    int stepsThisEpisode) {

    // Update histories with new metrics
    lossHistory.push_back(loss);
    rewardHistory.push_back(reward);
    successRateHistory.push_back(successRate);
    qValueHistory.push_back(avgQValue);
    explorationRateHistory.push_back(explorationRate);
    stepsPerEpisodeHistory.push_back(stepsThisEpisode);

    // Keep history size limited
    if (lossHistory.size() > maxHistorySize) lossHistory.pop_front();
    if (rewardHistory.size() > maxHistorySize) rewardHistory.pop_front();
    if (successRateHistory.size() > maxHistorySize) successRateHistory.pop_front();
    if (qValueHistory.size() > maxHistorySize) qValueHistory.pop_front();
    if (explorationRateHistory.size() > maxHistorySize) explorationRateHistory.pop_front();
    if (stepsPerEpisodeHistory.size() > maxHistorySize) stepsPerEpisodeHistory.pop_front();

    // Update curves
    updateCurves();

    // Update text displays
    std::stringstream ss;

    // Top info line
    ss.str("");
    ss << "Episode: " << currentEpisode << " | Steps: " << trainingSteps;
    episodeText.setString(ss.str());

    // First row of metrics
    ss.str("");
    ss << "Loss: " << std::fixed << std::setprecision(4) << loss;
    lossText.setString(ss.str());

    ss.str("");
    ss << "Reward: " << std::fixed << std::setprecision(2) << reward;
    rewardText.setString(ss.str());

    ss.str("");
    ss << "Success: " << std::fixed << std::setprecision(1) << successRate << "%";
    successText.setString(ss.str());

    // Second row of metrics
    ss.str("");
    ss << "Q-Value: " << std::fixed << std::setprecision(3) << avgQValue;
    qValueText.setString(ss.str());

    ss.str("");
    ss << "Explore: " << std::fixed << std::setprecision(2) << (explorationRate * 100) << "%";
    explorationText.setString(ss.str());

    ss.str("");
    ss << "Steps: " << stepsThisEpisode;
    stepsText.setString(ss.str());

    // Learning parameters
    ss.str("");
    ss << "LR: " << std::scientific << std::setprecision(1) << learningRate;
    learningRateText.setString(ss.str());

    ss.str("");
    ss << "ε: " << std::fixed << std::setprecision(3) << epsilon;
    epsilonText.setString(ss.str());
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