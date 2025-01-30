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
        OglGame game;

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