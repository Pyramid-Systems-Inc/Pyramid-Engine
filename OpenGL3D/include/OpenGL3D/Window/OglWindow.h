#pragma once
class OglWindow
{
public:
	//OpenGL Window Method
	OglWindow();
	~OglWindow();
	/// <summary>
	/// so in that we craate a dummy context and and then make
	/// a real one this method is to switch bettwen dummy and real one
	/// </summary>
	void makeCurrentContext();
	void present(bool vSync);
private:
	//assume null pointer for Win pointer
	void* m_Handle = nullptr;
	//assume null pointer for OpenGL Context pointer
	void* m_Context = nullptr;
};

