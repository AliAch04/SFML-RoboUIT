#pragma once
#include <SFML/Graphics.hpp>
#include "Maze.h"
#include "Robot.h"

class MazeWidget {
public:
    MazeWidget();

    void draw(
        sf::RenderWindow& window,
        Maze* maze,
        Robot* robot,
        float cellSize,
        sf::Vector2f offset,
        sf::Sprite& wallSprite,
        const sf::Texture& wallTexture,
        sf::Sprite& robotSprite,
        const sf::Texture& robotTexture
    );
};
