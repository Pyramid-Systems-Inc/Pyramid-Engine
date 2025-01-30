#pragma once
#include <OpenGL3D/Graphics/IGraphicsDevice.h>
#include <memory>

namespace Pyramid {

/**
 * @file OglGame.h
 * @brief Main game engine class that manages the graphics device and game loop
 */

/**
 * @class OglGame
 * @brief Core game engine class that manages the game loop and graphics device
 * 
 * This class serves as the main entry point for the game engine functionality.
 * It manages the graphics device and main game loop.
 * Inherit from this class to create your own game implementation.
 */
class OglGame
{
public:
    /**
     * @brief Constructor - Initializes the game engine components
     * @param api The graphics API to use (defaults to OpenGL)
     */
    OglGame(GraphicsAPI api = GraphicsAPI::OpenGL);

    /**
     * @brief Destructor - Cleans up engine resources
     */
    virtual ~OglGame();

    /**
     * @brief Called when the game engine is first created
     * Override this to initialize your game resources
     */
    virtual void onCreate();

    /**
     * @brief Called every frame to update game logic
     * Override this to implement your game update logic
     */
    virtual void onUpdate();

    /**
     * @brief Called when the game is shutting down
     * Override this to clean up your game resources
     */
    virtual void onQuit();

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
    /** @brief Flag indicating if the game loop is running */
    bool m_IsRunning = true;

    /** @brief Graphics device instance */
    std::unique_ptr<IGraphicsDevice> m_GraphicsDevice;
};

} // namespace Pyramid