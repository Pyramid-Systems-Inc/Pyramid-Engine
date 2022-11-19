#pragma once
#include <memory>
class OpenGLGraphicEngine;
class OglWindow;
/// <summary>
/// main Class Of the Game And OpenGl Context
/// that is used for game engine
/// </summary>
class OglGame
{
public:
	//Game Context Window
	OglGame();
	~OglGame();

	//OpenGl Create Method
	virtual void onCreate();
	//OpenGl Update Method
	virtual void onUpdate();
	//OpenGl Quit Method
	virtual void onQuit();

	// Run Function to start The Main Window
	void run();
	// Quit Function to Stop The Main Window
	void quit();

protected:
	//is the engine running
	bool m_IsRuning = true;
	//Pointer With OpenGl Graphic Engine Display
	std::unique_ptr<OpenGLGraphicEngine> m_GraphicEngine;
	//Pointer With Window Display
	std::unique_ptr<OglWindow> m_Display;
};