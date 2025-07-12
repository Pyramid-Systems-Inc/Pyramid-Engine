#include <Pyramid/Graphics/OpenGL/OpenGLDevice.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp> // For BufferUsage enum
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLIndexBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexArray.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLUniformBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/Shader/OpenGLShader.hpp>
#include <Pyramid/Graphics/Texture.hpp> // Added for ITexture2D factory methods
#include <glad/glad.h>

namespace Pyramid
{

    OpenGLDevice::OpenGLDevice(Window *window) // Changed
        : m_window(window)                     // Changed
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
        // m_window.reset(); // Removed as OpenGLDevice no longer owns the window
    }

    void OpenGLDevice::Clear(const Color &color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLDevice::Present(bool vsync)
    {
        m_window->Present(vsync);
    }

    void OpenGLDevice::DrawIndexed(u32 count)
    {
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLDevice::SetViewport(u32 x, u32 y, u32 width, u32 height)
    {
        glViewport(x, y, width, height);
    }

    std::shared_ptr<IVertexBuffer> OpenGLDevice::CreateVertexBuffer()
    {
        return std::make_shared<OpenGLVertexBuffer>();
    }

    std::shared_ptr<IIndexBuffer> OpenGLDevice::CreateIndexBuffer()
    {
        return std::make_shared<OpenGLIndexBuffer>();
    }

    std::shared_ptr<IVertexArray> OpenGLDevice::CreateVertexArray()
    {
        return std::make_shared<OpenGLVertexArray>();
    }

    std::shared_ptr<IShader> OpenGLDevice::CreateShader()
    {
        return std::make_shared<OpenGLShader>();
    }

    std::shared_ptr<ITexture2D> OpenGLDevice::CreateTexture2D(const TextureSpecification &specification, const void *data)
    {
        return ITexture2D::Create(specification, data);
    }

    std::shared_ptr<ITexture2D> OpenGLDevice::CreateTexture2D(const std::string &filepath, bool srgb, bool generateMips)
    {
        return ITexture2D::Create(filepath, srgb, generateMips);
    }

    std::shared_ptr<IUniformBuffer> OpenGLDevice::CreateUniformBuffer(size_t size, BufferUsage usage)
    {
        auto buffer = std::make_shared<OpenGLUniformBuffer>();
        if (buffer->Initialize(size, usage))
        {
            return buffer;
        }
        return nullptr;
    }

} // namespace Pyramid
