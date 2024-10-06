#include "OpenGL3D/Game/OglGame.h"

#include "OpenGL3D/Window/OglWindow.h"
#include "OpenGL3D/Graphic/OpenGLGraphicEngine.h"


OglGame::OglGame()
{
	/// <summary>
	/// Use The Graphic Engine (OpenGl)
	/// </summary>
	m_GraphicEngine = std::make_unique<OpenGLGraphicEngine>();

	m_Display = std::make_unique<OglWindow>();

	m_Display->makeCurrentContext();
}
//main Game Method
OglGame::~OglGame()
{
	
}
void OglGame::onCreate()
{
	
}
void OglGame::onUpdate()
{
	//Pick Color in Form Of Vector4
	//RGBA To Vector4 Color Here:https://codepen.io/sandstedt/full/gJqJEJ
	m_GraphicEngine->Clear(OGLVec4(0.207, 0.741, 0.796, 0.7));


	m_Display->present(false);
}
void OglGame::onQuit()
{
}
//Set The pool m_IsRuning To False - to Close the Program
void OglGame::quit()
{
	m_IsRuning = false;
}
