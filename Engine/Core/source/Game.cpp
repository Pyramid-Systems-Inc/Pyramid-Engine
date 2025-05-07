#include "Pyramid/Core/Game.hpp"
#include "Pyramid/Core/Game.hpp"
#include "Pyramid/Platform/Windows/Win32OpenGLWindow.hpp" // Added for concrete window creation
#include <Pyramid/Core/Log.hpp> // Added
// #include <cassert> // No longer needed directly
#include <memory>
#include <chrono>

namespace Pyramid {

Game::Game(GraphicsAPI api)
    : m_window(nullptr) // Initialize m_window
    , m_graphicsDevice(nullptr) // Initialize m_graphicsDevice
    , m_isRunning(false)
{
    // Create window first
    // For now, we assume Win32OpenGLWindow for OpenGL.
    // A more advanced system might have a WindowFactory or platform-specific creation.
    if (api == GraphicsAPI::OpenGL) {
        m_window = std::make_unique<Win32OpenGLWindow>();
    }
    // TODO: Add window creation for other APIs/platforms if necessary

    PYRAMID_CORE_ASSERT(m_window, "Failed to create window"); // Changed
    if (!m_window) return; // Early exit if window creation fails

    // Create graphics device, passing the window
    m_graphicsDevice = IGraphicsDevice::Create(api, m_window.get());
    PYRAMID_CORE_ASSERT(m_graphicsDevice, "Failed to create graphics device"); // Changed
}

Game::~Game()
{
    // m_graphicsDevice and m_window are unique_ptrs and will be cleaned up automatically.
    // The order of destruction (m_graphicsDevice first, then m_window)
    // is determined by their declaration order in Game.hpp.
    // OpenGLDevice's destructor calls its Shutdown, which no longer touches m_window directly.
}

void Game::onCreate()
{
    // Base implementation. Derived classes should call this.
    // Initialize graphics device
    if (!m_graphicsDevice || !m_graphicsDevice->Initialize()) {
        PYRAMID_LOG_CRITICAL("Graphics device failed to initialize!"); // Changed
        m_isRunning = false; // Prevent run() from looping
        return;
    }
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
    // Call onCreate which now handles graphics device initialization
    // and sets m_isRunning.
    // The Game constructor already ensures m_window and m_graphicsDevice are created (or asserts).
    onCreate(); 

    if (!m_isRunning) {
        PYRAMID_LOG_ERROR("Game::onCreate failed. Aborting run loop."); // Changed
        return; // Don't start loop if onCreate failed (e.g. device init failed)
    }

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_isRunning)
    {
        // Process window messages
        if (m_window && !m_window->ProcessMessages())
        {
            m_isRunning = false; // Window requested close
            break; 
        }

        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        onUpdate(deltaTime); // Game logic and usually clearing the screen
        onRender();      // Drawing and presenting the frame
    }
}

void Game::quit()
{
    m_isRunning = false;
}

} // namespace Pyramid
