#pragma once
// Main Class Of Main Window
class OglWindow;
// Game Class aka Game Window - declaration
class OglGame
{
	// Public Class of Main Game Window
public:
	//The Main Function To Display Game Window - See OglGame.cpp
	OglGame();
	//The Main Function To Delete Game Window - See OglGame.cpp
	~OglGame();

	// Run Function to start The Main Window
	void run();
	// Quit Function to Stop The Main Window
	void quit();

protected:

	// Is The Program Running
	bool m_IsRuning = true;
	// Is The Window Showing - Null Value at Start 
	OglWindow* m_Display = nullptr;
};