#pragma once
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Platform/Windows/Win32OpenGLWindow.hpp>

namespace Pyramid {

/**
 * @brief OpenGL implementation of the graphics device
 */
class OpenGLDevice : public IGraphicsDevice {
public:
    OpenGLDevice();
    virtual ~OpenGLDevice();

    bool Initialize() override;
    void Shutdown() override;
    void Clear(const Color& color) override;
    void Present(bool vsync) override;

    /**
     * @brief Get the OpenGL window
     * @return Win32OpenGLWindow* The window instance
     */
    Win32OpenGLWindow* GetWindow() const { return m_window.get(); }

private:
    std::unique_ptr<Win32OpenGLWindow> m_window;
};

} // namespace Pyramid
