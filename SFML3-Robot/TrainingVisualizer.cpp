#include "TrainingVisualizer.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

TrainingVisualizer::TrainingVisualizer(sf::RenderWindow& win, const sf::Font& f)
    : window(win), font(f), maxHistorySize(100),
    graphWidth(300.0f), graphHeight(120.0f),
    isPanelMode(true), isVisible(true) {  // Set panel mode by default

    // Initialize panel
    panelPosition = sf::Vector2f(620.0f, 80.0f);  // Right side of game
    panelSize = sf::Vector2f(350.0f, 500.0f);

    // Panel background
    panelBackground.setSize(panelSize);
    panelBackground.setPosition(panelPosition);
    panelBackground.setFillColor(sf::Color(30, 30, 40, 240));
    panelBackground.setOutlineThickness(2);
    panelBackground.setOutlineColor(sf::Color(100, 100, 150));

    // Panel title
    panelTitle.setFont(font);
    panelTitle.setString("TRAINING DASHBOARD");
    panelTitle.setCharacterSize(18);
    panelTitle.setFillColor(sf::Color::Cyan);
    panelTitle.setStyle(sf::Text::Bold);
    panelTitle.setPosition(panelPosition.x + 10, panelPosition.y + 10);

    // Close/minimize button
    closeButton.setSize(sf::Vector2f(25, 25));
    closeButton.setPosition(panelPosition.x + panelSize.x - 30, panelPosition.y + 8);
    closeButton.setFillColor(sf::Color(200, 50, 50, 200));
    closeButton.setOutlineThickness(1);
    closeButton.setOutlineColor(sf::Color::White);

    closeButtonText.setFont(font);
    closeButtonText.setString("X");
    closeButtonText.setCharacterSize(14);
    closeButtonText.setFillColor(sf::Color::White);
    closeButtonText.setStyle(sf::Text::Bold);
    sf::FloatRect closeBounds = closeButtonText.getLocalBounds();
    closeButtonText.setOrigin(closeBounds.width / 2, closeBounds.height / 2);
    closeButtonText.setPosition(closeButton.getPosition().x + 12.5f,
        closeButton.getPosition().y + 12.5f);

    // Graph position inside panel
    graphPosition = sf::Vector2f(panelPosition.x + 15, panelPosition.y + 50);

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

    // Initialize text displays - position them inside panel
    float textY = graphPosition.y + graphHeight + 15;

    // Stats section title
    sf::Text statsTitle("PERFORMANCE METRICS", font, 14);
    statsTitle.setFillColor(sf::Color::Yellow);
    statsTitle.setPosition(graphPosition.x, textY);

    // Move textY down for metrics
    textY += 25;

    lossText.setFont(font);
    lossText.setCharacterSize(13);
    lossText.setFillColor(sf::Color::Red);
    lossText.setPosition(graphPosition.x + 10, textY);

    rewardText.setFont(font);
    rewardText.setCharacterSize(13);
    rewardText.setFillColor(sf::Color::Green);
    rewardText.setPosition(graphPosition.x + 150, textY);

    textY += 20;

    successText.setFont(font);
    successText.setCharacterSize(13);
    successText.setFillColor(sf::Color::Cyan);
    successText.setPosition(graphPosition.x + 10, textY);

    textY += 30;

    trainingStepsText.setFont(font);
    trainingStepsText.setCharacterSize(14);
    trainingStepsText.setFillColor(sf::Color::Yellow);
    trainingStepsText.setPosition(graphPosition.x, textY);

    // Add some section dividers
    textY += 40;

    // Add more informative text areas
    sf::Text infoTitle("TRAINING INFO", font, 14);
    infoTitle.setFillColor(sf::Color::Magenta);
    infoTitle.setPosition(graphPosition.x, textY);

    textY += 25;

    // Add info text
    infoText.setFont(font);
    infoText.setCharacterSize(12);
    infoText.setFillColor(sf::Color(200, 200, 200));
    infoText.setPosition(graphPosition.x, textY);

    // Add legend for graphs
    sf::Text legendTitle("GRAPH LEGEND", font, 14);
    legendTitle.setFillColor(sf::Color::White);
    legendTitle.setPosition(graphPosition.x, panelPosition.y + panelSize.y - 100);

    // Legend items
    sf::Text lossLegend("Red: Loss", font, 11);
    lossLegend.setFillColor(sf::Color::Red);
    lossLegend.setPosition(graphPosition.x, panelPosition.y + panelSize.y - 80);

    sf::Text rewardLegend("Green: Reward", font, 11);
    rewardLegend.setFillColor(sf::Color::Green);
    rewardLegend.setPosition(graphPosition.x + 100, panelPosition.y + panelSize.y - 80);

    sf::Text successLegend("Cyan: Success Rate", font, 11);
    successLegend.setFillColor(sf::Color::Cyan);
    successLegend.setPosition(graphPosition.x + 200, panelPosition.y + panelSize.y - 80);
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

    // Keep history size limited
    if (static_cast<int>(lossHistory.size()) > maxHistorySize) lossHistory.pop_front();
    if (static_cast<int>(rewardHistory.size()) > maxHistorySize) rewardHistory.pop_front();
    if (static_cast<int>(successRateHistory.size()) > maxHistorySize) successRateHistory.pop_front();

    // Update curves
    updateCurves();

    // Update text displays
    std::stringstream ss;

    // Training steps
    ss << "Training Steps: " << trainingSteps;
    trainingStepsText.setString(ss.str());

    // Loss
    ss.str("");
    ss << "Loss: " << std::fixed << std::setprecision(4) << loss;
    lossText.setString(ss.str());

    // Reward
    ss.str("");
    ss << "Reward: " << std::fixed << std::setprecision(2) << reward;
    rewardText.setString(ss.str());

    // Success rate
    ss.str("");
    ss << "Success Rate: " << std::fixed << std::setprecision(1) << successRate << "%";
    successText.setString(ss.str());

    // Info text
    ss.str("");
    ss << "History Size: " << lossHistory.size() << " samples\n";
    ss << "Max History: " << maxHistorySize << " samples\n";
    ss << "Learning in progress...";
    infoText.setString(ss.str());
}

void TrainingVisualizer::draw() {
    if (!isVisible) return;

    if (isPanelMode) {
        // Draw panel background
        window.draw(panelBackground);

        // Draw panel title
        window.draw(panelTitle);

        // Draw close button
        window.draw(closeButton);
        window.draw(closeButtonText);
    }

    // Draw graph and content
    window.draw(graphBackground);

    // Draw curves with slight glow effect
    sf::RenderStates states;

    // Draw curves
    window.draw(lossCurve);
    window.draw(rewardCurve);
    window.draw(successCurve);

    // Draw text elements
    window.draw(trainingStepsText);
    window.draw(lossText);
    window.draw(rewardText);
    window.draw(successText);
    window.draw(infoText);

    // Add some visual separators if in panel mode
    if (isPanelMode) {
        // Horizontal separator
        sf::RectangleShape separator(sf::Vector2f(graphWidth, 1));
        separator.setPosition(graphPosition.x, graphPosition.y + graphHeight + 60);
        separator.setFillColor(sf::Color(100, 100, 100, 150));
        window.draw(separator);

        // Another separator
        separator.setSize(sf::Vector2f(graphWidth, 1));
        separator.setPosition(graphPosition.x, panelPosition.y + panelSize.y - 120);
        window.draw(separator);
    }
}

// Handle panel events (close button)
void TrainingVisualizer::handlePanelEvents(const sf::Vector2f& mousePos, bool mouseClicked) {
    if (isPanelMode && isVisible) {
        // Check if close button is hovered
        bool isHovered = closeButton.getGlobalBounds().contains(mousePos);

        if (isHovered) {
            closeButton.setFillColor(sf::Color(255, 80, 80, 255));

            if (mouseClicked) {
                isVisible = false;  // Hide panel
                // Optionally, you could also minimize it instead of hiding
            }
        }
        else {
            closeButton.setFillColor(sf::Color(200, 50, 50, 200));
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

void TrainingVisualizer::setPanelPosition(const sf::Vector2f& position) {
    panelPosition = position;
    panelBackground.setPosition(position);
    closeButton.setPosition(position.x + panelSize.x - 30, position.y + 8);
    closeButtonText.setPosition(closeButton.getPosition().x + 12.5f,
        closeButton.getPosition().y + 12.5f);

    // Update graph position relative to panel
    graphPosition = sf::Vector2f(position.x + 15, position.y + 50);
    graphBackground.setPosition(graphPosition);

    // Update text positions
    trainingStepsText.setPosition(graphPosition.x, graphPosition.y + graphHeight + 15);
    lossText.setPosition(graphPosition.x + 10, graphPosition.y + graphHeight + 40);
    rewardText.setPosition(graphPosition.x + 150, graphPosition.y + graphHeight + 40);
    successText.setPosition(graphPosition.x + 10, graphPosition.y + graphHeight + 60);
    infoText.setPosition(graphPosition.x, graphPosition.y + graphHeight + 90);
}

void TrainingVisualizer::setPanelSize(const sf::Vector2f& size) {
    panelSize = size;
    panelBackground.setSize(size);

    // Adjust graph size proportionally
    graphWidth = size.x - 30;
    graphHeight = size.y * 0.3f;  // 30% of panel height

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