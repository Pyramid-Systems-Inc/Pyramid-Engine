#include <OpenGL3D/Game/OglGame.h>
#include <OpenGL3D/Window/OglWindow.h>
#include <Windows.h>


OglGame::OglGame()
{
	m_Display = std::make_unique<OglWindow>();
}

OglGame::~OglGame()
{
	
}

void OglGame::run()
{
	
	
	while (m_IsRuning)
	{
		MSG msg = {};
		if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				m_IsRuning = false;
				continue;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}
		Sleep(1);
	}
}
//Set The pool m_IsRuning To False - Once the project closes
void OglGame::quit()
{
	m_IsRuning = false;
}
