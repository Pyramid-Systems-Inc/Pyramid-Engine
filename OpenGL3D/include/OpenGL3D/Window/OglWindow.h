#pragma once
// The Main Window Displayer
class OglWindow
{
	// Main Function about the Window it self
public:
	// The Main Function To Display Window - See OglWindow.cpp
	OglWindow();
	// The Main Function To Delete Window - See OglWindow.cpp
	~OglWindow();
	void OnDestroy();
	bool IsClosed();
private:
	void* m_Handle = nullptr;
};

