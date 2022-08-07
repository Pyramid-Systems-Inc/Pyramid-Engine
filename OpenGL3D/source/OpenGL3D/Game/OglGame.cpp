#include <OpenGL3D/Game/OglGame.h>
#include <OpenGL3D/Window/OglWindow.h>
#include <Windows.h>


OglGame::OglGame()
{
	m_Display = new OglWindow();
}

OglGame::~OglGame()
{
	delete m_Display;
}

void OglGame::run()
{
	//Log Mesg System
	MSG msg;

	//While The Project Is Running - See The Mesg and Log It!
	while (m_IsRuning && m_Display->IsClosed())
	{
		//
		if (PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
		{
			//
			TranslateMessage(&msg);
			DispatchMessage(&msg);


		}
		//Stop logging for 1 ms - avoid 
		Sleep(1);
	}
}
//Set The pool m_IsRuning To False - Once the project closes
void OglGame::quit()
{
	m_IsRuning = false;
}
