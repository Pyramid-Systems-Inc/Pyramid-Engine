#pragma once
#include <Pyramid/Core/Prerequisites.hpp>
#include <string>

namespace Pyramid
{

    /**
     * @brief Interface for shader program implementations
     */
    class IShader
    {
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
        virtual bool Compile(const std::string &vertexSrc, const std::string &fragmentSrc) = 0;

        // Uniform setters
        virtual void SetUniformInt(const std::string &name, int value) = 0;
        virtual void SetUniformFloat(const std::string &name, float value) = 0;
        virtual void SetUniformFloat2(const std::string &name, float v0, float v1) = 0;
        virtual void SetUniformFloat3(const std::string &name, float v0, float v1, float v2) = 0;
        virtual void SetUniformFloat4(const std::string &name, float v0, float v1, float v2, float v3) = 0;
        // For matrices, it's common to pass a pointer to a float array.
        // OpenGL expects column-major matrices by default.
        // The 'count' parameter for glUniformMatrix*fv is for arrays of matrices, typically 1 for a single matrix.
        // 'transpose' is GL_FALSE if the matrix is already column-major.
        virtual void SetUniformMat3(const std::string &name, const float *matrix_ptr, bool transpose = false, int count = 1) = 0;
        virtual void SetUniformMat4(const std::string &name, const float *matrix_ptr, bool transpose = false, int count = 1) = 0;

        /**
         * @brief Bind a uniform buffer to a named uniform block
         * @param blockName The name of the uniform block in the shader
         * @param buffer The uniform buffer to bind
         * @param bindingPoint The binding point index
         */
        virtual void BindUniformBuffer(const std::string &blockName, class IUniformBuffer *buffer, u32 bindingPoint) = 0;

        /**
         * @brief Set the binding point for a uniform block
         * @param blockName The name of the uniform block
         * @param bindingPoint The binding point index
         */
        virtual void SetUniformBlockBinding(const std::string &blockName, u32 bindingPoint) = 0;

        // TODO: Add overloads for Vec2, Vec3, Vec4, Mat3, Mat4 types when math library is more complete.
    };

} // namespace Pyramid
