#pragma once
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <glad/glad.h>

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

private:
    GLuint m_programId;
};

} // namespace Pyramid
