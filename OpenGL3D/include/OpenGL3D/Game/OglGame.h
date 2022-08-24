#pragma once
#include <memory>
class OglWindow;
class OglGame
{
public:
	OglGame();
	~OglGame();

	// Run Function to start The Main Window
	void run();
	// Quit Function to Stop The Main Window
	void quit();

protected:

	bool m_IsRuning = true;
	std::unique_ptr<OglWindow> m_Display;
};