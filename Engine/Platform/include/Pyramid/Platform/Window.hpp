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
};

} // namespace Pyramid
