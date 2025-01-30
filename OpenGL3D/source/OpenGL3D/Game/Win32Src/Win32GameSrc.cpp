#include <OpenGL3D/Game/OglGame.h>
#include <windows.h>

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        // Create and run the game
        Pyramid::OglGame game;
        game.run();
        return 0;
    }
    catch (const std::exception& e)
    {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}