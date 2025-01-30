#include "Pyramid/Core/Game.hpp"
#include "Pyramid/Graphics/OpenGL/OpenGLDevice.hpp"
#include <windows.h>
#include <cassert>
#include <memory>
#include <chrono>

namespace Pyramid {

Game::Game(GraphicsAPI api)
    : m_isRunning(false)
{
    // Create graphics device
    if (api == GraphicsAPI::OpenGL)
        m_graphicsDevice = std::make_unique<OpenGLDevice>();
    else
        m_graphicsDevice = IGraphicsDevice::Create(api);
    assert(m_graphicsDevice && "Failed to create graphics device");
}

Game::~Game()
{
    if (m_graphicsDevice)
        m_graphicsDevice->Shutdown();
}

void Game::onCreate()
{
    m_isRunning = true;
}

void Game::onUpdate(float deltaTime)
{
    // Default implementation - clear screen with dark blue color
    m_graphicsDevice->Clear(Color(0.2f, 0.3f, 0.3f, 1.0f));
}

void Game::onRender()
{
    // Present the frame
    m_graphicsDevice->Present(true);
}

void Game::run()
{
    onCreate();

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_isRunning)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        onUpdate(deltaTime);
        onRender();
    }
}

void Game::quit()
{
    m_isRunning = false;
}

} // namespace Pyramid
