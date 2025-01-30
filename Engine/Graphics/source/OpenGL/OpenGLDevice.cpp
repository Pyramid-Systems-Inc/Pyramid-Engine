#include <Pyramid/Graphics/OpenGL/OpenGLDevice.hpp>
#include <glad/glad.h>

namespace Pyramid {

OpenGLDevice::OpenGLDevice()
    : m_window(std::make_unique<Win32OpenGLWindow>())
{
}

OpenGLDevice::~OpenGLDevice()
{
    Shutdown();
}

bool OpenGLDevice::Initialize()
{
    // Initialize the window
    if (!m_window->Initialize())
        return false;

    // Make OpenGL context current
    m_window->MakeContextCurrent();

    return true;
}

void OpenGLDevice::Shutdown()
{
    m_window.reset();
}

void OpenGLDevice::Clear(const Color& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLDevice::Present(bool vsync)
{
    m_window->Present(vsync);
}

} // namespace Pyramid
