#pragma once
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <glad/glad.h>
#include <string>
#include <unordered_map>

namespace Pyramid {

/**
 * @brief OpenGL implementation of shader program
 */
class OpenGLShader : public IShader {
public:
    OpenGLShader();
    ~OpenGLShader() override;

    void Bind() override;
    void Unbind() override;
    bool Compile(const std::string& vertexSrc, const std::string& fragmentSrc) override;

    // Uniform setters
    void SetUniformInt(const std::string& name, int value) override;
    void SetUniformFloat(const std::string& name, float value) override;
    void SetUniformFloat2(const std::string& name, float v0, float v1) override;
    void SetUniformFloat3(const std::string& name, float v0, float v1, float v2) override;
    void SetUniformFloat4(const std::string& name, float v0, float v1, float v2, float v3) override;
    void SetUniformMat3(const std::string& name, const float* matrix_ptr, bool transpose = false, int count = 1) override;
    void SetUniformMat4(const std::string& name, const float* matrix_ptr, bool transpose = false, int count = 1) override;

private:
    GLint GetUniformLocation(const std::string& name); // Not const as it modifies cache

    GLuint m_programId;
    std::unordered_map<std::string, GLint> m_uniformLocationCache;
};

} // namespace Pyramid
