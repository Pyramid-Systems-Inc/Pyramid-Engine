#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/UI/UI.hpp>

#include <memory>
#include <unordered_map>

namespace Pyramid
{
    class IGraphicsDevice;
    class IIndexBuffer;
    class ITexture2D;
    class IVertexArray;
    class IVertexBuffer;
    class ResourceRegistry;
    class ShaderProgram;
    class TextureResource;

    struct UIRendererStats
    {
        u32 drawCalls = 0;
        u32 batchesSubmitted = 0;
        u32 batchesSkipped = 0;
        u32 verticesUploaded = 0;
        u32 indicesUploaded = 0;
    };

    /** Graphics adapter that consumes renderer-independent Pyramid::UI draw lists. */
    class UIRenderer final
    {
    public:
        UIRenderer() = default;
        UIRenderer(const UIRenderer&) = delete;
        UIRenderer& operator=(const UIRenderer&) = delete;
        ~UIRenderer();

        [[nodiscard]] bool Initialize(
            IGraphicsDevice& device,
            ResourceRegistry& resources,
            const Text::FontAtlas& debugFont);
        void Shutdown();

        [[nodiscard]] bool RegisterTexture(
            UI::TextureId id,
            const std::shared_ptr<ITexture2D>& texture);
        void UnregisterTexture(UI::TextureId id);

        [[nodiscard]] bool Render(
            const UI::DrawList& drawList,
            const UI::FrameInfo& frame);

        [[nodiscard]] bool IsInitialized() const { return m_device != nullptr; }
        [[nodiscard]] const UIRendererStats& GetStats() const { return m_stats; }

    private:
        [[nodiscard]] std::shared_ptr<ITexture2D> ResolveTexture(UI::TextureId id) const;
        void RestoreBaselineState();

        IGraphicsDevice* m_device = nullptr;
        std::shared_ptr<ShaderProgram> m_shader;
        std::shared_ptr<TextureResource> m_fontTexture;
        std::shared_ptr<IVertexBuffer> m_vertexBuffer;
        std::shared_ptr<IIndexBuffer> m_indexBuffer;
        std::shared_ptr<IVertexArray> m_vertexArray;
        std::unordered_map<UI::TextureId, std::weak_ptr<ITexture2D>> m_textures;
        UIRendererStats m_stats;
    };
} // namespace Pyramid
