#include <OpenGL3D/Window/OglWindow.h>
#include <windows.h>
#include <assert.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	switch (msg)
	{
	case WM_DESTROY:
	{
		OglWindow* window = (OglWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		window->OnDestroy();
		break;
	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return NULL;
}

//Main Class of Window Using Win32 API
//See: https://docs.microsoft.com/en-us/windows/win32/learnwin32/learn-to-program-for-windows
OglWindow::OglWindow()
{
	//Define Window Class
	WNDCLASSEX wn = {};

	//Assign Values:
	//Specifies the size, in bytes, of this structure.
	//Data Type = UINT, An unsigned INT. The range is 0 through 4294967295 decimal
	wn.cbSize = sizeof(WNDCLASSEX);
	//Class Name
	wn.lpszClassName = L"OpenGL3DWindow";
	/*
	A pointer to the window procedure, callback function, which you define in your 
	application, that processes messages sent to a window.See:" LRESULT CALLBACK WndProc "
	*/
	wn.lpfnWndProc = &WndProc;


	assert(RegisterClassEx(&wn));

	RECT rc = { 0,0,1024,768 };

	AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

	m_Handle = CreateWindowEx(NULL, L"OpenGL3DWindow", L"Main Window - East Wind", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, 
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left , rc.bottom - rc.top, NULL, NULL, NULL, NULL);

	assert(m_Handle);

	SetWindowLongPtr((HWND)m_Handle,GWLP_USERDATA,(LONG_PTR)this);

	ShowWindow((HWND)m_Handle, SW_SHOW);
	UpdateWindow((HWND)m_Handle);
}

OglWindow::~OglWindow()
{
	DestroyWindow((HWND)m_Handle);
}

void OglWindow::OnDestroy()
{
	m_Handle = nullptr;


}

bool OglWindow::IsClosed()
{
	return m_Handle;
}
