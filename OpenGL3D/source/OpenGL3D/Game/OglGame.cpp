#include "OpenGL3D/Game/OglGame.h"
#include <windows.h>
#include <cassert>

namespace Pyramid {

OglGame::OglGame(GraphicsAPI api)
{
    // Create graphics device
    m_GraphicsDevice = IGraphicsDevice::Create(api);
    assert(m_GraphicsDevice && "Failed to create graphics device");
}

OglGame::~OglGame()
{
    onQuit();
}

void OglGame::onCreate()
{
}

void OglGame::onUpdate()
{
    // Clear screen with dark blue color
    m_GraphicsDevice->Clear(Color(0.0f, 0.0f, 0.2f, 1.0f));
}

void OglGame::onQuit()
{
    if (m_GraphicsDevice)
        m_GraphicsDevice->Shutdown();
}

void OglGame::run()
{
    // Initialize graphics device
    if (!m_GraphicsDevice->Initialize())
        return;

    m_GraphicsDevice->MakeContextCurrent();

    // Call user initialization
    onCreate();

    // Main game loop
    MSG msg = {};
    while (m_IsRunning)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                quit();
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Update game logic
        onUpdate();

        // Present the frame
        m_GraphicsDevice->Present(true);
    }
}

void OglGame::quit()
{
    m_IsRunning = false;
}

} // namespace Pyramid
