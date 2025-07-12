#pragma once
#include "Pyramid/Graphics/GraphicsDevice.hpp"
#include "Pyramid/Platform/Window.hpp"
#include "Pyramid/Graphics/Shader/Shader.hpp"
#include "Pyramid/Graphics/Texture.hpp" // Added for ITexture2D and TextureSpecification
#include <memory>
#include <string> // For std::string in CreateTexture2D

namespace Pyramid
{

    /**
     * @brief OpenGL implementation of the graphics device
     */
    class OpenGLDevice : public IGraphicsDevice
    {
    public:
        explicit OpenGLDevice(Window *window); // Changed
        ~OpenGLDevice() override;

        bool Initialize() override;
        void Shutdown() override;
        void Clear(const Color &color) override;
        void Present(bool vsync) override;
        void DrawIndexed(u32 count) override;
        void SetViewport(u32 x, u32 y, u32 width, u32 height) override;

        // Add factory method implementations
        std::shared_ptr<IVertexBuffer> CreateVertexBuffer() override;
        std::shared_ptr<IIndexBuffer> CreateIndexBuffer() override;
        std::shared_ptr<IVertexArray> CreateVertexArray() override;
        std::shared_ptr<IShader> CreateShader() override;
        std::shared_ptr<ITexture2D> CreateTexture2D(const TextureSpecification &specification, const void *data = nullptr) override;    // Added
        std::shared_ptr<ITexture2D> CreateTexture2D(const std::string &filepath, bool srgb = false, bool generateMips = true) override; // Added
        std::shared_ptr<class IUniformBuffer> CreateUniformBuffer(size_t size, BufferUsage usage = BufferUsage::Dynamic) override;

        /**
         * @brief Get the window associated with this device
         * @return Window* The window instance
         */
        Window *GetWindow() const { return m_window; } // Changed

    private:
        Window *m_window; // Changed
    };

} // namespace Pyramid
