#include <Pyramid/Platform/Windows/Win32OpenGLWindow.hpp>
#include <Pyramid/Util/Log.hpp>
#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <windowsx.h>

// windowsx.h defines IsMinimized and IsMaximized as compatibility macros.
// They collide with the platform-neutral Window virtual method names below.
#ifdef IsMinimized
#undef IsMinimized
#endif
#ifdef IsMaximized
#undef IsMaximized
#endif

namespace Pyramid
{
    Key Win32OpenGLWindow::TranslateKey(WPARAM virtualKey, LPARAM keyData)
    {
        if (virtualKey >= static_cast<WPARAM>('0') &&
            virtualKey <= static_cast<WPARAM>('9'))
        {
            return static_cast<Key>(
                static_cast<u16>(Key::Num0) +
                static_cast<u16>(virtualKey - static_cast<WPARAM>('0')));
        }

        if (virtualKey >= static_cast<WPARAM>('A') &&
            virtualKey <= static_cast<WPARAM>('Z'))
        {
            return static_cast<Key>(
                static_cast<u16>(Key::A) +
                static_cast<u16>(virtualKey - static_cast<WPARAM>('A')));
        }

        if (virtualKey >= VK_F1 && virtualKey <= VK_F24)
        {
            return static_cast<Key>(
                static_cast<u16>(Key::F1) +
                static_cast<u16>(virtualKey - VK_F1));
        }

        if (virtualKey >= VK_NUMPAD0 && virtualKey <= VK_NUMPAD9)
        {
            return static_cast<Key>(
                static_cast<u16>(Key::Keypad0) +
                static_cast<u16>(virtualKey - VK_NUMPAD0));
        }

        switch (virtualKey)
        {
        case VK_SPACE: return Key::Space;
        case VK_OEM_7: return Key::Apostrophe;
        case VK_OEM_COMMA: return Key::Comma;
        case VK_OEM_MINUS: return Key::Minus;
        case VK_OEM_PERIOD: return Key::Period;
        case VK_OEM_2: return Key::Slash;
        case VK_OEM_1: return Key::Semicolon;
        case VK_OEM_PLUS: return Key::Equal;
        case VK_OEM_4: return Key::LeftBracket;
        case VK_OEM_5: return Key::Backslash;
        case VK_OEM_102: return Key::Backslash;
        case VK_OEM_6: return Key::RightBracket;
        case VK_OEM_3: return Key::GraveAccent;
        case VK_ESCAPE: return Key::Escape;
        case VK_TAB: return Key::Tab;
        case VK_BACK: return Key::Backspace;
        case VK_INSERT:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::Insert
                : Key::Keypad0;
        case VK_DELETE:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::Delete
                : Key::KeypadDecimal;
        case VK_END:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::End
                : Key::Keypad1;
        case VK_DOWN:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::Down
                : Key::Keypad2;
        case VK_NEXT:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::PageDown
                : Key::Keypad3;
        case VK_LEFT:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::Left
                : Key::Keypad4;
        case VK_CLEAR: return Key::Keypad5;
        case VK_RIGHT:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::Right
                : Key::Keypad6;
        case VK_HOME:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::Home
                : Key::Keypad7;
        case VK_UP:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::Up
                : Key::Keypad8;
        case VK_PRIOR:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::PageUp
                : Key::Keypad9;
        case VK_CAPITAL: return Key::CapsLock;
        case VK_SCROLL: return Key::ScrollLock;
        case VK_NUMLOCK: return Key::NumLock;
        case VK_SNAPSHOT: return Key::PrintScreen;
        case VK_PAUSE: return Key::Pause;
        case VK_DECIMAL: return Key::KeypadDecimal;
        case VK_DIVIDE: return Key::KeypadDivide;
        case VK_MULTIPLY: return Key::KeypadMultiply;
        case VK_SUBTRACT: return Key::KeypadSubtract;
        case VK_ADD: return Key::KeypadAdd;
        case VK_LWIN: return Key::LeftSuper;
        case VK_RWIN: return Key::RightSuper;
        case VK_APPS: return Key::Menu;
        case VK_RETURN:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::KeypadEnter
                : Key::Enter;
        case VK_LSHIFT: return Key::LeftShift;
        case VK_RSHIFT: return Key::RightShift;
        case VK_LCONTROL: return Key::LeftControl;
        case VK_RCONTROL: return Key::RightControl;
        case VK_LMENU: return Key::LeftAlt;
        case VK_RMENU: return Key::RightAlt;
        case VK_SHIFT:
        {
            const UINT scanCode = static_cast<UINT>((keyData >> 16) & 0xff);
            const UINT translated = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX);
            return translated == VK_RSHIFT ? Key::RightShift : Key::LeftShift;
        }
        case VK_CONTROL:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::RightControl
                : Key::LeftControl;
        case VK_MENU:
            return (keyData & (static_cast<LPARAM>(1) << 24)) != 0
                ? Key::RightAlt
                : Key::LeftAlt;
        default:
            return Key::Unknown;
        }
    }

    void Win32OpenGLWindow::UpdateMouseCapture()
    {
        const bool anyButtonDown =
            m_input.IsMouseButtonDown(MouseButton::Left) ||
            m_input.IsMouseButtonDown(MouseButton::Right) ||
            m_input.IsMouseButtonDown(MouseButton::Middle) ||
            m_input.IsMouseButtonDown(MouseButton::X1) ||
            m_input.IsMouseButtonDown(MouseButton::X2);

        if (anyButtonDown)
        {
            if (GetCapture() != m_hwnd)
            {
                SetCapture(m_hwnd);
            }
        }
        else if (GetCapture() == m_hwnd)
        {
            ReleaseCapture();
        }
    }

    LRESULT CALLBACK Win32OpenGLWindow::WndProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam)
    {
        Win32OpenGLWindow* window = nullptr;
        if (msg == WM_CREATE)
        {
            auto* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            window = reinterpret_cast<Win32OpenGLWindow*>(
                createStruct->lpCreateParams);
            SetWindowLongPtr(
                hwnd,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(window));
        }
        else
        {
            window = reinterpret_cast<Win32OpenGLWindow*>(
                GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        switch (msg)
        {
        case WM_SETFOCUS:
            if (window)
            {
                window->m_input.SetFocused(true);
            }
            return 0;

        case WM_KILLFOCUS:
            if (window)
            {
                window->m_input.SetFocused(false);
                window->UpdateMouseCapture();
            }
            return 0;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (window)
            {
                window->m_input.ProcessKey(
                    TranslateKey(wParam, lParam),
                    true);
            }
            return msg == WM_SYSKEYDOWN
                ? DefWindowProcW(hwnd, msg, wParam, lParam)
                : 0;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (window)
            {
                window->m_input.ProcessKey(
                    TranslateKey(wParam, lParam),
                    false);
            }
            return msg == WM_SYSKEYUP
                ? DefWindowProcW(hwnd, msg, wParam, lParam)
                : 0;

        case WM_UNICHAR:
            if (wParam == UNICODE_NOCHAR)
            {
                return TRUE;
            }
            if (window)
            {
                window->m_input.ProcessTextCodepoint(static_cast<char32_t>(wParam));
            }
            return 0;

        case WM_CHAR:
        case WM_SYSCHAR:
            if (window)
            {
                window->m_input.ProcessUtf16CodeUnit(static_cast<char16_t>(wParam));
            }
            return 0;

        case WM_DEADCHAR:
        case WM_SYSDEADCHAR:
            return 0;

        case WM_MOUSEMOVE:
            if (window)
            {
                window->m_input.ProcessMouseMove(
                    static_cast<f32>(GET_X_LPARAM(lParam)),
                    static_cast<f32>(GET_Y_LPARAM(lParam)));
            }
            return 0;

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_XBUTTONDOWN:
            if (window)
            {
                SetFocus(hwnd);
                MouseButton button = MouseButton::Left;
                if (msg == WM_RBUTTONDOWN)
                {
                    button = MouseButton::Right;
                }
                else if (msg == WM_MBUTTONDOWN)
                {
                    button = MouseButton::Middle;
                }
                else if (msg == WM_XBUTTONDOWN)
                {
                    button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1
                        ? MouseButton::X1
                        : MouseButton::X2;
                }

                window->m_input.ProcessMouseMove(
                    static_cast<f32>(GET_X_LPARAM(lParam)),
                    static_cast<f32>(GET_Y_LPARAM(lParam)));
                window->m_input.ProcessMouseButton(button, true);
                window->UpdateMouseCapture();
            }
            return msg == WM_XBUTTONDOWN ? TRUE : 0;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
            if (window)
            {
                MouseButton button = MouseButton::Left;
                if (msg == WM_RBUTTONUP)
                {
                    button = MouseButton::Right;
                }
                else if (msg == WM_MBUTTONUP)
                {
                    button = MouseButton::Middle;
                }
                else if (msg == WM_XBUTTONUP)
                {
                    button = GET_XBUTTON_WPARAM(wParam) == XBUTTON1
                        ? MouseButton::X1
                        : MouseButton::X2;
                }

                window->m_input.ProcessMouseMove(
                    static_cast<f32>(GET_X_LPARAM(lParam)),
                    static_cast<f32>(GET_Y_LPARAM(lParam)));
                window->m_input.ProcessMouseButton(button, false);
                window->UpdateMouseCapture();
            }
            return msg == WM_XBUTTONUP ? TRUE : 0;

        case WM_MOUSEWHEEL:
            if (window)
            {
                window->m_input.ProcessMouseWheel(
                    static_cast<f32>(GET_WHEEL_DELTA_WPARAM(wParam)) /
                    static_cast<f32>(WHEEL_DELTA));
            }
            return 0;

        case WM_MOUSEHWHEEL:
            if (window)
            {
                window->m_input.ProcessMouseWheel(
                    0.0f,
                    static_cast<f32>(GET_WHEEL_DELTA_WPARAM(wParam)) /
                    static_cast<f32>(WHEEL_DELTA));
            }
            return 0;

        case WM_CAPTURECHANGED:
            if (window && reinterpret_cast<HWND>(lParam) != hwnd)
            {
                window->m_input.ReleaseMouseButtons();
            }
            return 0;

        case WM_CLOSE:
            if (window)
            {
                window->m_shouldClose = true;
            }
            return 0;

        case WM_SIZE:
            if (window)
            {
                const int width = static_cast<int>(LOWORD(lParam));
                const int height = static_cast<int>(HIWORD(lParam));

                WindowResizeState state = WindowResizeState::Restored;
                if (wParam == SIZE_MINIMIZED)
                {
                    state = WindowResizeState::Minimized;
                }
                else if (wParam == SIZE_MAXIMIZED)
                {
                    state = WindowResizeState::Maximized;
                }

                const bool changed =
                    width != window->m_width ||
                    height != window->m_height ||
                    state != window->m_resizeState;

                window->m_width = width;
                window->m_height = height;
                window->m_resizeState = state;

                if (changed)
                {
                    window->DispatchResizeEvent({width, height, state});
                }
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }
    }

    Win32OpenGLWindow::Win32OpenGLWindow()
        : m_hwnd(nullptr),
          m_hdc(nullptr),
          m_hglrc(nullptr),
          m_width(800),
          m_height(600),
          m_shouldClose(false),
          m_resizeState(WindowResizeState::Restored)
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

    bool Win32OpenGLWindow::Initialize(const char *title, int width, int height)
    {
        PYRAMID_LOG_INFO("Initializing Win32 OpenGL window (", width, "x", height, ")...");

        m_width = width;
        m_height = height;

        if (!RegisterWindowClass())
        {
            PYRAMID_LOG_ERROR("Failed to register window class");
            return false;
        }

        PYRAMID_LOG_INFO("Window class registered successfully");

        // Create the window
        RECT windowRect = {0, 0, width, height};
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

        // Convert title to wide string
        std::wstring wTitle;
        if (title)
        {
            int titleLen = static_cast<int>(strlen(title));
            if (titleLen > 0)
            {
                int wideLen = MultiByteToWideChar(CP_UTF8, 0, title, titleLen, nullptr, 0);
                if (wideLen > 0)
                {
                    std::vector<wchar_t> wideBuf(wideLen);
                    MultiByteToWideChar(CP_UTF8, 0, title, titleLen, wideBuf.data(), wideLen);
                    wTitle.assign(wideBuf.data(), wideLen);
                }
            }
        }
        if (wTitle.empty())
        {
            wTitle = L"Pyramid Game"; // Default title if conversion fails or input is null/empty
        }

        m_hwnd = CreateWindowExW(
            0,                                  // Extended style
            L"PyramidWindowClass",              // Class name
            wTitle.c_str(),                     // Window title (Changed)
            WS_OVERLAPPEDWINDOW,                // Style
            CW_USEDEFAULT,                      // X position
            CW_USEDEFAULT,                      // Y position
            windowRect.right - windowRect.left, // Width
            windowRect.bottom - windowRect.top, // Height
            nullptr,                            // Parent window
            nullptr,                            // Menu
            GetModuleHandle(nullptr),           // Instance handle
            this                                // Additional application data
        );

        if (!m_hwnd)
        {
            PYRAMID_LOG_ERROR("Failed to create window");
            return false;
        }

        PYRAMID_LOG_INFO("Window created successfully");

        // Get the device context
        m_hdc = GetDC(m_hwnd);
        if (!m_hdc)
        {
            PYRAMID_LOG_ERROR("Failed to get device context");
            return false;
        }

        PYRAMID_LOG_INFO("Device context obtained successfully");

        // Create OpenGL context
        PYRAMID_LOG_INFO("Creating OpenGL context...");
        if (!CreateOpenGLContext())
        {
            PYRAMID_LOG_ERROR("Failed to create OpenGL context");
            return false;
        }

        PYRAMID_LOG_INFO("OpenGL context created successfully");

        // Show the window
        ShowWindow(m_hwnd, SW_SHOW);
        UpdateWindow(m_hwnd);
        m_input.SetFocused(GetFocus() == m_hwnd);

        return true;
    }

    bool Win32OpenGLWindow::RegisterWindowClass()
    {
        // Check if the class is already registered
        WNDCLASSEXW existingClass = {};
        if (GetClassInfoExW(GetModuleHandle(nullptr), L"PyramidWindowClass", &existingClass))
        {
            PYRAMID_LOG_INFO("Window class already registered, reusing existing class");
            return true; // Class already exists, no need to register again
        }

        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = Win32OpenGLWindow::WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = L"PyramidWindowClass";

        ATOM result = RegisterClassExW(&wc);
        if (result == 0)
        {
            DWORD error = GetLastError();
            if (error == ERROR_CLASS_ALREADY_EXISTS)
            {
                PYRAMID_LOG_INFO("Window class already exists (ERROR_CLASS_ALREADY_EXISTS), continuing...");
                return true; // This is actually fine
            }
            else
            {
                PYRAMID_LOG_ERROR("Failed to register window class, error code: ", error);
                return false;
            }
        }

        PYRAMID_LOG_INFO("Window class registered successfully");
        return true;
    }

    bool Win32OpenGLWindow::CreateOpenGLContext()
    {
        // First create a temporary context to get WGL extensions
        PIXELFORMATDESCRIPTOR pfd = {
            sizeof(PIXELFORMATDESCRIPTOR), // Size of this struct
            1,                             // Version
            PFD_DRAW_TO_WINDOW |           // Support window
                PFD_SUPPORT_OPENGL |       // Support OpenGL
                PFD_DOUBLEBUFFER,          // Double buffered
            PFD_TYPE_RGBA,                 // RGBA type
            32,                            // 32-bit color depth
            0, 0, 0, 0, 0, 0,              // Color bits ignored
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

        // Create temporary context
        HGLRC tempContext = wglCreateContext(m_hdc);
        if (!tempContext)
            return false;

        if (!wglMakeCurrent(m_hdc, tempContext))
        {
            wglDeleteContext(tempContext);
            return false;
        }

        // Initialize GLAD to get WGL extensions
        if (!gladLoadWGL(m_hdc))
        {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(tempContext);
            return false;
        }

        // Now create the actual OpenGL 4.6 context using WGL_ARB_create_context
        if (wglCreateContextAttribsARB)
        {
            // Try OpenGL 4.6 first, then fallback to lower versions
            const int versions[][2] = {
                {4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0}, {3, 3}};

#ifdef NDEBUG
            constexpr int contextFlagOptions[] = {
                WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB};
#else
            constexpr int contextFlagOptions[] = {
                WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
                WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB};
#endif

            for (const auto &version : versions)
            {
                for (const int contextFlags : contextFlagOptions)
                {
                    const int contextAttribs[] = {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, version[0],
                        WGL_CONTEXT_MINOR_VERSION_ARB, version[1],
                        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                        WGL_CONTEXT_FLAGS_ARB, contextFlags,
                        0};

                    m_hglrc = wglCreateContextAttribsARB(m_hdc, nullptr, contextAttribs);
                    if (!m_hglrc)
                    {
                        continue;
                    }

                    // Success! Clean up temporary context
                    wglMakeCurrent(nullptr, nullptr);
                    wglDeleteContext(tempContext);

                    // Make the new context current
                    if (!wglMakeCurrent(m_hdc, m_hglrc))
                    {
                        wglDeleteContext(m_hglrc);
                        m_hglrc = nullptr;
                        return false;
                    }

                    // Initialize GLAD with the new context
                    if (!gladLoadGL())
                    {
                        wglMakeCurrent(nullptr, nullptr);
                        wglDeleteContext(m_hglrc);
                        m_hglrc = nullptr;
                        return false;
                    }

#ifndef NDEBUG
                    if ((contextFlags & WGL_CONTEXT_DEBUG_BIT_ARB) == 0)
                    {
                        PYRAMID_LOG_WARN(
                            "Driver rejected a debug OpenGL context; continuing with a standard core context");
                    }
#endif

                    // Enhanced OpenGL context information logging
                    LogOpenGLContextInfo();

                    return true;
                }
            }
        }

        PYRAMID_LOG_ERROR("OpenGL 3.3 core or newer is required");
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(tempContext);
        return false;
    }

    void Win32OpenGLWindow::Present(bool vsync)
    {
        if (wglSwapIntervalEXT)
            wglSwapIntervalEXT(vsync ? 1 : 0);

        if (m_hdc)
            SwapBuffers(m_hdc);
    }

    void Win32OpenGLWindow::MakeContextCurrent()
    {
        if (m_hdc && m_hglrc)
            wglMakeCurrent(m_hdc, m_hglrc);
    }

    bool Win32OpenGLWindow::SetText(std::u32string_view text, std::string* error)
    {
        if (error)
        {
            error->clear();
        }

        std::u32string normalized;
        normalized.reserve(text.size());
        for (std::size_t index = 0; index < text.size(); ++index)
        {
            const char32_t codepoint = text[index];
            if (codepoint == U'\r')
            {
                if (index + 1U < text.size() && text[index + 1U] == U'\n')
                {
                    ++index;
                }
                normalized.push_back(U'\n');
            }
            else
            {
                normalized.push_back(codepoint);
            }
        }

        std::u32string windowsText;
        windowsText.reserve(normalized.size() + normalized.size() / 8U);
        for (char32_t codepoint : normalized)
        {
            if (codepoint == U'\n')
            {
                windowsText.push_back(U'\r');
            }
            windowsText.push_back(codepoint);
        }

        std::u16string utf16;
        if (!ClipboardEncoding::Utf32ToUtf16(windowsText, utf16, 4U * 1024U * 1024U, error))
        {
            return false;
        }
        utf16.push_back(u'\0');

        const SIZE_T bytes = utf16.size() * sizeof(char16_t);
        HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (!memory)
        {
            if (error)
            {
                *error = "GlobalAlloc failed for clipboard text";
            }
            return false;
        }

        void* destination = GlobalLock(memory);
        if (!destination)
        {
            if (error)
            {
                *error = "GlobalLock failed for clipboard text";
            }
            GlobalFree(memory);
            return false;
        }
        std::memcpy(destination, utf16.data(), bytes);
        GlobalUnlock(memory);

        if (!OpenClipboard(m_hwnd))
        {
            if (error)
            {
                *error = "OpenClipboard failed";
            }
            GlobalFree(memory);
            return false;
        }

        if (!EmptyClipboard())
        {
            if (error)
            {
                *error = "EmptyClipboard failed";
            }
            GlobalFree(memory);
            CloseClipboard();
            return false;
        }

        if (!SetClipboardData(CF_UNICODETEXT, memory))
        {
            if (error)
            {
                *error = "SetClipboardData failed";
            }
            GlobalFree(memory);
            CloseClipboard();
            return false;
        }

        CloseClipboard();
        return true;
    }

    ClipboardTextResult Win32OpenGLWindow::GetText() const
    {
        ClipboardTextResult result;
        if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
        {
            result.error = "clipboard does not contain Unicode text";
            return result;
        }
        if (!OpenClipboard(m_hwnd))
        {
            result.error = "OpenClipboard failed";
            return result;
        }

        HANDLE handle = GetClipboardData(CF_UNICODETEXT);
        if (!handle)
        {
            result.error = "GetClipboardData failed";
            CloseClipboard();
            return result;
        }

        const SIZE_T byteSize = GlobalSize(handle);
        const auto* data = static_cast<const char16_t*>(GlobalLock(handle));
        if (!data || byteSize < sizeof(char16_t))
        {
            result.error = "clipboard Unicode text is unavailable";
            if (data)
            {
                GlobalUnlock(handle);
            }
            CloseClipboard();
            return result;
        }

        const std::size_t maximumUnits = byteSize / sizeof(char16_t);
        std::size_t length = 0;
        while (length < maximumUnits && data[length] != u'\0')
        {
            ++length;
        }
        if (length == maximumUnits)
        {
            result.error = "clipboard Unicode text is not terminated";
            GlobalUnlock(handle);
            CloseClipboard();
            return result;
        }

        std::u32string decoded;
        std::string conversionError;
        const bool converted = ClipboardEncoding::Utf16ToUtf32(
            std::u16string_view(data, length),
            decoded,
            4U * 1024U * 1024U,
            &conversionError);
        GlobalUnlock(handle);
        CloseClipboard();
        if (!converted)
        {
            result.error = conversionError;
            return result;
        }

        result.text.reserve(decoded.size());
        for (std::size_t index = 0; index < decoded.size(); ++index)
        {
            if (decoded[index] == U'\r')
            {
                if (index + 1U < decoded.size() && decoded[index + 1U] == U'\n')
                {
                    ++index;
                }
                result.text.push_back(U'\n');
            }
            else
            {
                result.text.push_back(decoded[index]);
            }
        }
        result.success = true;
        return result;
    }

    bool Win32OpenGLWindow::ProcessMessages()
    {
        m_input.BeginFrame();

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

    void Win32OpenGLWindow::SetTitle(const char* title)
    {
        if (!m_hwnd || !title)
            return;

        const int length = static_cast<int>(strlen(title));
        if (length <= 0)
            return;

        const int wideLength = MultiByteToWideChar(CP_UTF8, 0, title, length, nullptr, 0);
        if (wideLength <= 0)
            return;

        std::vector<wchar_t> buffer(static_cast<size_t>(wideLength) + 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, title, length, buffer.data(), wideLength);
        SetWindowTextW(m_hwnd, buffer.data());
    }

    void Win32OpenGLWindow::SetSize(int width, int height)
    {
        if (!m_hwnd || width <= 0 || height <= 0)
            return;

        RECT rect = {0, 0, width, height};
        const DWORD style = static_cast<DWORD>(GetWindowLongPtrW(m_hwnd, GWL_STYLE));
        AdjustWindowRect(&rect, style, FALSE);
        SetWindowPos(
            m_hwnd,
            nullptr,
            0,
            0,
            rect.right - rect.left,
            rect.bottom - rect.top,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void Win32OpenGLWindow::SetPosition(int x, int y)
    {
        if (m_hwnd)
            SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    void Win32OpenGLWindow::SetVisible(bool visible)
    {
        if (m_hwnd)
            ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
    }

    bool Win32OpenGLWindow::IsMinimized() const
    {
        return m_hwnd && IsIconic(m_hwnd) != FALSE;
    }

    bool Win32OpenGLWindow::IsMaximized() const
    {
        return m_hwnd && IsZoomed(m_hwnd) != FALSE;
    }

    void Win32OpenGLWindow::LogOpenGLContextInfo()
    {
        // Basic OpenGL information
        const char *version_str = reinterpret_cast<const char *>(glGetString(GL_VERSION));
        const char *renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
        const char *vendor = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
        const char *glsl_version = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

        PYRAMID_LOG_INFO("=== OpenGL Context Information ===");
        PYRAMID_LOG_INFO("OpenGL Version: ", version_str ? version_str : "Unknown");
        PYRAMID_LOG_INFO("Renderer: ", renderer ? renderer : "Unknown");
        PYRAMID_LOG_INFO("Vendor: ", vendor ? vendor : "Unknown");
        PYRAMID_LOG_INFO("GLSL Version: ", glsl_version ? glsl_version : "Unknown");

        // Get OpenGL version numbers
        GLint major, minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
        PYRAMID_LOG_INFO("OpenGL Version (parsed): ", major, ".", minor);

        // Check for key OpenGL 4.6 features
        if (major >= 4)
        {
            PYRAMID_LOG_INFO("=== OpenGL 4.x Features Available ===");

            // Check for specific features that are important for our engine
            if (major > 4 || (major == 4 && minor >= 6))
            {
                PYRAMID_LOG_INFO("✅ OpenGL 4.6 features available");
                PYRAMID_LOG_INFO("  - SPIR-V shader support");
                PYRAMID_LOG_INFO("  - Anisotropic filtering");
                PYRAMID_LOG_INFO("  - Polygon offset clamp");
            }

            if (major > 4 || (major == 4 && minor >= 5))
            {
                PYRAMID_LOG_INFO("✅ OpenGL 4.5 features available");
                PYRAMID_LOG_INFO("  - Direct State Access (DSA)");
                PYRAMID_LOG_INFO("  - Clip control");
            }

            if (major > 4 || (major == 4 && minor >= 4))
            {
                PYRAMID_LOG_INFO("✅ OpenGL 4.4 features available");
                PYRAMID_LOG_INFO("  - Buffer storage");
                PYRAMID_LOG_INFO("  - Multi-bind");
            }

            if (major > 4 || (major == 4 && minor >= 3))
            {
                PYRAMID_LOG_INFO("✅ OpenGL 4.3 features available");
                PYRAMID_LOG_INFO("  - Compute shaders");
                PYRAMID_LOG_INFO("  - Shader storage buffer objects");
                PYRAMID_LOG_INFO("  - Multi-draw indirect");
            }
        }

        // Check hardware limits important for our engine
        GLint max_texture_size, max_texture_units, max_uniform_buffer_bindings;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
        glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_uniform_buffer_bindings);

        PYRAMID_LOG_INFO("=== Hardware Limits ===");
        PYRAMID_LOG_INFO("Max Texture Size: ", max_texture_size, "x", max_texture_size);
        PYRAMID_LOG_INFO("Max Texture Units: ", max_texture_units);
        PYRAMID_LOG_INFO("Max UBO Bindings: ", max_uniform_buffer_bindings);

        PYRAMID_LOG_INFO("===================================");
    }

} // namespace Pyramid
