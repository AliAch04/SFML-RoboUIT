class TrainingVisualizer {
private:
    bool isVisible = true;
    sf::RenderWindow& window;
    sf::Font font;

    // Panel members
    bool isPanelMode;
    sf::Vector2f panelPosition;
    sf::Vector2f panelSize;
    sf::RectangleShape panelBackground;
    sf::Text panelTitle;

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

    void setPanelMode(bool panelMode);
    void setPanelPosition(const sf::Vector2f& position);
    void setPanelSize(const sf::Vector2f& size);

   
    void handlePanelEvents(const sf::Vector2f& mousePos, bool mouseClicked) {}

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