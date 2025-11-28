#pragma once
#include "Enums.h"
#include "Point.h"
#include <SFML/Graphics.hpp>

class Robot {
private:
    float fx = 0.0f, fy = 0.0f;
    Point gridPos{ 0,0 };
    Point targetPos{ 0,0 };
    float moveDuration = 0.3f;
    float elapsed = 0.0f;
    bool moving = false;
    RobotState state = RobotState::IDLE;
    int stepCount = 0;

public:
    Robot() = default;

    void setPosition(Point p);
    Point getPosition() const { return gridPos; }
    void setState(RobotState s) { state = s; }
    RobotState getState() const { return state; }

    void setMoveDuration(float duration) { moveDuration = duration; }
    float getMoveDuration() const { return moveDuration; }

    void moveTo(Point next);
    void update(float dt);
    void pause();
    void resume();
    bool isPaused() const { return state == RobotState::PAUSED; }

    sf::Vector2f getFloatPos(float cellSize) const;
    int getSteps() const { return stepCount; }
    bool isMoving() const { return moving; }
};