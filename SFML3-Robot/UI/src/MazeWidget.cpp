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
    const sf::Texture& robotTexture)
{
    if (!maze) return;

    // Only used for START/END overlays now (floor is drawn by GameEngine)
    sf::RectangleShape cellShape(sf::Vector2f(cellSize, cellSize));

    for (int y = 0; y < maze->height; ++y) {
        for (int x = 0; x < maze->width; ++x) {

            CellType t = maze->grid[y][x]->getType();

            float drawX = x * cellSize + offset.x;
            float drawY = y * cellSize + offset.y;

            if (t == CellType::WALL) {
                wallSprite.setPosition(drawX, drawY);
                wallSprite.setScale(
                    cellSize / (float)wallTexture.getSize().x,
                    cellSize / (float)wallTexture.getSize().y
                );
                window.draw(wallSprite);
            }
            else if (t == CellType::START) {
                cellShape.setPosition(drawX, drawY);
                cellShape.setFillColor(sf::Color(100, 220, 100, 160)); // slight transparency
                window.draw(cellShape);
            }
            else if (t == CellType::END) {
                cellShape.setPosition(drawX, drawY);
                cellShape.setFillColor(sf::Color(220, 100, 100, 160)); // slight transparency
                window.draw(cellShape);
            }
            else {
                // DO NOTHING: let the floor texture drawn in GameEngine show here
            }
        }
    }

    // ===== DRAW ROBOT =====
    if (robot) {
        sf::Vector2f floatPos = robot->getFloatPos(cellSize);

        float scaleX = cellSize / (float)robotTexture.getSize().x;
        float scaleY = cellSize / (float)robotTexture.getSize().y;
        robotSprite.setScale(scaleX, scaleY);

        robotSprite.setPosition(
            floatPos.x + offset.x,
            floatPos.y + offset.y
        );

        window.draw(robotSprite);
    }
}
