#pragma once
class OglWindow
{
public:
	OglWindow();
	~OglWindow();
private:
	void* m_Handle = nullptr;
	void* m_Context = nullptr;
};

