#include "Robot.h"

void Robot::setPosition(Point p) {
    gridPos = p;
    fx = static_cast<float>(p.x);
    fy = static_cast<float>(p.y);
    targetPos = p;
    moving = false;
}

void Robot::moveTo(Point next) {
    if (next == gridPos) return;
    targetPos = next;
    elapsed = 0.0f;
    moving = true;
    state = RobotState::MOVING;
    stepCount++;
}

void Robot::update(float dt) {
    if (state == RobotState::PAUSED) return;
    if (!moving) return;

    elapsed += dt;
    float t = moveDuration <= 0.0f ? 1.0f : (elapsed / moveDuration);
    if (t >= 1.0f) {
        fx = static_cast<float>(targetPos.x);
        fy = static_cast<float>(targetPos.y);
        gridPos = targetPos;
        moving = false;
        state = RobotState::IDLE;
    }
    else {
        float sx = static_cast<float>(gridPos.x);
        float sy = static_cast<float>(gridPos.y);
        fx = sx + (static_cast<float>(targetPos.x) - sx) * t;
        fy = sy + (static_cast<float>(targetPos.y) - sy) * t;
    }
}

void Robot::pause() {
    if (state == RobotState::MOVING || state == RobotState::IDLE) {
        state = RobotState::PAUSED;
    }
}

void Robot::resume() {
    if (state == RobotState::PAUSED) {
        state = moving ? RobotState::MOVING : RobotState::IDLE;
    }
}

sf::Vector2f Robot::getFloatPos(float cellSize) const {
    return { fx * cellSize, fy * cellSize };
}