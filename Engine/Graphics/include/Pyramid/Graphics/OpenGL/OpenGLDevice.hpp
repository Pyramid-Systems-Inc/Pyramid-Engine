#pragma once
#include "Pyramid/Graphics/GraphicsDevice.hpp"
#include "Pyramid/Platform/Window.hpp"
#include "Pyramid/Graphics/Shader/Shader.hpp" // Added for IShader
#include <memory>

namespace Pyramid
{

    /**
     * @brief OpenGL implementation of the graphics device
     */
    class OpenGLDevice : public IGraphicsDevice
    {
    public:
        explicit OpenGLDevice(Window* window); // Changed
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
        std::shared_ptr<IShader> CreateShader() override; // Added

        /**
         * @brief Get the window associated with this device
         * @return Window* The window instance
         */
        Window* GetWindow() const { return m_window; } // Changed

    private:
        Window* m_window; // Changed
    };

} // namespace Pyramid
