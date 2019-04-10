#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include "Game.h"
#include "Assets.h"
#include <iostream>

int main()
{
    Assets assets;
    Game game;
    game.run();

    return 0;
}