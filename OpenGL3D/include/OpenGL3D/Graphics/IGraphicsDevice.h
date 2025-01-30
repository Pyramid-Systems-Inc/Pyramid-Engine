#pragma once
#include <OpenGL3D/Core/Prerequisites.h>
#include <memory>

namespace Pyramid {

/**
 * @brief Color structure representing RGBA color values
 */
struct Color {
    float r, g, b, a;
    
    Color(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}
};

/**
 * @brief Graphics API type enumeration
 */
enum class GraphicsAPI {
    OpenGL,
    DirectX9,
    DirectX11,
    DirectX12,
    Vulkan
};

/**
 * @brief Interface for graphics device implementations
 * 
 * This interface defines the basic functionality that all graphics
 * device implementations must provide, regardless of the underlying API.
 */
class IGraphicsDevice {
public:
    virtual ~IGraphicsDevice() = default;

    /**
     * @brief Initialize the graphics device
     * @return true if initialization was successful
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Shutdown and cleanup the graphics device
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Clear the current render target
     * @param color The color to clear with
     */
    virtual void Clear(const Color& color) = 0;

    /**
     * @brief Present the rendered frame to the screen
     * @param vsync Enable/disable vertical synchronization
     */
    virtual void Present(bool vsync) = 0;

    /**
     * @brief Make this device's context current
     */
    virtual void MakeContextCurrent() = 0;

    /**
     * @brief Get the type of graphics API this device uses
     * @return The graphics API type
     */
    virtual GraphicsAPI GetAPIType() const = 0;

    /**
     * @brief Create a graphics device
     * @param api The graphics API to use
     * @return Unique pointer to the created device
     */
    static std::unique_ptr<IGraphicsDevice> Create(GraphicsAPI api);
};

} // namespace Pyramid
