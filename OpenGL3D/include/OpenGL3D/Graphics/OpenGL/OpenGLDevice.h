#pragma once
#include <OpenGL3D/Graphics/IGraphicsDevice.h>
#include <OpenGL3D/Window/OglWindow.h>

namespace Pyramid {

/**
 * @brief OpenGL version enumeration
 */
enum class OpenGLVersion {
    GL_3_3,  // Minimum modern OpenGL
    GL_4_0,  // Current version
    GL_4_3,  // Added compute shaders
    GL_4_6   // Latest
};

/**
 * @brief OpenGL implementation of the graphics device interface
 */
class OpenGLDevice : public IGraphicsDevice {
public:
    OpenGLDevice();
    ~OpenGLDevice() override;

    bool Initialize() override;
    void Shutdown() override;
    void Clear(const Color& color) override;
    void Present(bool vsync) override;
    void MakeContextCurrent() override;
    GraphicsAPI GetAPIType() const override { return GraphicsAPI::OpenGL; }

    /**
     * @brief Set the desired OpenGL version
     * @param version The OpenGL version to use
     */
    void SetVersion(OpenGLVersion version) { m_RequestedVersion = version; }

    /**
     * @brief Get the current OpenGL version
     * @return The current OpenGL version
     */
    OpenGLVersion GetVersion() const { return m_CurrentVersion; }

private:
    bool InitializeOpenGL();
    bool CreateDeviceContext();
    void DetectOpenGLVersion();

private:
    std::unique_ptr<OglWindow> m_Window;
    OpenGLVersion m_RequestedVersion;
    OpenGLVersion m_CurrentVersion;
    bool m_IsInitialized;
};

} // namespace Pyramid
