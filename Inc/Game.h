#ifndef GAME_HPP
#define GAME_HPP

#include "SFML/Graphics.hpp"
#include "Common.h"
#include "Assets.h"

class Game
{

private:
    Assets* m_assets;

public:
    Game();
    void run();

private:
    void handlePlayerInput(sf::Keyboard::Key key, bool isPressed);
    void processEvents();
    void update(sf::Time deltaTime);
    void render();

private:
    sf::RenderWindow mWindow;
    sf::Texture mTexture;
    sf::Sprite mPlayer;
    bool mIsMovingUp = false;
    bool mIsMovingDown = false;
    bool mIsMovingLeft = false;
    bool mIsMovingRight = false;
};

#endif
