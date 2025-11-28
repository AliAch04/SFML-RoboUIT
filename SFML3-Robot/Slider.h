#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Slider {
private:
    sf::RectangleShape track;
    sf::CircleShape thumb;
    sf::Text label;
    sf::Text valueText;
    float minValue, maxValue, currentValue;
    bool dragging = false;

public:
    Slider(const sf::Vector2f& position, float width, float minVal, float maxVal, float initialVal,
        const std::string& sliderLabel, sf::Font& font);

    void updateThumbPosition();
    void updateValueText();
    bool contains(const sf::Vector2f& point) const;
    void setValueFromMouse(const sf::Vector2f& point);
    void setDragging(bool drag) { dragging = drag; }
    bool isDragging() const { return dragging; }
    float getValue() const { return currentValue; }
    void setValue(float value);
    void draw(sf::RenderWindow& window) const;
};