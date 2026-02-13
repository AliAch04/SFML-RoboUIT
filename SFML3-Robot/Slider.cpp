#include "Slider.h"
#include "UIComponents.h"
#include <algorithm>
#include <string>

Slider::Slider(const sf::Vector2f& position, float width, float minVal, float maxVal, float initialVal,
    const std::string& sliderLabel, sf::Font& font)
    : minValue(minVal), maxValue(maxVal), currentValue(initialVal) {

    track.setSize(sf::Vector2f(width, 5));
    track.setPosition(position);
    track.setFillColor(sf::Color(150, 150, 150));

    thumb.setRadius(8);
    thumb.setFillColor(sf::Color::White);
    thumb.setOutlineThickness(1);
    thumb.setOutlineColor(sf::Color::Black);

    label.setFont(font);
    label.setString(sliderLabel);
    label.setCharacterSize(18);
    label.setFillColor(sf::Color::White);
    label.setPosition(position.x, position.y - 25);

    valueText.setFont(font);
    valueText.setCharacterSize(16);
    valueText.setFillColor(sf::Color::White);
    valueText.setPosition(position.x + width + 10, position.y - 5);

    updateThumbPosition();
    updateValueText();
}

void Slider::updateThumbPosition() {
    float ratio = (currentValue - minValue) / (maxValue - minValue);
    float x = track.getPosition().x + ratio * track.getSize().x;
    thumb.setPosition(x - thumb.getRadius(), track.getPosition().y - thumb.getRadius());
}

void Slider::updateValueText() {
    valueText.setString(toString(currentValue));
}

bool Slider::contains(const sf::Vector2f& point) const {
    return thumb.getGlobalBounds().contains(point);
}

void Slider::setValueFromMouse(const sf::Vector2f& point) {
    float relativeX = point.x - track.getPosition().x;
    float ratio = std::max(0.0f, std::min(1.0f, relativeX / track.getSize().x));
    currentValue = minValue + ratio * (maxValue - minValue);
    updateThumbPosition();
    updateValueText();
}

void Slider::setValue(float value) {
    currentValue = std::max(minValue, std::min(maxValue, value));
    updateThumbPosition();
    updateValueText();
}

void Slider::draw(sf::RenderWindow& window) const {
    window.draw(track);
    window.draw(thumb);
    window.draw(label);
    window.draw(valueText);
}

void Slider::setPosition(const sf::Vector2f& position) {
    // Move the entire slider to new position
    track.setPosition(position.x, position.y - track.getSize().y / 2.0f);

    // Update thumb position
    float trackLength = track.getSize().x;
    float t = (currentValue - minValue) / (maxValue - minValue);
    float thumbX = position.x + t * trackLength;
    thumb.setPosition(thumbX, position.y);

    // Update label position (above slider)
    sf::FloatRect labelBounds = label.getLocalBounds();
    label.setPosition(position.x, position.y - 40.0f);

    // Update value text position (to the right of slider)
    sf::FloatRect valueBounds = valueText.getLocalBounds();
    valueText.setPosition(position.x + trackLength + 20.0f,
        position.y - valueBounds.height / 2.0f);
}