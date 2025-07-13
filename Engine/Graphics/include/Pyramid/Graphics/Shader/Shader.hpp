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

        /**
         * @brief Compile shader from source code with geometry shader
         * @param vertexSrc Vertex shader source code
         * @param geometrySrc Geometry shader source code
         * @param fragmentSrc Fragment shader source code
         * @return true if compilation was successful
         */
        virtual bool CompileWithGeometry(const std::string &vertexSrc, const std::string &geometrySrc, const std::string &fragmentSrc) = 0;

        /**
         * @brief Compile shader from source code with tessellation shaders
         * @param vertexSrc Vertex shader source code
         * @param tessControlSrc Tessellation control shader source code
         * @param tessEvalSrc Tessellation evaluation shader source code
         * @param fragmentSrc Fragment shader source code
         * @return true if compilation was successful
         */
        virtual bool CompileWithTessellation(const std::string &vertexSrc, const std::string &tessControlSrc,
                                             const std::string &tessEvalSrc, const std::string &fragmentSrc) = 0;

        /**
         * @brief Compile shader from source code with all shader stages
         * @param vertexSrc Vertex shader source code
         * @param tessControlSrc Tessellation control shader source code (optional, can be empty)
         * @param tessEvalSrc Tessellation evaluation shader source code (optional, can be empty)
         * @param geometrySrc Geometry shader source code (optional, can be empty)
         * @param fragmentSrc Fragment shader source code
         * @return true if compilation was successful
         */
        virtual bool CompileAdvanced(const std::string &vertexSrc, const std::string &tessControlSrc,
                                     const std::string &tessEvalSrc, const std::string &geometrySrc,
                                     const std::string &fragmentSrc) = 0;

        /**
         * @brief Compile compute shader from source code
         * @param computeSrc Compute shader source code
         * @return true if compilation was successful
         */
        virtual bool CompileCompute(const std::string &computeSrc) = 0;

        /**
         * @brief Dispatch compute shader
         * @param numGroupsX Number of work groups in X dimension
         * @param numGroupsY Number of work groups in Y dimension
         * @param numGroupsZ Number of work groups in Z dimension
         */
        virtual void DispatchCompute(u32 numGroupsX, u32 numGroupsY, u32 numGroupsZ) = 0;

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

        /**
         * @brief Bind a shader storage buffer to a named storage block
         * @param blockName The name of the storage block in the shader
         * @param buffer The shader storage buffer to bind
         * @param bindingPoint The binding point index
         */
        virtual void BindShaderStorageBuffer(const std::string &blockName, class IShaderStorageBuffer *buffer, u32 bindingPoint) = 0;

        /**
         * @brief Set the binding point for a shader storage block
         * @param blockName The name of the storage block
         * @param bindingPoint The binding point index
         */
        virtual void SetShaderStorageBlockBinding(const std::string &blockName, u32 bindingPoint) = 0;

        // TODO: Add overloads for Vec2, Vec3, Vec4, Mat3, Mat4 types when math library is more complete.
    };

} // namespace Pyramid
