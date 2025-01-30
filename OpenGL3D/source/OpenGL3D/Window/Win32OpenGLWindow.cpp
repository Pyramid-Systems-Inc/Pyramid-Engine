#include <OpenGL3D/Window/OglWindow.h>
#include <glad\glad_wgl.h>
#include <glad\glad.h>
#include <windows.h>
#include <assert.h>

/**
 * @brief Windows event handler callback function
 * 
 * This function processes all window events (messages) sent by the Windows OS.
 * It handles basic window operations like closing and destroying the window.
 * 
 * @param hwnd Window handle
 * @param msg Message type
 * @param wParam Additional message information
 * @param lParam Additional message information
 * @return LRESULT Result of message processing
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) 
{
    switch (msg)
    {
    case WM_DESTROY:
    {
        // Get the window instance associated with this window
        // This can be used to handle cleanup if needed
        GetWindowLongPtr(hwnd, GWLP_USERDATA);
        break;
    }
    case WM_CLOSE:
    {
        // Post quit message to terminate the application
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return NULL;
}

/**
 * @brief Constructor for the Win32 OpenGL window implementation
 * 
 * Creates a Win32 window and initializes the OpenGL context.
 * The window is created with default size of 1024x768 pixels.
 */
OglWindow::OglWindow()
{
    // Create and register the window class
    WNDCLASSEXW wn = {};
    wn.cbSize = sizeof(WNDCLASSEXW);
    wn.lpszClassName = L"PyramidGameEngine";
    wn.lpfnWndProc = &WndProc;
    wn.style = CS_OWNDC;

    ATOM classId = RegisterClassExW(&wn);
    assert(classId);

    // Calculate window size with standard decorations
    RECT rc = { 0, 0, 1024, 768 };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

    // Create the main window
    m_Handle = CreateWindowExW(
        0,                              // Extended window style
        (LPCWSTR)(DWORD_PTR)classId,   // Window class
        L"Pyramid Game Engine",         // Window title
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,  // Window position
        rc.right - rc.left,            // Window width
        rc.bottom - rc.top,            // Window height
        NULL, NULL, NULL, NULL
    );

    assert(m_Handle);

    // Associate the window instance with the window handle
    SetWindowLongPtr((HWND)m_Handle, GWLP_USERDATA, (LONG_PTR)this);

    // Show and update the window
    ShowWindow((HWND)m_Handle, SW_SHOW);
    UpdateWindow((HWND)m_Handle);

    // Initialize OpenGL context
    auto hDC = GetDC(HWND(m_Handle));

    // Configure pixel format for OpenGL
    int glPixelFormat = 0;
    UINT numFormats = 0;

    // Define pixel format attributes for modern OpenGL
    int pixelFormatAttributes[] =
    {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };

    // Choose and set pixel format
    wglChoosePixelFormatARB(hDC, pixelFormatAttributes, nullptr, 1, &glPixelFormat, &numFormats);
    assert(numFormats);

    PIXELFORMATDESCRIPTOR pixelFormatDesc = {};
    DescribePixelFormat(hDC, glPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormatDesc);
    SetPixelFormat(hDC, glPixelFormat, &pixelFormatDesc);

    // Create OpenGL 4.0 core profile context
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

void OglWindow::makeCurrentContext()
{
    wglMakeCurrent(GetDC(HWND(m_Handle)), HGLRC(m_Context));
}

void OglWindow::present(bool vSync)
{
    wglSwapIntervalEXT(vSync);
    wglSwapLayerBuffers(GetDC(HWND(m_Handle)), WGL_SWAP_MAIN_PLANE);
}
