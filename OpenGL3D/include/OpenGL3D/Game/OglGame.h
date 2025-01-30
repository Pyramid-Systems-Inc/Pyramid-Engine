/**
 * @file OglGame.h
 * @brief Main game engine class that manages the OpenGL context and game loop
 */

#pragma once
#include <memory>

class OpenGLGraphicEngine;
class OglWindow;

/**
 * @class OglGame
 * @brief Core game engine class that manages the game loop and OpenGL context
 * 
 * This class serves as the main entry point for the game engine functionality.
 * It manages the game window, graphics engine, and main game loop.
 * Inherit from this class to create your own game implementation.
 */
class OglGame
{
public:
    /**
     * @brief Constructor - Initializes the game engine components
     */
    OglGame();

    /**
     * @brief Destructor - Cleans up engine resources
     */
    ~OglGame();

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
    bool m_IsRuning = true;

    /** @brief Graphics engine instance that manages OpenGL rendering */
    std::unique_ptr<OpenGLGraphicEngine> m_GraphicEngine;

    /** @brief Window instance that manages the game window */
    std::unique_ptr<OglWindow> m_Display;
};