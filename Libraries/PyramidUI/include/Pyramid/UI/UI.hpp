#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Input/InputActions.hpp>
#include <Pyramid/Math/Vec2.hpp>
#include <Pyramid/Text/Text.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Pyramid::UI
{
    using WidgetId = u64;
    using TextureId = u64;

    constexpr TextureId DebugFontTextureId = 1;

    struct Rect
    {
        f32 x = 0.0f;
        f32 y = 0.0f;
        f32 width = 0.0f;
        f32 height = 0.0f;

        [[nodiscard]] bool IsValid() const { return width > 0.0f && height > 0.0f; }
        [[nodiscard]] bool Contains(const Math::Vec2& point) const;
        [[nodiscard]] Rect Intersect(const Rect& other) const;
        [[nodiscard]] bool operator==(const Rect& other) const;
        [[nodiscard]] bool operator!=(const Rect& other) const { return !(*this == other); }
    };

    struct Insets
    {
        f32 left = 0.0f;
        f32 top = 0.0f;
        f32 right = 0.0f;
        f32 bottom = 0.0f;

        [[nodiscard]] bool IsFiniteNonNegative() const;
    };

    struct FrameInfo
    {
        f32 width = 0.0f;
        f32 height = 0.0f;
        f32 dpiScale = 1.0f;
        f32 deltaTime = 0.0f;

        [[nodiscard]] bool IsValid() const;
    };

    struct Theme
    {
        Color background = Color(0.055f, 0.065f, 0.085f, 0.96f);
        Color panel = Color(0.085f, 0.10f, 0.135f, 0.98f);
        Color border = Color(0.28f, 0.32f, 0.40f, 1.0f);
        Color text = Color(0.92f, 0.94f, 0.98f, 1.0f);
        Color mutedText = Color(0.62f, 0.68f, 0.78f, 1.0f);
        Color accent = Color(0.18f, 0.62f, 0.92f, 1.0f);
        Color hovered = Color(0.16f, 0.20f, 0.28f, 1.0f);
        Color pressed = Color(0.11f, 0.15f, 0.22f, 1.0f);
        Color disabled = Color(0.28f, 0.30f, 0.34f, 0.65f);
        f32 padding = 8.0f;
        f32 spacing = 6.0f;
        f32 borderWidth = 1.0f;
        f32 defaultRowHeight = 22.0f;
        f32 textScale = 1.0f;
    };

    struct Vertex
    {
        Math::Vec2 position = Math::Vec2::Zero;
        Math::Vec2 uv = Math::Vec2::Zero;
        Color color = Color::White;
    };

    struct DrawBatch
    {
        TextureId texture = DebugFontTextureId;
        Rect clip;
        u32 indexOffset = 0;
        u32 indexCount = 0;
    };

    class DrawList final
    {
    public:
        void Clear();
        void AddQuad(
            const Rect& rect,
            const Rect& uv,
            const Color& color,
            TextureId texture,
            const Rect& clip);
        void AddNineSlice(
            const Rect& rect,
            const Rect& uv,
            const Insets& border,
            const Insets& uvBorder,
            const Color& color,
            TextureId texture,
            const Rect& clip);

        [[nodiscard]] const std::vector<Vertex>& GetVertices() const { return m_vertices; }
        [[nodiscard]] const std::vector<u32>& GetIndices() const { return m_indices; }
        [[nodiscard]] const std::vector<DrawBatch>& GetBatches() const { return m_batches; }
        [[nodiscard]] bool Empty() const { return m_indices.empty(); }

    private:
        std::vector<Vertex> m_vertices;
        std::vector<u32> m_indices;
        std::vector<DrawBatch> m_batches;
    };

    enum class FlowDirection : u8
    {
        Vertical = 0,
        Horizontal
    };

    struct PanelOptions
    {
        Math::Vec2 position = Math::Vec2(12.0f, 12.0f);
        Math::Vec2 size = Math::Vec2(340.0f, 430.0f);
        bool visible = true;
        bool enabled = true;
        bool blocksPointer = true;
    };

    struct ItemOptions
    {
        f32 width = 0.0f;
        f32 height = 0.0f;
        bool enabled = true;
    };

    struct ScrollAreaOptions
    {
        f32 height = 140.0f;
        f32 wheelStep = 28.0f;
        bool showScrollbar = true;
        bool stickToBottom = false;
        bool enabled = true;
    };

    struct ContextStats
    {
        u32 retainedElements = 0;
        u32 visibleElements = 0;
        u32 interactiveElements = 0;
        u32 vertices = 0;
        u32 indices = 0;
        u32 batches = 0;
    };

    /**
     * Hybrid UI context. Immediate widget calls reconcile retained element state,
     * while layout, focus, pointer capture and draw generation are shared by both
     * debug and future retained game/editor surfaces.
     */
    class Context final
    {
    public:
        Context();

        void SetEnabled(bool enabled);
        [[nodiscard]] bool IsEnabled() const { return m_enabled; }
        void SetTheme(const Theme& theme) { m_theme = theme; }
        [[nodiscard]] const Theme& GetTheme() const { return m_theme; }
        [[nodiscard]] bool SetFontAtlas(const Text::FontAtlas& font);
        [[nodiscard]] const Text::FontAtlas& GetFontAtlas() const { return m_font; }
        [[nodiscard]] const Text::FontAtlas& GetDebugFont() const { return m_font; }

        /** Hit-tests the previous retained frame before action contexts evaluate. */
        [[nodiscard]] InputConsumptionMask PrepareInput(const InputState& input);

        [[nodiscard]] bool BeginFrame(const FrameInfo& frame, const InputState& input);
        [[nodiscard]] const DrawList& EndFrame();

        /** Full-frame overlay used by menus and modal screens. */
        void Overlay(
            std::string_view id,
            const Color& color,
            bool blocksPointer = true);
        [[nodiscard]] bool BeginPanel(std::string_view label, const PanelOptions& options = {});
        void EndPanel();
        [[nodiscard]] bool BeginHorizontal(std::string_view id, f32 height = 0.0f);
        void EndHorizontal();

        void PushId(WidgetId id);
        void PushId(std::string_view id);
        void PopId();

        void Label(std::string_view text, const ItemOptions& options = {});
        void LabelColored(
            std::string_view text,
            const Color& color,
            const ItemOptions& options = {});
        void WrappedLabel(
            std::string_view text,
            const ItemOptions& options = {},
            const Color* color = nullptr);
        void LabelValue(
            std::string_view label,
            std::string_view value,
            const ItemOptions& options = {});
        void Separator();
        void Spacer(f32 height = 0.0f);
        [[nodiscard]] bool CollapsingHeader(
            std::string_view label,
            bool& open,
            const ItemOptions& options = {});
        [[nodiscard]] bool BeginScrollArea(
            std::string_view id,
            const ScrollAreaOptions& options = {});
        void EndScrollArea();
        [[nodiscard]] bool Button(std::string_view label, const ItemOptions& options = {});
        [[nodiscard]] bool Checkbox(
            std::string_view label,
            bool& value,
            const ItemOptions& options = {});
        [[nodiscard]] bool SliderFloat(
            std::string_view label,
            f32& value,
            f32 minimum,
            f32 maximum,
            const ItemOptions& options = {});
        void ProgressBar(
            std::string_view label,
            f32 fraction,
            const ItemOptions& options = {});
        void Image(
            std::string_view id,
            TextureId texture,
            const Math::Vec2& size,
            const Rect& uv = Rect{0.0f, 0.0f, 1.0f, 1.0f});

        [[nodiscard]] const DrawList& GetDrawList() const { return m_drawList; }
        [[nodiscard]] const FrameInfo& GetFrameInfo() const { return m_frame; }
        [[nodiscard]] ContextStats GetStats() const;
        [[nodiscard]] WidgetId GetFocusedWidget() const { return m_focusedId; }
        [[nodiscard]] bool WantsPointerInput() const { return m_wantsPointer; }
        [[nodiscard]] bool WantsKeyboardInput() const { return m_focusedId != 0; }

    private:
        enum class ElementKind : u8
        {
            Overlay,
            Panel,
            Container,
            Label,
            Separator,
            Button,
            Checkbox,
            Slider,
            Progress,
            Image,
            CollapsingHeader,
            ScrollArea
        };

        struct ElementState
        {
            WidgetId id = 0;
            WidgetId parent = 0;
            ElementKind kind = ElementKind::Label;
            Rect rect;
            Rect clip;
            bool visible = false;
            bool enabled = true;
            bool interactive = false;
            bool blocksPointer = false;
            f32 scrollOffset = 0.0f;
            f32 contentExtent = 0.0f;
            bool followsEnd = true;
            u64 visitedFrame = 0;
        };

        struct LayoutState
        {
            WidgetId owner = 0;
            FlowDirection flow = FlowDirection::Vertical;
            Rect content;
            Rect clip;
            Math::Vec2 cursor = Math::Vec2::Zero;
            f32 rowHeight = 0.0f;
            Rect viewport;
            WidgetId scrollOwner = 0;
            f32 scrollOffset = 0.0f;
            f32 wheelStep = 0.0f;
            bool showScrollbar = false;
            bool stickToBottom = false;
            bool scrollArea = false;
        };

        [[nodiscard]] WidgetId MakeId(std::string_view label) const;
        [[nodiscard]] WidgetId CurrentParent() const;
        [[nodiscard]] Rect Allocate(const ItemOptions& options, f32 defaultHeight);
        [[nodiscard]] bool IsHovered(const Rect& rect) const;
        [[nodiscard]] bool IsKeyboardActivate(WidgetId id) const;
        void RecordElement(
            WidgetId id,
            ElementKind kind,
            const Rect& rect,
            const Rect& clip,
            bool enabled,
            bool interactive,
            bool blocksPointer);
        void DrawSolid(const Rect& rect, const Color& color, const Rect& clip);
        void DrawBorder(const Rect& rect, const Color& color, const Rect& clip);
        void DrawText(
            std::string_view text,
            const Math::Vec2& position,
            const Color& color,
            const Rect& clip);
        void DrawTextRight(
            std::string_view text,
            f32 right,
            f32 y,
            const Color& color,
            const Rect& clip);
        [[nodiscard]] Text::LayoutResult BuildWrappedText(
            std::string_view text,
            const Math::Vec2& position,
            f32 maximumWidth) const;
        void DrawTextLayout(
            const Text::LayoutResult& layout,
            const Color& color,
            const Rect& clip);
        void AdvanceKeyboardFocus();
        void ResetFrameState();

        Theme m_theme;
        Text::FontAtlas m_font;
        DrawList m_drawList;
        FrameInfo m_frame;
        const InputState* m_input = nullptr;
        std::unordered_map<WidgetId, ElementState> m_elements;
        std::vector<WidgetId> m_drawOrder;
        std::vector<WidgetId> m_interactiveOrder;
        std::vector<LayoutState> m_layoutStack;
        std::vector<WidgetId> m_idStack;
        u64 m_frameIndex = 0;
        WidgetId m_hotId = 0;
        WidgetId m_activeId = 0;
        WidgetId m_focusedId = 0;
        bool m_enabled = true;
        bool m_frameActive = false;
        bool m_wantsPointer = false;
    };
} // namespace Pyramid::UI
