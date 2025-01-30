#include <OpenGL3D/Graphics/OpenGL/OpenGLDevice.h>
#include <glad/glad.h>
#include <cassert>

namespace Pyramid {

OpenGLDevice::OpenGLDevice()
    : m_RequestedVersion(OpenGLVersion::GL_4_0)
    , m_CurrentVersion(OpenGLVersion::GL_4_0)
    , m_IsInitialized(false)
{
}

OpenGLDevice::~OpenGLDevice()
{
    Shutdown();
}

bool OpenGLDevice::Initialize()
{
    if (m_IsInitialized)
        return true;

    // Create window and OpenGL context
    m_Window = std::make_unique<OglWindow>();
    if (!m_Window)
        return false;

    // Initialize OpenGL
    if (!InitializeOpenGL())
        return false;

    m_IsInitialized = true;
    return true;
}

void OpenGLDevice::Shutdown()
{
    if (!m_IsInitialized)
        return;

    m_Window.reset();
    m_IsInitialized = false;
}

void OpenGLDevice::Clear(const Color& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLDevice::Present(bool vsync)
{
    if (m_Window)
        m_Window->present(vsync);
}

void OpenGLDevice::MakeContextCurrent()
{
    if (m_Window)
        m_Window->makeCurrentContext();
}

bool OpenGLDevice::InitializeOpenGL()
{
    // Make context current
    MakeContextCurrent();

    // Initialize GLAD
    if (!gladLoadGL())
        return false;

    // Detect OpenGL version
    DetectOpenGLVersion();

    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    return true;
}

void OpenGLDevice::DetectOpenGLVersion()
{
    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

    // Set current version based on detected version
    if (majorVersion >= 4) {
        if (minorVersion >= 6)
            m_CurrentVersion = OpenGLVersion::GL_4_6;
        else if (minorVersion >= 3)
            m_CurrentVersion = OpenGLVersion::GL_4_3;
        else
            m_CurrentVersion = OpenGLVersion::GL_4_0;
    }
    else {
        m_CurrentVersion = OpenGLVersion::GL_3_3;
    }
}

// Factory function implementation
std::unique_ptr<IGraphicsDevice> IGraphicsDevice::Create(GraphicsAPI api)
{
    switch (api)
    {
    case GraphicsAPI::OpenGL:
        return std::make_unique<OpenGLDevice>();
    default:
        return nullptr;
    }
}

} // namespace Pyramid
