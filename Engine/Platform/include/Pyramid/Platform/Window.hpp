#pragma once
#include <Pyramid/Core/Prerequisites.hpp>

namespace Pyramid {

/**
 * @brief Interface for window management
 */
class Window {
public:
    virtual ~Window() = default;

    /**
     * @brief Initialize the window
     * @param title Window title
     * @param width Window width
     * @param height Window height
     * @return true if successful
     */
    virtual bool Initialize(const char* title = "Pyramid Game", int width = 800, int height = 600) = 0;

    /**
     * @brief Present the window contents
     * @param vsync Enable/disable vertical sync
     */
    virtual void Present(bool vsync) = 0;

    /**
     * @brief Make the graphics context current
     */
    virtual void MakeContextCurrent() = 0;

    /**
     * @brief Process window messages
     * @return true if the window should continue running, false if it should close
     */
    virtual bool ProcessMessages() = 0;

    /**
     * @brief Get the window handle
     * @return Platform-specific window handle
     */
    virtual void* GetHandle() const = 0;

    /**
     * @brief Get the window width
     * @return Window width in pixels
     */
    virtual int GetWidth() const = 0;

    /**
     * @brief Get the window height
     * @return Window height in pixels
     */
    virtual int GetHeight() const = 0;

    /**
     * @brief Check if the window should close
     * @return true if close was requested
     */
    virtual bool ShouldClose() const { return false; }

    /**
     * @brief Set the window title
     * @param title New window title
     */
    virtual void SetTitle(const char* title) {}

    /**
     * @brief Set the window size
     * @param width New width
     * @param height New height
     */
    virtual void SetSize(int width, int height) {}

    /**
     * @brief Set the window position
     * @param x New x position
     * @param y New y position
     */
    virtual void SetPosition(int x, int y) {}

    /**
     * @brief Show or hide the window
     * @param visible true to show, false to hide
     */
    virtual void SetVisible(bool visible) {}

    /**
     * @brief Check if the window is minimized
     * @return true if minimized
     */
    virtual bool IsMinimized() const { return false; }

    /**
     * @brief Check if the window is maximized
     * @return true if maximized
     */
    virtual bool IsMaximized() const { return false; }
};

} // namespace Pyramid
