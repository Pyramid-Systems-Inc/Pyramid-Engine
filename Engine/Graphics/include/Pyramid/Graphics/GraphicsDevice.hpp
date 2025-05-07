#pragma once
#include <Pyramid/Core/Prerequisites.hpp>
#include <memory>
#include <string> // Added for std::string

namespace Pyramid
{
    // Forward declarations
    class Window;
    class IShader;
    class ITexture2D; // Added
    struct TextureSpecification; // Added

    /**
     * @brief Interface for graphics device implementations
     */
    class IGraphicsDevice
    {
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
        virtual void Clear(const Color &color) = 0;

        /**
         * @brief Present the rendered frame
         * @param vsync Enable/disable vertical sync
         */
        virtual void Present(bool vsync) = 0;

        /**
         * @brief Draw indexed primitives
         * @param count The number of indices to draw
         */
        virtual void DrawIndexed(u32 count) = 0;

        /**
         * @brief Set the viewport dimensions
         * @param x The x-coordinate of the viewport
         * @param y The y-coordinate of the viewport
         * @param width The width of the viewport
         * @param height The height of the viewport
         */
        virtual void SetViewport(u32 x, u32 y, u32 width, u32 height) = 0;

        /**
         * @brief Create a vertex buffer
         * @return std::shared_ptr<IVertexBuffer> The created vertex buffer
         */
        virtual std::shared_ptr<class IVertexBuffer> CreateVertexBuffer() = 0;

        /**
         * @brief Create an index buffer
         * @return std::shared_ptr<IIndexBuffer> The created index buffer
         */
        virtual std::shared_ptr<class IIndexBuffer> CreateIndexBuffer() = 0;

        /**
         * @brief Create a vertex array
         * @return std::shared_ptr<IVertexArray> The created vertex array
         */
        virtual std::shared_ptr<class IVertexArray> CreateVertexArray() = 0;

        /**
         * @brief Create a shader program
         * @return std::shared_ptr<IShader> The created shader program
         */
        virtual std::shared_ptr<IShader> CreateShader() = 0;

        /**
         * @brief Create a 2D Texture from specification and data
         * @param specification The texture specification
         * @param data Optional raw pixel data
         * @return std::shared_ptr<ITexture2D> The created texture
         */
        virtual std::shared_ptr<ITexture2D> CreateTexture2D(const TextureSpecification& specification, const void* data = nullptr) = 0; // Added

        /**
         * @brief Create a 2D Texture from a filepath
         * @param filepath Path to the image file
         * @param srgb Whether the texture should be treated as sRGB
         * @param generateMips Whether to generate mipmaps
         * @return std::shared_ptr<ITexture2D> The created texture
         */
        virtual std::shared_ptr<ITexture2D> CreateTexture2D(const std::string& filepath, bool srgb = false, bool generateMips = true) = 0; // Added
        
        /**
         * @brief Create a graphics device for the specified API
         * @param api The graphics API to use
         * @param window The window to associate with the graphics device
         * @return std::unique_ptr<IGraphicsDevice> The created graphics device
         */
        static std::unique_ptr<IGraphicsDevice> Create(GraphicsAPI api, Window* window); // Changed
    };

} // namespace Pyramid
