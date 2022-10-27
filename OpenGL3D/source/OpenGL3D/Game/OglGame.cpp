#include <OpenGL3D/Game/OglGame.h>
#include <OpenGL3D/Window/OglWindow.h>
#include <OpenGL3D\Graphic\OpenGLGraphicEngine.h>
#include <Windows.h>


OglGame::OglGame()
{
	m_GraphicEngine = std::make_unique<OpenGLGraphicEngine>();

	m_Display = std::make_unique<OglWindow>();
}
//main Game Method
OglGame::~OglGame()
{
	
}
//Main Run method
void OglGame::run()
{
	
	//if the engine is running
	while (m_IsRuning)
	{
		//Message value
		MSG msg = {};
		if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			// If msg Value is == WM_QUIT Then Begin event Quit
			if (msg.message == WM_QUIT)
			{
				m_IsRuning = false;
				continue;
			}
			//Other Dispatch Message
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}
		//Wait For 1 ms
		Sleep(1);
	}
}
//Set The pool m_IsRuning To False - Once the project closes
void OglGame::quit()
{
	m_IsRuning = false;
}
