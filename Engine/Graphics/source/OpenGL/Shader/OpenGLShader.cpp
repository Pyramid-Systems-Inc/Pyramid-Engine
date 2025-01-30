#include <Pyramid/Graphics/OpenGL/Shader/OpenGLShader.hpp>
#include <vector>
#include <iostream>

namespace Pyramid {

OpenGLShader::OpenGLShader()
    : m_programId(0)
{
}

OpenGLShader::~OpenGLShader()
{
    if (m_programId)
        glDeleteProgram(m_programId);
}

void OpenGLShader::Bind()
{
    glUseProgram(m_programId);
}

void OpenGLShader::Unbind()
{
    glUseProgram(0);
}

bool OpenGLShader::Compile(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    // Create shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile vertex shader
    const char* source = vertexSrc.c_str();
    glShaderSource(vertexShader, 1, &source, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled)
    {
        GLint maxLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

        glDeleteShader(vertexShader);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog.data() << std::endl;
        return false;
    }

    // Compile fragment shader
    source = fragmentSrc.c_str();
    glShaderSource(fragmentShader, 1, &source, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (!isCompiled)
    {
        GLint maxLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog.data() << std::endl;
        return false;
    }

    // Create program and link shaders
    m_programId = glCreateProgram();
    glAttachShader(m_programId, vertexShader);
    glAttachShader(m_programId, fragmentShader);
    glLinkProgram(m_programId);

    // Check program linking
    GLint isLinked = 0;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &isLinked);
    if (!isLinked)
    {
        GLint maxLength = 0;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(m_programId, maxLength, &maxLength, &infoLog[0]);

        glDeleteProgram(m_programId);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        std::cerr << "Shader program linking failed:\n" << infoLog.data() << std::endl;
        return false;
    }

    // Cleanup
    glDetachShader(m_programId, vertexShader);
    glDetachShader(m_programId, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

} // namespace Pyramid
