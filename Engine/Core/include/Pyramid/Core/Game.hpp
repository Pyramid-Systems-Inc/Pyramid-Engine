#pragma once
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <memory>

namespace Pyramid {

/**
 * @file Game.hpp
 * @brief Main game engine class that manages the graphics device and game loop
 */

/**
 * @class Game
 * @brief Core game engine class that manages the game loop and graphics device
 * 
 * This class serves as the main entry point for the game engine functionality.
 * It manages the graphics device and main game loop.
 * Inherit from this class to create your own game implementation.
 */
class Game
{
public:
    /**
     * @brief Constructor - Initializes the game engine components
     * @param api The graphics API to use (defaults to OpenGL)
     */
    explicit Game(GraphicsAPI api = GraphicsAPI::OpenGL);

    /**
     * @brief Destructor - Cleans up engine resources
     */
    virtual ~Game();

    /**
     * @brief Starts the main game loop
     * This function blocks until the game is quit
     */
    void run();

    /**
     * @brief Signals the game loop to stop
     * Call this to gracefully shut down the game
     */
    void quit();

protected:
    /**
     * @brief Called when the game engine is first created
     * Override this to initialize your game resources
     */
    virtual void onCreate();

    /**
     * @brief Called every frame to update game logic
     * Override this to implement your game update logic
     * @param deltaTime Time elapsed since last update in seconds
     */
    virtual void onUpdate(float deltaTime);

    /**
     * @brief Called every frame to render the game
     * Override this to implement your game rendering
     */
    virtual void onRender();

private:
    std::unique_ptr<IGraphicsDevice> m_graphicsDevice;
    bool m_isRunning;
};

} // namespace Pyramid