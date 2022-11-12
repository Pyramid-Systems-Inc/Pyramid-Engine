#include <OpenGL3D/Window/OglWindow.h>
#include <glad\glad_wgl.h>
#include <glad\glad.h>
#include <windows.h>
#include <assert.h>

/*
WndProc:
	This function MUST be created and used to determine how your application will respond to various events.
	The Windows procedure may also be called the event handler.
	------------------------------------------------------------------
	Event Message System:
		What You see here is how win32 API works, consents of Event Message
		System that handle window quit and resize and minimizing and maximizing,Etc.
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	 //change between Event Message
	switch (msg)
	{
	//event for destroying windows
	case WM_DESTROY:
	{
		//the gets the window info and the sets the data based on user events
		OglWindow* window = (OglWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		/*
		GWLP_USERDATA:
			Sets the user data associated with the window.This data is intended for use by the application 
			that created the window.Its value is initially zero.
		*/
		break;
	}
	//event for closing the window
	case WM_CLOSE:
	{
		//send the quit message
		PostQuitMessage(0);
		break;
	}
	//send the case of above to the window DefWindowProc
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return NULL;
}

/* 
	Main Class of Window Using Win32 API:
	See: https://docs.microsoft.com/en-us/windows/win32/learnwin32/learn-to-program-for-windows
*/
OglWindow::OglWindow()
{
	//Define Window Class, contains window class information
	WNDCLASSEX wn = {};

	/*
		Assign Values, Specifies the size, in bytes, of this structure.
		Data Type = UINT, An unsigned INT. The range is 0 through 4294967295 decimal
	*/
	wn.cbSize = sizeof(WNDCLASSEX);
	//Class Name
	wn.lpszClassName = L"OpenGL3DWindow";
	/*
	A pointer to the window procedure, callback function, which you define in your 
	application, that processes messages sent to a window. See:" LRESULT CALLBACK WndProc
	*/
	wn.lpfnWndProc = &WndProc;

	//window class register
	auto classId = RegisterClassEx(&wn);
	//assigning class id
	assert(classId);

	//main starting size of the window
	//need to be more data oriented / TO Do
	RECT rc = { 0,0,1024,768 };

	//windows window theme values
	AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

	//windows handler that create window, with all its values, needs more explain
	m_Handle = CreateWindowEx(NULL, MAKEINTATOM(classId), L"Main Window - East Wind", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left , rc.bottom - rc.top, NULL, NULL, NULL, NULL);

	//Check and assumes window handler and Pointer Does Exist
	assert(m_Handle); 

	//window long pointer set
	SetWindowLongPtr((HWND)m_Handle,GWLP_USERDATA,(LONG_PTR)this);

	//show window
	ShowWindow((HWND)m_Handle, SW_SHOW);

	//update based on events
	UpdateWindow((HWND)m_Handle);


	//Create OpenGL Context 

	auto hDC = GetDC(HWND(m_Handle));


	//Pixel Format Attributes
	//
	int glPixelFormat = 0;
	//
	UINT numFormats = 0;

	int pixelFormatAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,
		GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,
		GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,
		GL_TRUE,
		WGL_ACCELERATION_ARB,
		WGL_FULL_ACCELERATION_ARB,
		WGL_PIXEL_TYPE_ARB,
		WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB,32,
		WGL_DEPTH_BITS_ARB,24,
		WGL_STENCIL_BITS_ARB,8,0
	};

	wglChoosePixelFormatARB(hDC, pixelFormatAttributes, nullptr, 1, &glPixelFormat, &numFormats);

	assert(numFormats);

	PIXELFORMATDESCRIPTOR pixelFormatDesc = {};

	DescribePixelFormat(hDC, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDesc);
	SetPixelFormat(hDC, glPixelFormat, &pixelFormatDesc);

	int OpenGlAttributes[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_LAYER_PLANE_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	m_Context = wglCreateContextAttribsARB(hDC, 0, OpenGlAttributes);

	assert(m_Context);
}

OglWindow::~OglWindow()
{
	wglDeleteContext(HGLRC(m_Context));
	DestroyWindow((HWND)m_Handle);
}
