#include "Game.h"

Game::Game() : mWindow(sf::VideoMode(640, 480), "SFML Application") , mTexture(), mPlayer()
{
    m_assets = new Assets();
    // TODO: Make game setting class that define where to load texture
    // Below is testing code for loading texture from pixels
    sf::Image image;
    AssetFrames frames = m_assets->get_by_id(73);
    uint8_t pixels[frames.frames[1].width*frames.frames[1].height*4];
    for(int i = 0; i < frames.frames[1].width*frames.frames[1].height; ++i) {
        // cout << "-" << frames.frames[1].pixels[i].r << endl;
        *(pixels + i*4) = frames.frames[1].pixels[i].r;
        *(pixels + i*4 + 1) = frames.frames[1].pixels[i].g;
        *(pixels + i*4 + 2) = frames.frames[1].pixels[i].b;
        *(pixels + i*4 + 3) = frames.frames[1].pixels[i].a;
    }
    image.create(frames.frames[1].width, frames.frames[1].height, pixels);

    if (!mTexture.loadFromImage(image)){
        cout << "Error loading asset from image" << endl;
        exit(1);
    }
    mPlayer.setTexture(mTexture);
    mPlayer.setPosition(100.f, 100.f);
}

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Time TimePerFrame = sf::seconds(1.f / 60.f);
    while (mWindow.isOpen())
    {
        processEvents();
        timeSinceLastUpdate += clock.restart();
        while (timeSinceLastUpdate > TimePerFrame)
        {
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            update(TimePerFrame);
        }
        render();
    }
}

void Game::handlePlayerInput(sf::Keyboard::Key key, bool isPressed)
{
    //TODO: Make key mapping
    if (key == sf::Keyboard::W)
        mIsMovingUp = isPressed;
    else if (key == sf::Keyboard::S)
        mIsMovingDown = isPressed;
    else if (key == sf::Keyboard::A)
        mIsMovingLeft = isPressed;
    else if (key == sf::Keyboard::D)
        mIsMovingRight = isPressed;
}

void Game::processEvents()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        switch (event.type)
        {
            case sf::Event::Closed:
                mWindow.close();
                break;

            case sf::Event::KeyPressed:
                handlePlayerInput(event.key.code, true);
                break;
            case sf::Event::KeyReleased:
                handlePlayerInput(event.key.code, false);
                break;
            default:
                break;
        }
    }
}

void Game::update(sf::Time deltaTime)
{
    float PlayerSpeed = 100.f;
    sf::Vector2f movement(0.f, 0.f);
    if (mIsMovingUp)
        movement.y -= PlayerSpeed;
    if (mIsMovingDown)
        movement.y += PlayerSpeed;
    if (mIsMovingLeft)
        movement.x -= PlayerSpeed;
    if (mIsMovingRight)
        movement.x += PlayerSpeed;
    mPlayer.move(movement * deltaTime.asSeconds());
}

void Game::render()
{
    mWindow.clear();
    mWindow.draw(mPlayer);
    mWindow.display();
}
