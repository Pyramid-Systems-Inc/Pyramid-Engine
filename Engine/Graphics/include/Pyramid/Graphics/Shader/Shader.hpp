#pragma once
#include <Pyramid/Core/Prerequisites.hpp>
#include <string>

namespace Pyramid {

/**
 * @brief Interface for shader program implementations
 */
class IShader {
public:
    virtual ~IShader() = default;

    /**
     * @brief Bind the shader program for use
     */
    virtual void Bind() = 0;

    /**
     * @brief Unbind the shader program
     */
    virtual void Unbind() = 0;

    /**
     * @brief Compile shader from source code
     * @param vertexSrc Vertex shader source code
     * @param fragmentSrc Fragment shader source code
     * @return true if compilation was successful
     */
    virtual bool Compile(const std::string& vertexSrc, const std::string& fragmentSrc) = 0;
};

} // namespace Pyramid
