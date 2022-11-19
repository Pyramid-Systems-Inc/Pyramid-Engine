#pragma once
class OglWindow
{
public:
	//OpenGL Window Method
	OglWindow();
	~OglWindow();

	void makeCurrentContext();
	void present(bool vSync);
private:
	//assume null pointer for win pointer
	void* m_Handle = nullptr;
	//assume null pointer for OpenGL Context pointer
	void* m_Context = nullptr;
};

