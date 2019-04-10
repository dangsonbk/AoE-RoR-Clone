#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include "Game.h"
#include <iostream>

int main()
{
    Game* game = new Game();
    game->run();

    delete game;
    return 0;
}
