#include "ControlPanelWidget.h"

ControlPanelWidget::ControlPanelWidget() {
    panel.setSize(sf::Vector2f(200, 600));
    panel.setPosition(600, 0);
    panel.setFillColor(sf::Color(50, 50, 50));
}

void ControlPanelWidget::draw(sf::RenderWindow& window) {
    window.draw(panel);
}
