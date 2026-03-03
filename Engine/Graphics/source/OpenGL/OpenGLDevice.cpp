#include <Pyramid/Graphics/OpenGL/OpenGLDevice.hpp>
#include <Pyramid/Util/Log.hpp>                      // For logging
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp> // For BufferUsage enum
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLIndexBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLVertexArray.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLUniformBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLInstanceBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/Buffer/OpenGLShaderStorageBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLStateManager.hpp>
#include <Pyramid/Graphics/OpenGL/Shader/OpenGLShader.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLTexture.hpp>
#include <Pyramid/Graphics/Texture.hpp> // Added for ITexture2D factory methods
#include <glad/glad.h>
#include <sstream>

namespace Pyramid
{
    namespace
    {
        constexpr u32 kMaxTextureSlots = 32;

        const char *ToGlErrorName(GLenum errorCode)
        {
            switch (errorCode)
            {
            case GL_INVALID_ENUM:
                return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:
                return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION:
                return "GL_INVALID_OPERATION";
            case GL_STACK_OVERFLOW:
                return "GL_STACK_OVERFLOW";
            case GL_STACK_UNDERFLOW:
                return "GL_STACK_UNDERFLOW";
            case GL_OUT_OF_MEMORY:
                return "GL_OUT_OF_MEMORY";
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                return "GL_INVALID_FRAMEBUFFER_OPERATION";
            default:
                return "GL_UNKNOWN_ERROR";
            }
        }

        void DrainOpenGLErrors()
        {
            while (glGetError() != GL_NO_ERROR)
            {
            }
        }

        bool CaptureOpenGLError(const char *operation, std::string &lastError, bool logFailure = true)
        {
            const GLenum error = glGetError();
            if (error == GL_NO_ERROR)
            {
                return true;
            }

            std::ostringstream errorStream;
            errorStream << operation << " failed with OpenGL error 0x" << std::hex << error
                        << " (" << ToGlErrorName(error) << ")";
            lastError = errorStream.str();

            if (logFailure)
            {
                PYRAMID_LOG_ERROR(lastError);
            }

            return false;
        }
    } // namespace


    OpenGLDevice::OpenGLDevice(Window *window)
        : m_window(window), m_initialized(false), m_deviceInfoCached(false)
    {
    }

    OpenGLDevice::~OpenGLDevice()
    {
        Shutdown();
    }

    bool OpenGLDevice::Initialize()
    {
        PYRAMID_LOG_INFO("Initializing OpenGL graphics device...");

        if (m_initialized)
        {
            return true;
        }

        if (!m_window)
        {
            m_lastError = "Cannot initialize OpenGL device: window is null";
            PYRAMID_LOG_ERROR(m_lastError);
            return false;
        }

        // Initialize the window only if it does not already exist.
        if (!m_window->GetHandle() && !m_window->Initialize())
        {
            m_lastError = "Failed to initialize window";
            PYRAMID_LOG_ERROR("Window initialization failed in OpenGLDevice::Initialize()");
            return false;
        }

        PYRAMID_LOG_INFO("Window ready, setting up OpenGL context...");

        // Make OpenGL context current
        m_window->MakeContextCurrent();
        DrainOpenGLErrors();

        auto &stateManager = OpenGLStateManager::GetInstance();
        stateManager.InvalidateState();
        stateManager.ResetToDefaults();
        stateManager.SetViewport(0, 0, m_window->GetWidth(), m_window->GetHeight());

        if (!CaptureOpenGLError("OpenGLDevice::Initialize", m_lastError))
        {
            return false;
        }

        m_initialized = true;
        m_deviceInfoCached = false; // Force device info refresh
        m_lastError.clear();

        return true;
    }

    void OpenGLDevice::Shutdown()
    {
        if (m_initialized)
        {
            OpenGLStateManager::GetInstance().InvalidateState();
            DrainOpenGLErrors();

            m_initialized = false;
            m_deviceInfoCached = false;
            m_deviceInfo.clear();
            m_lastError.clear();
        }
    }

    void OpenGLDevice::Clear(const Color &color)
    {
        OpenGLStateManager::GetInstance().SetClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CaptureOpenGLError("OpenGLDevice::Clear", m_lastError, false);
    }

    void OpenGLDevice::Present(bool vsync)
    {
        if (!m_window)
        {
            m_lastError = "Cannot present frame: window is null";
            PYRAMID_LOG_ERROR(m_lastError);
            return;
        }

        m_window->Present(vsync);
    }

    void OpenGLDevice::DrawIndexed(u32 count)
    {
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        CaptureOpenGLError("OpenGLDevice::DrawIndexed", m_lastError, false);
    }

    void OpenGLDevice::DrawIndexedInstanced(u32 indexCount, u32 instanceCount)
    {
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
        CaptureOpenGLError("OpenGLDevice::DrawIndexedInstanced", m_lastError, false);
    }

    void OpenGLDevice::DrawArraysInstanced(u32 vertexCount, u32 instanceCount, u32 firstVertex)
    {
        glDrawArraysInstanced(GL_TRIANGLES, firstVertex, vertexCount, instanceCount);
        CaptureOpenGLError("OpenGLDevice::DrawArraysInstanced", m_lastError, false);
    }

    void OpenGLDevice::SetViewport(u32 x, u32 y, u32 width, u32 height)
    {
        OpenGLStateManager::GetInstance().SetViewport(x, y, width, height);
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

    std::shared_ptr<IInstanceBuffer> OpenGLDevice::CreateInstanceBuffer()
    {
        return std::make_shared<OpenGLInstanceBuffer>();
    }

    std::shared_ptr<IShaderStorageBuffer> OpenGLDevice::CreateShaderStorageBuffer()
    {
        return std::make_shared<OpenGLShaderStorageBuffer>();
    }

    void OpenGLDevice::EnableBlend(bool enable)
    {
        OpenGLStateManager::GetInstance().EnableBlend(enable);
    }

    void OpenGLDevice::SetBlendFunc(u32 sfactor, u32 dfactor)
    {
        OpenGLStateManager::GetInstance().SetBlendFunc(static_cast<GLenum>(sfactor), static_cast<GLenum>(dfactor));
    }

    void OpenGLDevice::EnableDepthTest(bool enable)
    {
        OpenGLStateManager::GetInstance().EnableDepthTest(enable);
    }

    void OpenGLDevice::SetDepthFunc(u32 func)
    {
        OpenGLStateManager::GetInstance().SetDepthFunc(static_cast<GLenum>(func));
    }

    void OpenGLDevice::EnableDepthClamp(bool enable)
    {
        OpenGLStateManager::GetInstance().EnableDepthClamp(enable);
    }

    void OpenGLDevice::EnableCullFace(bool enable)
    {
        OpenGLStateManager::GetInstance().EnableCullFace(enable);
    }

    void OpenGLDevice::SetCullFace(u32 mode)
    {
        OpenGLStateManager::GetInstance().SetCullFace(static_cast<GLenum>(mode));
    }

    void OpenGLDevice::SetClearColor(f32 r, f32 g, f32 b, f32 a)
    {
        OpenGLStateManager::GetInstance().SetClearColor(r, g, b, a);
    }

    u32 OpenGLDevice::GetStateChangeCount() const
    {
        return OpenGLStateManager::GetInstance().GetStateChangeCount();
    }

    void OpenGLDevice::ResetStateChangeCount()
    {
        OpenGLStateManager::GetInstance().ResetStateChangeCount();
    }

    std::string OpenGLDevice::GetDeviceInfo() const
    {
        if (!m_deviceInfoCached)
        {
            if (m_initialized)
            {
                const char *vendor = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
                const char *renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
                const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
                const char *glslVersion = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));

                m_deviceInfo = std::string("OpenGL Device Info:\n") +
                               "  Vendor: " + (vendor ? vendor : "Unknown") + "\n" +
                               "  Renderer: " + (renderer ? renderer : "Unknown") + "\n" +
                               "  Version: " + (version ? version : "Unknown") + "\n" +
                               "  GLSL Version: " + (glslVersion ? glslVersion : "Unknown");
            }
            else
            {
                m_deviceInfo = "OpenGL Device not initialized";
            }
            m_deviceInfoCached = true;
        }
        return m_deviceInfo;
    }

    bool OpenGLDevice::IsValid() const
    {
        return m_initialized && m_window && m_window->GetHandle();
    }

    std::string OpenGLDevice::GetLastError() const
    {
        return m_lastError;
    }

    void OpenGLDevice::SetWireframeMode(bool enable)
    {
        OpenGLStateManager::GetInstance().SetPolygonMode(enable ? GL_LINE : GL_FILL);
        CaptureOpenGLError("OpenGLDevice::SetWireframeMode", m_lastError, false);
    }

    void OpenGLDevice::SetPolygonMode(u32 mode)
    {
        OpenGLStateManager::GetInstance().SetPolygonMode(static_cast<GLenum>(mode));
        CaptureOpenGLError("OpenGLDevice::SetPolygonMode", m_lastError, false);
    }

    void OpenGLDevice::BindFramebuffer(IFramebuffer *framebuffer)
    {
        if (framebuffer)
        {
            // TODO: Implement when IFramebuffer interface is available
            m_lastError = "Framebuffer binding not yet implemented";
        }
        else
        {
            // Bind default framebuffer
            OpenGLStateManager::GetInstance().BindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void OpenGLDevice::BindFramebufferHandle(u32 framebufferId)
    {
        OpenGLStateManager::GetInstance().BindFramebuffer(GL_FRAMEBUFFER, framebufferId);
    }

    void OpenGLDevice::BindShader(IShader *shader)
    {
        if (shader)
        {
            shader->Bind();
        }
        else
        {
            OpenGLStateManager::GetInstance().UseProgram(0);
        }
    }

    void OpenGLDevice::BindVertexArray(IVertexArray *vao)
    {
        if (vao)
        {
            vao->Bind();
        }
        else
        {
            OpenGLStateManager::GetInstance().BindVertexArray(0);
        }
    }

    void OpenGLDevice::BindTexture(ITexture2D *texture, u32 slot)
    {
        if (slot >= kMaxTextureSlots)
        {
            m_lastError = "Texture slot out of range: " + std::to_string(slot);
            PYRAMID_LOG_ERROR(m_lastError);
            return;
        }

        if (texture)
        {
            // Cast to OpenGL texture to access OpenGL-specific binding
            OpenGLTexture2D *glTexture = dynamic_cast<OpenGLTexture2D *>(texture);
            if (glTexture)
            {
                OpenGLStateManager::GetInstance().BindTextureUnit(slot, GL_TEXTURE_2D, glTexture->GetRendererID());
            }
            else
            {
                m_lastError = "Failed to bind texture: invalid OpenGL texture";
                PYRAMID_LOG_ERROR(m_lastError);
            }
        }
        else
        {
            OpenGLStateManager::GetInstance().BindTextureUnit(slot, GL_TEXTURE_2D, 0);
        }
    }

    void OpenGLDevice::BindNativeTexture(u32 textureId, u32 slot, u32 target)
    {
        if (slot >= kMaxTextureSlots)
        {
            m_lastError = "Native texture slot out of range: " + std::to_string(slot);
            PYRAMID_LOG_ERROR(m_lastError);
            return;
        }

        OpenGLStateManager::GetInstance().BindTextureUnit(slot, static_cast<GLenum>(target), textureId);
    }

    void OpenGLDevice::SetTextureBorderColor(u32 textureId, u32 target, f32 r, f32 g, f32 b, f32 a)
    {
        const GLfloat borderColor[] = {r, g, b, a};
        OpenGLStateManager::GetInstance().BindTextureUnit(0, static_cast<GLenum>(target), textureId);
        glTexParameterfv(static_cast<GLenum>(target), GL_TEXTURE_BORDER_COLOR, borderColor);
        CaptureOpenGLError("OpenGLDevice::SetTextureBorderColor", m_lastError);
    }

    void OpenGLDevice::BindUniformBuffer(IUniformBuffer *buffer, u32 bindingPoint)
    {
        if (buffer)
        {
            buffer->Bind(bindingPoint);
        }
        else
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, 0);
            OpenGLStateManager::GetInstance().BindBuffer(GL_UNIFORM_BUFFER, 0);
        }
    }

    void OpenGLDevice::ClearBuffers(u32 clearMask)
    {
        glClear(static_cast<GLbitfield>(clearMask));
        CaptureOpenGLError("OpenGLDevice::ClearBuffers", m_lastError, false);
    }

} // namespace Pyramid
