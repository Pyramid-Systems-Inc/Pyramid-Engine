#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp> // For BufferUsage enum
#include <Pyramid/Graphics/OpenGL/OpenGLDevice.hpp>
#include <Pyramid/Platform/Window.hpp> // Added for Window* parameter

namespace Pyramid
{

    std::unique_ptr<IGraphicsDevice> IGraphicsDevice::Create(GraphicsAPI api, Window *window) // Changed
    {
        switch (api)
        {
        case GraphicsAPI::OpenGL:
            return std::make_unique<OpenGLDevice>(window); // Changed
        default:
            return nullptr;
        }
    }

} // namespace Pyramid
