#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLDevice.hpp>

namespace Pyramid {

std::unique_ptr<IGraphicsDevice> IGraphicsDevice::Create(GraphicsAPI api)
{
    switch (api)
    {
    case GraphicsAPI::OpenGL:
        return std::make_unique<OpenGLDevice>();
    default:
        return nullptr;
    }
}

} // namespace Pyramid
