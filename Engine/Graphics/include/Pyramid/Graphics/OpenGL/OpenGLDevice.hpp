#pragma once
#include "Pyramid/Graphics/GraphicsDevice.hpp"
#include "Pyramid/Platform/Windows/Win32OpenGLWindow.hpp"
#include <memory>

namespace Pyramid
{

    /**
     * @brief OpenGL implementation of the graphics device
     */
    class OpenGLDevice : public IGraphicsDevice
    {
    public:
        OpenGLDevice();
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

        /**
         * @brief Get the OpenGL window
         * @return Win32OpenGLWindow* The window instance
         */
        Win32OpenGLWindow *GetWindow() const { return m_window.get(); }

    private:
        std::unique_ptr<Win32OpenGLWindow> m_window;
    };

} // namespace Pyramid
