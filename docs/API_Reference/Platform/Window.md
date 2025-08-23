# Window System API Reference

## Overview

The Pyramid Engine's Window System provides platform-abstracted window management with support for OpenGL context creation, event handling, and cross-platform window operations.

## Classes

### Window Interface

The base window interface for platform abstraction:

```cpp
class Window {
public:
    virtual ~Window() = default;
    
    // Window lifecycle
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void ProcessEvents() = 0;
    
    // OpenGL context management
    virtual void MakeContextCurrent() = 0;
    virtual void Present(bool vsync) = 0;
    
    // Window properties
    virtual u32 GetWidth() const = 0;
    virtual u32 GetHeight() const = 0;
    virtual void SetTitle(const std::string& title) = 0;
    virtual bool ShouldClose() const = 0;
    
    // Window state
    virtual void SetSize(u32 width, u32 height) = 0;
    virtual void SetPosition(i32 x, i32 y) = 0;
    virtual void Minimize() = 0;
    virtual void Maximize() = 0;
    virtual void Restore() = 0;
    virtual void SetFullscreen(bool fullscreen) = 0;
    
    // Input integration
    virtual void SetCursorMode(CursorMode mode) = 0;
    virtual Math::Vec2 GetCursorPosition() const = 0;
    virtual void SetCursorPosition(const Math::Vec2& position) = 0;
};
```

### Win32OpenGLWindow

Windows-specific OpenGL window implementation:

```cpp
class Win32OpenGLWindow : public Window {
public:
    Win32OpenGLWindow();
    ~Win32OpenGLWindow() override;
    
    // Window interface implementation
    bool Initialize() override;
    void Shutdown() override;
    void ProcessEvents() override;
    
    void MakeContextCurrent() override;
    void Present(bool vsync) override;
    
    u32 GetWidth() const override;
    u32 GetHeight() const override;
    void SetTitle(const std::string& title) override;
    bool ShouldClose() const override;
    
    void SetSize(u32 width, u32 height) override;
    void SetPosition(i32 x, i32 y) override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    void SetFullscreen(bool fullscreen) override;
    
    void SetCursorMode(CursorMode mode) override;
    Math::Vec2 GetCursorPosition() const override;
    void SetCursorPosition(const Math::Vec2& position) override;
    
    // Windows-specific methods
    HWND GetWindowHandle() const { return m_windowHandle; }
    HDC GetDeviceContext() const { return m_deviceContext; }
    HGLRC GetRenderContext() const { return m_renderContext; }
    
private:
    // Windows-specific implementation
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    bool CreateWindowClass();
    bool CreateWindow();
    bool CreateOpenGLContext();
    void DestroyOpenGLContext();
    
    HWND m_windowHandle = nullptr;
    HDC m_deviceContext = nullptr;
    HGLRC m_renderContext = nullptr;
    HINSTANCE m_instance = nullptr;
    std::string m_className = "PyramidEngineWindow";
    std::string m_title = "Pyramid Engine";
    u32 m_width = 1280;
    u32 m_height = 720;
    bool m_shouldClose = false;
    bool m_fullscreen = false;
};
```

## Enumerations

### CursorMode

Cursor display and behavior modes:

```cpp
enum class CursorMode {
    Normal,     // Cursor visible and free to move
    Hidden,     // Cursor hidden but free to move
    Disabled    // Cursor hidden and locked to window center
};
```

### WindowState

Window state enumeration:

```cpp
enum class WindowState {
    Normal,
    Minimized,
    Maximized,
    Fullscreen
};
```

## Events

The window system generates various events for application handling:

### WindowEvent

Base window event structure:

```cpp
struct WindowEvent {
    enum Type {
        Resize,
        Close,
        Focus,
        LostFocus,
        Minimize,
        Maximize,
        Restore,
        Move
    };
    
    Type type;
    union {
        struct { u32 width, height; } resize;
        struct { i32 x, y; } move;
    } data;
};
```

## Usage Examples

### Basic Window Creation

```cpp
#include <Pyramid/Platform/Windows/Win32OpenGLWindow.hpp>

// Create and initialize window
auto window = std::make_unique<Win32OpenGLWindow>();
if (!window->Initialize()) {
    PYRAMID_LOG_ERROR("Failed to initialize window");
    return false;
}

// Set window properties
window->SetTitle("My Game");
window->SetSize(1920, 1080);

// Main loop
while (!window->ShouldClose()) {
    // Process window events
    window->ProcessEvents();
    
    // Make OpenGL context current
    window->MakeContextCurrent();
    
    // Render frame
    RenderFrame();
    
    // Present frame
    window->Present(true);  // Enable VSync
}

// Cleanup
window->Shutdown();
```

### Fullscreen Toggle

```cpp
void ToggleFullscreen(Window* window) {
    static bool isFullscreen = false;
    static u32 windowedWidth = 1280;
    static u32 windowedHeight = 720;
    static i32 windowedX = 100;
    static i32 windowedY = 100;
    
    if (!isFullscreen) {
        // Save windowed state
        windowedWidth = window->GetWidth();
        windowedHeight = window->GetHeight();
        // windowedX and windowedY would need to be saved via additional API
        
        // Set fullscreen
        window->SetFullscreen(true);
        isFullscreen = true;
        
        PYRAMID_LOG_INFO("Switched to fullscreen mode");
    } else {
        // Restore windowed state
        window->SetFullscreen(false);
        window->SetSize(windowedWidth, windowedHeight);
        window->SetPosition(windowedX, windowedY);
        isFullscreen = false;
        
        PYRAMID_LOG_INFO("Switched to windowed mode");
    }
}
```

### Window Event Handling

```cpp
class WindowEventHandler {
public:
    void HandleWindowEvent(const WindowEvent& event) {
        switch (event.type) {
            case WindowEvent::Resize:
                OnWindowResize(event.data.resize.width, event.data.resize.height);
                break;
                
            case WindowEvent::Close:
                OnWindowClose();
                break;
                
            case WindowEvent::Focus:
                OnWindowFocus();
                break;
                
            case WindowEvent::LostFocus:
                OnWindowLostFocus();
                break;
                
            case WindowEvent::Minimize:
                OnWindowMinimize();
                break;
                
            case WindowEvent::Maximize:
                OnWindowMaximize();
                break;
        }
    }
    
private:
    void OnWindowResize(u32 width, u32 height) {
        PYRAMID_LOG_INFO("Window resized to ", width, "x", height);
        
        // Update viewport
        glViewport(0, 0, width, height);
        
        // Update camera aspect ratio
        if (m_camera) {
            float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
            m_camera->SetAspectRatio(aspectRatio);
        }
        
        // Update framebuffers
        if (m_framebuffer) {
            m_framebuffer->Resize(width, height);
        }
    }
    
    void OnWindowClose() {
        PYRAMID_LOG_INFO("Window close requested");
        // Initiate graceful shutdown
        Game::GetInstance()->Quit();
    }
    
    void OnWindowFocus() {
        PYRAMID_LOG_DEBUG("Window gained focus");
        // Resume game logic if paused
        m_gamePaused = false;
        
        // Resume audio
        AudioManager::ResumeAll();
    }
    
    void OnWindowLostFocus() {
        PYRAMID_LOG_DEBUG("Window lost focus");
        // Pause game logic
        m_gamePaused = true;
        
        // Pause audio
        AudioManager::PauseAll();
    }
    
    Camera* m_camera = nullptr;
    Framebuffer* m_framebuffer = nullptr;
    bool m_gamePaused = false;
};
```

### Cursor Management

```cpp
class CursorManager {
public:
    void SetGameMode(Window* window, bool gameMode) {
        if (gameMode) {
            // Hide and lock cursor for FPS-style games
            window->SetCursorMode(CursorMode::Disabled);
            m_gameMode = true;
            PYRAMID_LOG_DEBUG("Cursor disabled for game mode");
        } else {
            // Show cursor for menu navigation
            window->SetCursorMode(CursorMode::Normal);
            m_gameMode = false;
            PYRAMID_LOG_DEBUG("Cursor enabled for menu mode");
        }
    }
    
    void UpdateCursorForUI(Window* window, bool overUI) {
        if (m_gameMode) return;  // Don't change cursor in game mode
        
        if (overUI) {
            // Show interactive cursor
            window->SetCursorMode(CursorMode::Normal);
        } else {
            // Could set different cursor states here
            window->SetCursorMode(CursorMode::Normal);
        }
    }
    
    Math::Vec2 GetRelativeMouseMovement(Window* window) {
        if (!m_gameMode) return Math::Vec2::Zero;
        
        Math::Vec2 currentPos = window->GetCursorPosition();
        Math::Vec2 centerPos(window->GetWidth() / 2.0f, window->GetHeight() / 2.0f);
        
        Math::Vec2 delta = currentPos - centerPos;
        
        // Reset cursor to center for next frame
        window->SetCursorPosition(centerPos);
        
        return delta;
    }
    
private:
    bool m_gameMode = false;
};
```

### Multi-Monitor Support

```cpp
class MonitorManager {
public:
    struct MonitorInfo {
        std::string name;
        u32 width;
        u32 height;
        i32 x;
        i32 y;
        bool isPrimary;
    };
    
    static std::vector<MonitorInfo> GetMonitors() {
        std::vector<MonitorInfo> monitors;
        
        #ifdef _WIN32
        // Windows implementation
        EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 
                           reinterpret_cast<LPARAM>(&monitors));
        #endif
        
        return monitors;
    }
    
    static MonitorInfo GetPrimaryMonitor() {
        auto monitors = GetMonitors();
        for (const auto& monitor : monitors) {
            if (monitor.isPrimary) {
                return monitor;
            }
        }
        
        // Fallback
        return {"Primary", 1920, 1080, 0, 0, true};
    }
    
    static void MoveWindowToMonitor(Window* window, const MonitorInfo& monitor) {
        // Center window on the specified monitor
        u32 windowWidth = window->GetWidth();
        u32 windowHeight = window->GetHeight();
        
        i32 centerX = monitor.x + (monitor.width - windowWidth) / 2;
        i32 centerY = monitor.y + (monitor.height - windowHeight) / 2;
        
        window->SetPosition(centerX, centerY);
        
        PYRAMID_LOG_INFO("Moved window to monitor: ", monitor.name);
    }
    
private:
    #ifdef _WIN32
    static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, 
                                        LPRECT lprcMonitor, LPARAM dwData) {
        auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
        
        MONITORINFOEX monitorInfo;
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        
        if (GetMonitorInfo(hMonitor, &monitorInfo)) {
            MonitorInfo info;
            info.name = monitorInfo.szDevice;
            info.width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
            info.height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
            info.x = monitorInfo.rcMonitor.left;
            info.y = monitorInfo.rcMonitor.top;
            info.isPrimary = (monitorInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
            
            monitors->push_back(info);
        }
        
        return TRUE;
    }
    #endif
};
```

## Platform-Specific Features

### Windows (Win32) Implementation

The Win32 implementation provides:

- **Native Window Creation**: Direct Win32 API window creation
- **OpenGL Context Management**: WGL-based OpenGL context creation and management
- **Event Processing**: Native Windows message processing
- **Multi-Monitor Support**: Full multi-monitor awareness and management
- **High-DPI Support**: Automatic high-DPI scaling support

### Planned Platform Support

#### Linux (X11/Wayland)
- X11 window management
- Wayland protocol support
- OpenGL context creation via GLX
- XInput event handling

#### macOS (Cocoa)
- Cocoa window management
- OpenGL context creation via NSOpenGLContext
- Cocoa event handling
- Retina display support

## Integration with Graphics System

The window system integrates seamlessly with the graphics device:

```cpp
// Window creation and graphics device initialization
auto window = std::make_unique<Win32OpenGLWindow>();
window->Initialize();

// Graphics device uses the window for context creation
auto graphicsDevice = std::make_unique<OpenGLDevice>(window.get());
graphicsDevice->Initialize();

// Window provides context management
window->MakeContextCurrent();

// Rendering operations...
graphicsDevice->Clear(Color::Black);
graphicsDevice->Present();

// Window handles buffer swapping
window->Present(true);  // VSync enabled
```

## Performance Considerations

### Event Processing
- **Efficient Message Handling**: Optimized Windows message processing
- **Event Batching**: Multiple events processed per frame when needed
- **Minimal Overhead**: Direct platform API usage for best performance

### Context Management
- **Context Caching**: OpenGL context switching minimized
- **VSync Control**: Fine-grained VSync control for different scenarios
- **Resource Sharing**: Shared contexts for multi-threaded rendering (planned)

## Error Handling

The window system provides comprehensive error handling:

```cpp
// Window creation with error checking
auto window = std::make_unique<Win32OpenGLWindow>();
if (!window->Initialize()) {
    PYRAMID_LOG_ERROR("Window initialization failed");
    
    // Get specific error information (Windows)
    #ifdef _WIN32
    DWORD error = GetLastError();
    PYRAMID_LOG_ERROR("Windows error code: ", error);
    #endif
    
    return false;
}
```

## See Also

- [Core Game Class](../Core/Game.md) - Main game loop integration
- [Graphics Device](../Graphics/GraphicsDevice.md) - OpenGL context integration
- [Input System](../Input/InputSystem.md) - Window event integration
- [Platform Abstraction](Platform.md) - Cross-platform considerations
