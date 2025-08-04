/**
 * @file Main.cpp
 * @brief Entry point for the Basic Rendering Example
 *
 * This file contains the main entry point for the Basic Rendering Example.
 * It initializes the game engine and starts the main game loop.
 */

#include "BasicRendering.hpp"
#include <windows.h>

/**
 * @brief Main entry point for the Basic Rendering Example
 *
 * Initializes the game engine and starts the main game loop.
 * All exceptions are caught and logged to the console.
 *
 * @return int Returns 0 on successful execution, 1 on error
 */
int WINAPI WinMain(
    [[maybe_unused]] HINSTANCE hInstance,
    [[maybe_unused]] HINSTANCE hPrevInstance,
    [[maybe_unused]] LPSTR lpCmdLine,
    [[maybe_unused]] int nCmdShow)
{
    try
    {
        // Create and run the basic rendering example
        BasicRendering example;
        example.run();
        return 0;
    }
    catch (const std::exception& e)
    {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}