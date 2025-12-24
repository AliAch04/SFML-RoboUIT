#include "MazeWidget.h"

MazeWidget::MazeWidget() {}

void MazeWidget::draw(sf::RenderWindow& window,
    Maze* maze,
    Robot* robot,
    float cellSize,
    sf::Vector2f offset,
    sf::Sprite& wallSprite,
    const sf::Texture& wallTexture,
    sf::Sprite& robotSprite,
    const sf::Texture& robotTexture) {

    if (!maze) return;

    // ===== DRAW MAZE =====
    sf::RectangleShape cellShape(sf::Vector2f(cellSize, cellSize));

    for (int y = 0; y < maze->height; ++y) {
        for (int x = 0; x < maze->width; ++x) {

            CellType t = maze->grid[y][x]->getType();

            float drawX = x * cellSize + offset.x;
            float drawY = y * cellSize + offset.y;

            if (t == CellType::WALL) {
                wallSprite.setPosition(drawX, drawY);
                wallSprite.setScale(
                    cellSize / wallTexture.getSize().x,
                    cellSize / wallTexture.getSize().y
                );
                window.draw(wallSprite);
            }
            else if (t == CellType::START) {
                cellShape.setPosition(drawX, drawY);
                cellShape.setFillColor(sf::Color(100, 220, 100));
                window.draw(cellShape);
            }
            else if (t == CellType::END) {
                cellShape.setPosition(drawX, drawY);
                cellShape.setFillColor(sf::Color(220, 100, 100));
                window.draw(cellShape);
            }
            else {
                cellShape.setPosition(drawX, drawY);
                cellShape.setFillColor(sf::Color::Black);
                window.draw(cellShape);
            }
        }
    }

    // ===== DRAW ROBOT =====
    if (robot) {
        sf::Vector2f floatPos = robot->getFloatPos(cellSize);

        float scaleX = cellSize / robotTexture.getSize().x;
        float scaleY = cellSize / robotTexture.getSize().y;
        robotSprite.setScale(scaleX, scaleY);

        robotSprite.setPosition(
            floatPos.x + offset.x,
            floatPos.y + offset.y
        );

        window.draw(robotSprite);
    }
}
