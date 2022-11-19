#include <OpenGL3D/Game/OglGame.h>
#include <OpenGL3D/Window/OglWindow.h>
#include <OpenGL3D\Graphic\OpenGLGraphicEngine.h>
#include <Windows.h>



//Main Run method
void OglGame::run()
{
	//Before Starting The Engine Run OpenGL Context Creation
	onCreate();


	//if the engine is running
	while (m_IsRuning)
	{
		//Message value
		MSG msg = {};
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
		{
			// If msg Value is == WM_QUIT Then Begin event Quit
			if (msg.message == WM_QUIT)
			{
				//Set The Is Running To false
				m_IsRuning = false;
				continue;
			}
			//Other Translate Or Dispatch Message
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}
		//OpenGL Context Update
		onUpdate();
	}
	//OpenGl Quit
	onQuit();
}