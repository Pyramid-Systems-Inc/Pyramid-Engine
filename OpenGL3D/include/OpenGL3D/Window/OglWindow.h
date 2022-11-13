#pragma once
class OglWindow
{
public:
	OglWindow();
	~OglWindow();

	void makeCurrentContext();
	void present(bool vSync);
private:
	void* m_Handle = nullptr;
	void* m_Context = nullptr;
};

