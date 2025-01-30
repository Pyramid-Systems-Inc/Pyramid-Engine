#include <OpenGL3D\Graphic\OpenGLGraphicEngine.h>
#include <glad\glad_wgl.h>
#include <glad\glad.h>
#include <assert.h>
#include <stdexcept>


OpenGLGraphicEngine::OpenGLGraphicEngine()
{
	//Define Window Class, contains window class information
	WNDCLASSEXW wn = {};

	/*
		Assign Values, Specifies the size, in bytes, of this structure.
		Data Type = UINT, An unsigned INT. The range is 0 through 4294967295 decimal
	*/
	wn.cbSize = sizeof(WNDCLASSEXW);
	//Class Name
	wn.lpszClassName = L"OpenGL3DSecWindow";
	/*
	A pointer to the window procedure, callback function, which you define in your
	application, that processes messages sent to a window. See:" LRESULT CALLBACK WndProc
	*/
	wn.lpfnWndProc = DefWindowProcW;

	wn.style = CS_OWNDC;

	//window class register
	ATOM classId = RegisterClassExW(&wn);
	//assigning class id
	assert(classId);

	//SecWindows handler that create A Dummy Window that is being used for OpenGL Context
	auto SecWindow = CreateWindowExW(
		0,                              // Extended window style
		(LPCWSTR)(DWORD_PTR)classId,   // Window class
		L"",                           // Window title
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, NULL, NULL
	);

	//Check and assumes SecWindow handler and Pointer Does Exist
	assert(SecWindow);

	//Get Context From SecWindow aka Dummy Window
	auto SecDeviceContext = GetDC(SecWindow);

	//describes the pixel format of a drawing surface
	PIXELFORMATDESCRIPTOR PixelFormatDesc = {};

	//the size of this data structure. default is :: sizeof(PIXELFORMATDESCRIPTOR).
	PixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);

	//the version of this data structure. This value should be set to 1.
	PixelFormatDesc.nVersion = 1;

	//
	PixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;

	//
	PixelFormatDesc.iPixelType = PFD_TYPE_RGBA;

	//
	PixelFormatDesc.cColorBits = 32;

	//
	PixelFormatDesc.cAlphaBits = 8;

	//
	PixelFormatDesc.cDepthBits = 24;
	//
	PixelFormatDesc.cStencilBits = 8;
	//
	PixelFormatDesc.iLayerType = PFD_MAIN_PLANE;

	//
	auto PixelFormat = ChoosePixelFormat(SecDeviceContext, &PixelFormatDesc);
	//
	SetPixelFormat(SecDeviceContext, PixelFormat, &PixelFormatDesc);

	auto SecContext = wglCreateContext(SecDeviceContext);

	assert(SecContext);

	wglMakeCurrent(SecDeviceContext, SecContext);

	if (!gladLoadWGL(SecDeviceContext))
		throw std::runtime_error("OpenGLGraphicEngine - error - gladLoadWGL failed");

	if (!gladLoadGL())
		throw std::runtime_error("OpenGLGraphicEngine - error - gladLoadGL failed");

	wglMakeCurrent(SecDeviceContext, 0);

	wglDeleteContext(SecContext);

	ReleaseDC(SecWindow, SecDeviceContext);

	DestroyWindow(SecWindow);
}

OpenGLGraphicEngine::~OpenGLGraphicEngine()
{

}