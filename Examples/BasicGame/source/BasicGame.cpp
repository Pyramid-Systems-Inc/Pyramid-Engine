#include "BasicGame.hpp"

BasicGame::BasicGame()
    : Game(Pyramid::GraphicsAPI::OpenGL)
{
}

void BasicGame::onCreate()
{
    Game::onCreate();
}

void BasicGame::onUpdate(float deltaTime)
{
    Game::onUpdate(deltaTime);
}

void BasicGame::onRender()
{
    Game::onRender();
}
