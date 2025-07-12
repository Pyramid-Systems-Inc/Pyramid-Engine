#include <Pyramid/Graphics/OpenGL/Shader/OpenGLShader.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Util/Log.hpp> // Updated to use Utils logging system
#include <vector>
// #include <iostream> // No longer needed directly if using Log macros

namespace Pyramid
{

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

    bool OpenGLShader::Compile(const std::string &vertexSrc, const std::string &fragmentSrc)
    {
        // Create shaders
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile vertex shader
        const char *source = vertexSrc.c_str();
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
            PYRAMID_LOG_ERROR("Vertex shader compilation failed:\n", infoLog.data()); // Changed
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
            PYRAMID_LOG_ERROR("Fragment shader compilation failed:\n", infoLog.data()); // Changed
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
            PYRAMID_LOG_ERROR("Shader program linking failed:\n", infoLog.data()); // Changed
            return false;
        }

        // Cleanup
        glDetachShader(m_programId, vertexShader);
        glDetachShader(m_programId, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return true;
    }

    GLint OpenGLShader::GetUniformLocation(const std::string &name)
    {
        if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
            return m_uniformLocationCache[name];

        GLint location = glGetUniformLocation(m_programId, name.c_str());
        if (location == -1)
            PYRAMID_LOG_WARN("Uniform '", name, "' not found in shader program ", m_programId); // Changed

        m_uniformLocationCache[name] = location;
        return location;
    }

    void OpenGLShader::SetUniformInt(const std::string &name, int value)
    {
        Bind(); // Ensure shader is bound
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1i(location, value);
    }

    void OpenGLShader::SetUniformFloat(const std::string &name, float value)
    {
        Bind();
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform1f(location, value);
    }

    void OpenGLShader::SetUniformFloat2(const std::string &name, float v0, float v1)
    {
        Bind();
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform2f(location, v0, v1);
    }

    void OpenGLShader::SetUniformFloat3(const std::string &name, float v0, float v1, float v2)
    {
        Bind();
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform3f(location, v0, v1, v2);
    }

    void OpenGLShader::SetUniformFloat4(const std::string &name, float v0, float v1, float v2, float v3)
    {
        Bind();
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniform4f(location, v0, v1, v2, v3);
    }

    void OpenGLShader::SetUniformMat3(const std::string &name, const float *matrix_ptr, bool transpose, int count)
    {
        Bind();
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix3fv(location, count, transpose ? GL_TRUE : GL_FALSE, matrix_ptr);
    }

    void OpenGLShader::SetUniformMat4(const std::string &name, const float *matrix_ptr, bool transpose, int count)
    {
        Bind();
        GLint location = GetUniformLocation(name);
        if (location != -1)
            glUniformMatrix4fv(location, count, transpose ? GL_TRUE : GL_FALSE, matrix_ptr);
    }

    void OpenGLShader::BindUniformBuffer(const std::string &blockName, IUniformBuffer *buffer, u32 bindingPoint)
    {
        if (!buffer)
        {
            PYRAMID_LOG_ERROR("Uniform buffer is null");
            return;
        }

        // Set the uniform block binding point
        SetUniformBlockBinding(blockName, bindingPoint);

        // Bind the buffer to the binding point
        buffer->Bind(bindingPoint);
    }

    void OpenGLShader::SetUniformBlockBinding(const std::string &blockName, u32 bindingPoint)
    {
        if (m_programId == 0)
        {
            PYRAMID_LOG_ERROR("Shader program not compiled");
            return;
        }

        // Get the uniform block index
        GLuint blockIndex = glGetUniformBlockIndex(m_programId, blockName.c_str());
        if (blockIndex == GL_INVALID_INDEX)
        {
            PYRAMID_LOG_WARN("Uniform block '", blockName, "' not found in shader program ", m_programId);
            return;
        }

        // Set the binding point for the uniform block
        glUniformBlockBinding(m_programId, blockIndex, bindingPoint);

        PYRAMID_LOG_INFO("Bound uniform block '", blockName, "' to binding point ", bindingPoint);
    }

} // namespace Pyramid
