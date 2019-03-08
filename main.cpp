#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include "Game.h"
#include "Assets.h"
#include <iostream>

int main()
{
    Game game;
    Assets assets;
    game.run();

    return 0;
}