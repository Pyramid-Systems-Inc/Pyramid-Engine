/**
 * @file Main.cpp
 * @brief Entry point for the Pyramid Game Engine application
 * 
 * This file contains the main entry point for the Pyramid Game Engine.
 * It initializes the game engine, handles exceptions, and starts the main game loop.
 */

#include <OpenGL3D/Game/OglGame.h>
#include <iostream>

/**
 * @brief Custom game class that inherits from OglGame
 */
class PyramidGame : public Pyramid::OglGame
{
public:
    PyramidGame() : OglGame(Pyramid::GraphicsAPI::OpenGL) {}

    void onCreate() override
    {
        // Initialize game resources here
    }

    void onUpdate() override
    {
        // Call base class update first
        OglGame::onUpdate();

        // Add game-specific update logic here
    }

    void onQuit() override
    {
        // Clean up game resources here
        OglGame::onQuit();
    }
};

/**
 * @brief Main entry point for the Pyramid Game Engine
 * 
 * Initializes the game engine and starts the main game loop.
 * All exceptions are caught and logged to the console.
 * 
 * @return int Returns 0 on successful execution, 1 on error
 */
int main() 
{
    try
    {
        // Create the main game instance
        PyramidGame game;

        // Start the game loop
        game.run();

        return 0;
    }
    catch (const std::exception& e)
    {
        // Log any unhandled exceptions
        std::cout << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
}