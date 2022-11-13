#include <OpenGL3D/Game/OglGame.h>
#include <OpenGL3D/Window/OglWindow.h>
#include <OpenGL3D\Graphic\OpenGLGraphicEngine.h>


OglGame::OglGame()
{
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
	m_GraphicEngine->Clear(OGLVec4(1, 0, 0, 20));


	m_Display->present(false);
}
void OglGame::onQuit()
{
}
//Set The pool m_IsRuning To False - Once the project closes
void OglGame::quit()
{
	m_IsRuning = false;
}
