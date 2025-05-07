#include <Pyramid/Platform/Windows/Win32OpenGLWindow.hpp>
#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <string> // For strlen
#include <vector> // For dynamic buffer for wide string conversion

// Ensure <windows.h> is available for MultiByteToWideChar,
// It's usually pulled in by glad_wgl.h or other Windows-specific headers.
// If not, it would need to be explicitly included, but that's unlikely here.

namespace Pyramid {

LRESULT CALLBACK Win32OpenGLWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Win32OpenGLWindow* window = nullptr;
    if (msg == WM_CREATE)
    {
        CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<Win32OpenGLWindow*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else
    {
        window = reinterpret_cast<Win32OpenGLWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch (msg)
    {
    case WM_CLOSE:
        if (window)
            window->m_shouldClose = true;
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

Win32OpenGLWindow::Win32OpenGLWindow()
    : m_hwnd(nullptr)
    , m_hdc(nullptr)
    , m_hglrc(nullptr)
    , m_width(800)
    , m_height(600)
    , m_shouldClose(false)
{
}

Win32OpenGLWindow::~Win32OpenGLWindow()
{
    if (m_hglrc)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(m_hglrc);
    }

    if (m_hdc)
        ReleaseDC(m_hwnd, m_hdc);

    if (m_hwnd)
        DestroyWindow(m_hwnd);
}

bool Win32OpenGLWindow::Initialize(const char* title, int width, int height)
{
    m_width = width;
    m_height = height;

    if (!RegisterWindowClass())
        return false;

    // Create the window
    RECT windowRect = { 0, 0, width, height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Convert title to wide string
    std::wstring wTitle;
    if (title) {
        int titleLen = static_cast<int>(strlen(title));
        if (titleLen > 0) {
            int wideLen = MultiByteToWideChar(CP_UTF8, 0, title, titleLen, nullptr, 0);
            if (wideLen > 0) {
                std::vector<wchar_t> wideBuf(wideLen);
                MultiByteToWideChar(CP_UTF8, 0, title, titleLen, wideBuf.data(), wideLen);
                wTitle.assign(wideBuf.data(), wideLen);
            }
        }
    }
    if (wTitle.empty()) {
        wTitle = L"Pyramid Game"; // Default title if conversion fails or input is null/empty
    }

    m_hwnd = CreateWindowExW(
        0,                              // Extended style
        L"PyramidWindowClass",          // Class name
        wTitle.c_str(),                 // Window title (Changed)
        WS_OVERLAPPEDWINDOW,            // Style
        CW_USEDEFAULT,                  // X position
        CW_USEDEFAULT,                  // Y position
        windowRect.right - windowRect.left,  // Width
        windowRect.bottom - windowRect.top,  // Height
        nullptr,                        // Parent window
        nullptr,                        // Menu
        GetModuleHandle(nullptr),       // Instance handle
        this                           // Additional application data
    );

    if (!m_hwnd)
        return false;

    // Get the device context
    m_hdc = GetDC(m_hwnd);
    if (!m_hdc)
        return false;

    // Create OpenGL context
    if (!CreateOpenGLContext())
        return false;

    // Show the window
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    return true;
}

bool Win32OpenGLWindow::RegisterWindowClass()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = Win32OpenGLWindow::WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = L"PyramidWindowClass";

    return RegisterClassExW(&wc) != 0;
}

bool Win32OpenGLWindow::CreateOpenGLContext()
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  // Size of this struct
        1,                              // Version
        PFD_DRAW_TO_WINDOW |           // Support window
        PFD_SUPPORT_OPENGL |           // Support OpenGL
        PFD_DOUBLEBUFFER,              // Double buffered
        PFD_TYPE_RGBA,                 // RGBA type
        32,                            // 32-bit color depth
        0, 0, 0, 0, 0, 0,             // Color bits ignored
        0,                             // No alpha buffer
        0,                             // Shift bit ignored
        0,                             // No accumulation buffer
        0, 0, 0, 0,                    // Accumulation bits ignored
        24,                            // 24-bit z-buffer
        8,                             // 8-bit stencil buffer
        0,                             // No auxiliary buffer
        PFD_MAIN_PLANE,                // Main drawing layer
        0,                             // Reserved
        0, 0, 0                        // Layer masks ignored
    };

    int pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
    if (!pixelFormat)
        return false;

    if (!SetPixelFormat(m_hdc, pixelFormat, &pfd))
        return false;

    m_hglrc = wglCreateContext(m_hdc);
    if (!m_hglrc)
        return false;

    if (!wglMakeCurrent(m_hdc, m_hglrc))
        return false;

    // Initialize GLAD
    if (!gladLoadWGL(m_hdc) || !gladLoadGL())
        return false;

    return true;
}

void Win32OpenGLWindow::Present(bool vsync)
{
    if (vsync)
        wglSwapIntervalEXT(1);
    else
        wglSwapIntervalEXT(0);

    SwapBuffers(m_hdc);
}

void Win32OpenGLWindow::MakeContextCurrent()
{
    if (m_hdc && m_hglrc)
        wglMakeCurrent(m_hdc, m_hglrc);
}

bool Win32OpenGLWindow::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return !m_shouldClose;
}

} // namespace Pyramid
