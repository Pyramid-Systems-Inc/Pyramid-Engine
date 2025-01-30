#pragma once
#include <Pyramid/Core/Prerequisites.hpp>
#include <memory>

namespace Pyramid {

/**
 * @brief Interface for graphics device implementations
 */
class IGraphicsDevice {
public:
    virtual ~IGraphicsDevice() = default;

    /**
     * @brief Initialize the graphics device
     * @return true if successful
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Shutdown the graphics device
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Clear the screen with a specified color
     * @param color The color to clear with
     */
    virtual void Clear(const Color& color) = 0;

    /**
     * @brief Present the rendered frame
     * @param vsync Enable/disable vertical sync
     */
    virtual void Present(bool vsync) = 0;

    /**
     * @brief Create a graphics device for the specified API
     * @param api The graphics API to use
     * @return std::unique_ptr<IGraphicsDevice> The created graphics device
     */
    static std::unique_ptr<IGraphicsDevice> Create(GraphicsAPI api);
};

} // namespace Pyramid
