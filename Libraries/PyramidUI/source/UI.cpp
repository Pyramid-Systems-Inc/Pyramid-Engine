#include <Pyramid/UI/UI.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace Pyramid::UI
{
    namespace
    {
        constexpr u64 kFnvOffset = 14695981039346656037ull;
        constexpr u64 kFnvPrime = 1099511628211ull;

        WidgetId HashBytes(WidgetId seed, const void* data, std::size_t size)
        {
            WidgetId hash = seed == 0 ? kFnvOffset : seed;
            const auto* bytes = static_cast<const u8*>(data);
            for (std::size_t index = 0; index < size; ++index)
            {
                hash ^= bytes[index];
                hash *= kFnvPrime;
            }
            return hash == 0 ? 1 : hash;
        }

        WidgetId HashString(WidgetId seed, std::string_view value)
        {
            return HashBytes(seed, value.data(), value.size());
        }

        f32 Clamp01(f32 value)
        {
            return (std::max)(0.0f, (std::min)(value, 1.0f));
        }

        bool IsFiniteRect(const Rect& rect)
        {
            return std::isfinite(rect.x) && std::isfinite(rect.y) &&
                std::isfinite(rect.width) && std::isfinite(rect.height);
        }
    } // namespace

    bool Rect::Contains(const Math::Vec2& point) const
    {
        return IsValid() && point.x >= x && point.y >= y &&
            point.x < x + width && point.y < y + height;
    }

    Rect Rect::Intersect(const Rect& other) const
    {
        const f32 left = (std::max)(x, other.x);
        const f32 top = (std::max)(y, other.y);
        const f32 right = (std::min)(x + width, other.x + other.width);
        const f32 bottom = (std::min)(y + height, other.y + other.height);
        if (right <= left || bottom <= top)
        {
            return {};
        }
        return {left, top, right - left, bottom - top};
    }

    bool Rect::operator==(const Rect& other) const
    {
        return x == other.x && y == other.y &&
            width == other.width && height == other.height;
    }

    bool Insets::IsFiniteNonNegative() const
    {
        return left >= 0.0f && top >= 0.0f && right >= 0.0f && bottom >= 0.0f &&
            std::isfinite(left) && std::isfinite(top) &&
            std::isfinite(right) && std::isfinite(bottom);
    }

    bool FrameInfo::IsValid() const
    {
        return width > 0.0f && height > 0.0f && dpiScale > 0.0f &&
            std::isfinite(width) && std::isfinite(height) &&
            std::isfinite(dpiScale) && std::isfinite(deltaTime);
    }

    void DrawList::Clear()
    {
        m_vertices.clear();
        m_indices.clear();
        m_batches.clear();
    }

    void DrawList::AddQuad(
        const Rect& rect,
        const Rect& uv,
        const Color& color,
        TextureId texture,
        const Rect& clip)
    {
        if (!rect.IsValid() || !clip.IsValid() || texture == 0 ||
            !IsFiniteRect(rect) || !IsFiniteRect(uv) || !IsFiniteRect(clip) ||
            !rect.Intersect(clip).IsValid())
        {
            return;
        }

        if (m_vertices.size() > static_cast<std::size_t>(std::numeric_limits<u32>::max() - 4U) ||
            m_indices.size() > static_cast<std::size_t>(std::numeric_limits<u32>::max() - 6U))
        {
            return;
        }

        const u32 base = static_cast<u32>(m_vertices.size());
        m_vertices.push_back({Math::Vec2(rect.x, rect.y), Math::Vec2(uv.x, uv.y), color});
        m_vertices.push_back({
            Math::Vec2(rect.x + rect.width, rect.y),
            Math::Vec2(uv.x + uv.width, uv.y),
            color});
        m_vertices.push_back({
            Math::Vec2(rect.x + rect.width, rect.y + rect.height),
            Math::Vec2(uv.x + uv.width, uv.y + uv.height),
            color});
        m_vertices.push_back({
            Math::Vec2(rect.x, rect.y + rect.height),
            Math::Vec2(uv.x, uv.y + uv.height),
            color});

        m_indices.insert(
            m_indices.end(),
            {base, base + 1U, base + 2U, base, base + 2U, base + 3U});

        if (m_batches.empty() || m_batches.back().texture != texture ||
            m_batches.back().clip != clip)
        {
            DrawBatch batch;
            batch.texture = texture;
            batch.clip = clip;
            batch.indexOffset = static_cast<u32>(m_indices.size() - 6U);
            batch.indexCount = 6;
            m_batches.push_back(batch);
        }
        else
        {
            m_batches.back().indexCount += 6;
        }
    }

    void DrawList::AddNineSlice(
        const Rect& rect,
        const Rect& uv,
        const Insets& border,
        const Insets& uvBorder,
        const Color& color,
        TextureId texture,
        const Rect& clip)
    {
        if (!rect.IsValid() || !clip.IsValid() || texture == 0 ||
            !IsFiniteRect(rect) || !IsFiniteRect(uv) || !IsFiniteRect(clip) ||
            !border.IsFiniteNonNegative() || !uvBorder.IsFiniteNonNegative())
        {
            return;
        }

        const f32 left = (std::min)(border.left, rect.width * 0.5f);
        const f32 right = (std::min)(border.right, rect.width - left);
        const f32 top = (std::min)(border.top, rect.height * 0.5f);
        const f32 bottom = (std::min)(border.bottom, rect.height - top);
        const f32 uvLeft = (std::min)(uvBorder.left, uv.width);
        const f32 uvRight = (std::min)(uvBorder.right, uv.width - uvLeft);
        const f32 uvTop = (std::min)(uvBorder.top, uv.height);
        const f32 uvBottom = (std::min)(uvBorder.bottom, uv.height - uvTop);

        const f32 x[4] = {rect.x, rect.x + left, rect.x + rect.width - right, rect.x + rect.width};
        const f32 y[4] = {rect.y, rect.y + top, rect.y + rect.height - bottom, rect.y + rect.height};
        const f32 u[4] = {uv.x, uv.x + uvLeft, uv.x + uv.width - uvRight, uv.x + uv.width};
        const f32 v[4] = {uv.y, uv.y + uvTop, uv.y + uv.height - uvBottom, uv.y + uv.height};

        for (u32 row = 0; row < 3; ++row)
        {
            for (u32 column = 0; column < 3; ++column)
            {
                AddQuad(
                    {x[column], y[row], x[column + 1] - x[column], y[row + 1] - y[row]},
                    {u[column], v[row], u[column + 1] - u[column], v[row + 1] - v[row]},
                    color,
                    texture,
                    clip);
            }
        }
    }

    Context::Context()
        : m_font(Text::CreateDebugFontAtlas())
    {
        m_idStack.push_back(kFnvOffset);
    }

    bool Context::SetFontAtlas(const Text::FontAtlas& font)
    {
        if (m_frameActive || !font.IsValid())
        {
            return false;
        }
        m_font = font;
        m_elements.clear();
        m_drawOrder.clear();
        m_interactiveOrder.clear();
        m_focusedId = 0;
        m_hotId = 0;
        m_activeId = 0;
        return true;
    }

    void Context::SetEnabled(bool enabled)
    {
        m_enabled = enabled;
        if (!m_enabled)
        {
            m_activeId = 0;
            m_hotId = 0;
            m_focusedId = 0;
            m_wantsPointer = false;
        }
    }

    InputConsumptionMask Context::PrepareInput(const InputState& input)
    {
        InputConsumptionMask mask;
        m_wantsPointer = false;
        if (!m_enabled || !input.HasFocus())
        {
            return mask;
        }

        if (input.HasMousePosition())
        {
            const MousePosition mouse = input.GetMousePosition();
            const Math::Vec2 pointer(mouse.x, mouse.y);
            for (auto iterator = m_drawOrder.rbegin(); iterator != m_drawOrder.rend(); ++iterator)
            {
                const auto found = m_elements.find(*iterator);
                if (found == m_elements.end())
                {
                    continue;
                }
                const ElementState& element = found->second;
                if (element.visible && element.blocksPointer && element.rect.Contains(pointer))
                {
                    m_wantsPointer = true;
                    break;
                }
            }
        }

        if (m_activeId != 0)
        {
            m_wantsPointer = true;
        }
        if (m_wantsPointer)
        {
            mask.ConsumeAllMouse();
        }

        if (m_focusedId != 0)
        {
            mask.ConsumeKey(Key::Tab);
            mask.ConsumeKey(Key::Enter);
            mask.ConsumeKey(Key::Space);
            mask.ConsumeKey(Key::Left);
            mask.ConsumeKey(Key::Right);
            mask.ConsumeKey(Key::Up);
            mask.ConsumeKey(Key::Down);
        }
        return mask;
    }

    bool Context::BeginFrame(const FrameInfo& frame, const InputState& input)
    {
        if (m_frameActive || !m_enabled || !frame.IsValid() || !m_font.IsValid())
        {
            return false;
        }

        ++m_frameIndex;
        m_frame = frame;
        m_input = &input;
        m_frameActive = true;

        if (input.WasKeyPressed(Key::Tab))
        {
            AdvanceKeyboardFocus();
        }
        ResetFrameState();

        LayoutState root;
        root.owner = 0;
        root.flow = FlowDirection::Vertical;
        root.content = {0.0f, 0.0f, frame.width, frame.height};
        root.clip = root.content;
        root.cursor = Math::Vec2::Zero;
        m_layoutStack.push_back(root);

        if (!input.IsMouseButtonDown(MouseButton::Left))
        {
            m_activeId = 0;
        }
        return true;
    }

    const DrawList& Context::EndFrame()
    {
        if (!m_frameActive)
        {
            return m_drawList;
        }

        while (m_layoutStack.size() > 1)
        {
            m_layoutStack.pop_back();
        }

        for (auto iterator = m_elements.begin(); iterator != m_elements.end();)
        {
            if (iterator->second.visitedFrame != m_frameIndex)
            {
                if (m_focusedId == iterator->first)
                {
                    m_focusedId = 0;
                }
                if (m_activeId == iterator->first)
                {
                    m_activeId = 0;
                }
                iterator = m_elements.erase(iterator);
            }
            else
            {
                ++iterator;
            }
        }

        m_frameActive = false;
        m_input = nullptr;
        m_layoutStack.clear();
        return m_drawList;
    }

    void Context::Overlay(
        std::string_view idText,
        const Color& color,
        bool blocksPointer)
    {
        if (!m_frameActive)
        {
            return;
        }
        const Rect rect{0.0f, 0.0f, m_frame.width, m_frame.height};
        const WidgetId id = MakeId(idText);
        RecordElement(
            id,
            ElementKind::Overlay,
            rect,
            rect,
            true,
            false,
            blocksPointer);
        DrawSolid(rect, color, rect);
    }

    bool Context::BeginPanel(std::string_view label, const PanelOptions& options)
    {
        if (!m_frameActive || !options.visible || options.size.x <= 0.0f ||
            options.size.y <= 0.0f)
        {
            return false;
        }

        const WidgetId id = MakeId(label);
        Rect rect{options.position.x, options.position.y, options.size.x, options.size.y};
        rect = rect.Intersect({0.0f, 0.0f, m_frame.width, m_frame.height});
        if (!rect.IsValid())
        {
            return false;
        }

        const Rect clip = rect;
        RecordElement(
            id,
            ElementKind::Panel,
            rect,
            clip,
            options.enabled,
            false,
            options.blocksPointer);
        DrawSolid(rect, m_theme.panel, clip);
        DrawBorder(rect, m_theme.border, clip);

        const f32 headerHeight = m_theme.defaultRowHeight + m_theme.padding;
        DrawText(
            label,
            Math::Vec2(rect.x + m_theme.padding, rect.y + m_theme.padding),
            options.enabled ? m_theme.text : m_theme.disabled,
            clip);
        DrawSolid(
            {rect.x + m_theme.padding, rect.y + headerHeight,
             rect.width - m_theme.padding * 2.0f, m_theme.borderWidth},
            m_theme.border,
            clip);

        LayoutState layout;
        layout.owner = id;
        layout.flow = FlowDirection::Vertical;
        layout.content = {
            rect.x + m_theme.padding,
            rect.y + headerHeight + m_theme.spacing,
            (std::max)(0.0f, rect.width - m_theme.padding * 2.0f),
            (std::max)(0.0f, rect.height - headerHeight - m_theme.padding - m_theme.spacing)};
        layout.clip = layout.content.Intersect(clip);
        layout.cursor = Math::Vec2(layout.content.x, layout.content.y);
        m_layoutStack.push_back(layout);
        m_idStack.push_back(id);
        return true;
    }

    void Context::EndPanel()
    {
        if (m_layoutStack.size() > 1)
        {
            m_layoutStack.pop_back();
        }
        if (m_idStack.size() > 1)
        {
            m_idStack.pop_back();
        }
    }

    bool Context::BeginHorizontal(std::string_view idText, f32 height)
    {
        if (!m_frameActive || m_layoutStack.empty())
        {
            return false;
        }
        ItemOptions options;
        options.height = height > 0.0f ? height : m_theme.defaultRowHeight;
        const Rect row = Allocate(options, options.height);
        if (!row.IsValid())
        {
            return false;
        }

        const WidgetId id = MakeId(idText);
        RecordElement(id, ElementKind::Container, row, m_layoutStack.back().clip, true, false, false);
        LayoutState layout;
        layout.owner = id;
        layout.flow = FlowDirection::Horizontal;
        layout.content = row;
        layout.clip = row.Intersect(m_layoutStack.back().clip);
        layout.cursor = Math::Vec2(row.x, row.y);
        layout.rowHeight = row.height;
        m_layoutStack.push_back(layout);
        m_idStack.push_back(id);
        return true;
    }

    void Context::EndHorizontal()
    {
        EndPanel();
    }

    void Context::PushId(WidgetId id)
    {
        const WidgetId seed = m_idStack.empty() ? kFnvOffset : m_idStack.back();
        m_idStack.push_back(HashBytes(seed, &id, sizeof(id)));
    }

    void Context::PushId(std::string_view id)
    {
        m_idStack.push_back(HashString(m_idStack.empty() ? kFnvOffset : m_idStack.back(), id));
    }

    void Context::PopId()
    {
        if (m_idStack.size() > 1)
        {
            m_idStack.pop_back();
        }
    }

    void Context::Label(std::string_view text, const ItemOptions& options)
    {
        const f32 rowHeight = options.height > 0.0f
            ? options.height
            : m_font.lineHeight * m_theme.textScale + 2.0f;
        const Rect rect = Allocate(options, rowHeight);
        if (!rect.IsValid())
        {
            return;
        }
        const WidgetId id = MakeId(text);
        const Rect clip = m_layoutStack.back().clip;
        RecordElement(id, ElementKind::Label, rect, clip, options.enabled, false, false);
        DrawText(
            text,
            Math::Vec2(rect.x, rect.y + 1.0f),
            options.enabled ? m_theme.text : m_theme.disabled,
            clip);
    }

    void Context::LabelColored(
        std::string_view text,
        const Color& color,
        const ItemOptions& options)
    {
        const f32 rowHeight = options.height > 0.0f
            ? options.height
            : m_font.lineHeight * m_theme.textScale + 2.0f;
        const Rect rect = Allocate(options, rowHeight);
        if (!rect.IsValid())
        {
            return;
        }
        const WidgetId id = MakeId(text);
        const Rect clip = m_layoutStack.back().clip;
        RecordElement(id, ElementKind::Label, rect, clip, options.enabled, false, false);
        DrawText(
            text,
            Math::Vec2(rect.x, rect.y + 1.0f),
            options.enabled ? color : m_theme.disabled,
            clip);
    }

    void Context::WrappedLabel(
        std::string_view text,
        const ItemOptions& options,
        const Color* color)
    {
        if (m_layoutStack.empty())
        {
            return;
        }
        const f32 width = options.width > 0.0f
            ? (std::min)(options.width, m_layoutStack.back().content.width)
            : m_layoutStack.back().content.width;
        const Text::LayoutResult measured =
            BuildWrappedText(text, Math::Vec2::Zero, width);
        const f32 height = options.height > 0.0f
            ? options.height
            : (std::max)(m_font.lineHeight * m_theme.textScale, measured.metrics.height) + 2.0f;
        ItemOptions allocation = options;
        allocation.width = width;
        allocation.height = height;
        const Rect rect = Allocate(allocation, height);
        if (!rect.IsValid())
        {
            return;
        }

        const WidgetId id = MakeId(text);
        const Rect clip = m_layoutStack.back().clip;
        RecordElement(id, ElementKind::Label, rect, clip, options.enabled, false, false);
        const Text::LayoutResult layout = BuildWrappedText(
            text,
            Math::Vec2(rect.x, rect.y + 1.0f),
            rect.width);
        DrawTextLayout(
            layout,
            options.enabled ? (color ? *color : m_theme.text) : m_theme.disabled,
            clip);
    }

    void Context::LabelValue(
        std::string_view label,
        std::string_view value,
        const ItemOptions& options)
    {
        const f32 rowHeight = options.height > 0.0f
            ? options.height
            : m_font.lineHeight * m_theme.textScale + 2.0f;
        const Rect rect = Allocate(options, rowHeight);
        if (!rect.IsValid())
        {
            return;
        }
        const WidgetId id = MakeId(label);
        const Rect clip = m_layoutStack.back().clip;
        RecordElement(id, ElementKind::Label, rect, clip, options.enabled, false, false);
        const Color color = options.enabled ? m_theme.text : m_theme.disabled;
        DrawText(label, Math::Vec2(rect.x, rect.y + 1.0f), m_theme.mutedText, clip);
        DrawTextRight(value, rect.x + rect.width, rect.y + 1.0f, color, clip);
    }

    void Context::Separator()
    {
        ItemOptions options;
        options.height = m_theme.spacing + m_theme.borderWidth;
        const Rect rect = Allocate(options, options.height);
        if (!rect.IsValid())
        {
            return;
        }
        const WidgetId id = MakeId("##separator");
        const Rect clip = m_layoutStack.back().clip;
        RecordElement(id, ElementKind::Separator, rect, clip, true, false, false);
        DrawSolid(
            {rect.x, rect.y + m_theme.spacing * 0.5f, rect.width, m_theme.borderWidth},
            m_theme.border,
            clip);
    }

    void Context::Spacer(f32 height)
    {
        ItemOptions options;
        options.height = height > 0.0f ? height : m_theme.spacing;
        (void)Allocate(options, options.height);
    }

    bool Context::CollapsingHeader(
        std::string_view label,
        bool& open,
        const ItemOptions& options)
    {
        const Rect rect = Allocate(options, m_theme.defaultRowHeight);
        if (!rect.IsValid())
        {
            return false;
        }
        const WidgetId id = MakeId(label);
        const Rect clip = m_layoutStack.back().clip;
        const bool hovered = options.enabled && IsHovered(rect);
        const bool toggled = options.enabled && m_input &&
            ((hovered && m_input->WasMouseButtonPressed(MouseButton::Left)) ||
             IsKeyboardActivate(id));
        if (toggled)
        {
            open = !open;
            m_activeId = id;
            m_focusedId = id;
        }

        RecordElement(
            id,
            ElementKind::CollapsingHeader,
            rect,
            clip,
            options.enabled,
            true,
            true);
        DrawSolid(rect, hovered ? m_theme.hovered : m_theme.background, clip);
        DrawBorder(rect, m_focusedId == id ? m_theme.accent : m_theme.border, clip);
        DrawText(
            open ? "v" : ">",
            Math::Vec2(rect.x + 5.0f, rect.y + 6.0f),
            options.enabled ? m_theme.accent : m_theme.disabled,
            clip);
        DrawText(
            label,
            Math::Vec2(rect.x + 18.0f, rect.y + 6.0f),
            options.enabled ? m_theme.text : m_theme.disabled,
            clip);
        return toggled;
    }

    bool Context::BeginScrollArea(
        std::string_view idText,
        const ScrollAreaOptions& options)
    {
        if (!m_frameActive || m_layoutStack.empty() || !std::isfinite(options.height) ||
            options.height <= 0.0f || !std::isfinite(options.wheelStep) ||
            options.wheelStep < 0.0f)
        {
            return false;
        }

        ItemOptions item;
        item.height = options.height;
        item.enabled = options.enabled;
        const Rect viewport = Allocate(item, options.height);
        if (!viewport.IsValid())
        {
            return false;
        }

        const WidgetId id = MakeId(idText);
        const Rect parentClip = m_layoutStack.back().clip;
        const Rect clip = viewport.Intersect(parentClip);
        if (!clip.IsValid())
        {
            return false;
        }
        RecordElement(
            id,
            ElementKind::ScrollArea,
            viewport,
            clip,
            options.enabled,
            false,
            true);

        ElementState& element = m_elements[id];
        const f32 maximumOffset =
            (std::max)(0.0f, element.contentExtent - viewport.height);
        element.scrollOffset = (std::max)(0.0f, (std::min)(element.scrollOffset, maximumOffset));
        if (options.stickToBottom && element.followsEnd)
        {
            element.scrollOffset = maximumOffset;
        }
        if (options.enabled && IsHovered(clip) && m_input)
        {
            const f32 wheel = m_input->GetMouseWheelDelta();
            if (wheel != 0.0f)
            {
                element.scrollOffset = (std::max)(
                    0.0f,
                    (std::min)(
                        maximumOffset,
                        element.scrollOffset - wheel * options.wheelStep));
                element.followsEnd =
                    element.scrollOffset >= (std::max)(0.0f, maximumOffset - 1.0f);
            }
        }

        DrawSolid(viewport, m_theme.background, parentClip);
        DrawBorder(viewport, m_theme.border, parentClip);

        LayoutState layout;
        layout.owner = id;
        layout.flow = FlowDirection::Vertical;
        layout.viewport = viewport;
        layout.content = {
            viewport.x + 4.0f,
            viewport.y + 4.0f,
            (std::max)(0.0f, viewport.width - (options.showScrollbar ? 14.0f : 8.0f)),
            viewport.height - 8.0f};
        layout.clip = {
            clip.x + 1.0f,
            clip.y + 1.0f,
            (std::max)(0.0f, clip.width - 2.0f),
            (std::max)(0.0f, clip.height - 2.0f)};
        layout.cursor = Math::Vec2(layout.content.x, layout.content.y - element.scrollOffset);
        layout.scrollOwner = id;
        layout.scrollOffset = element.scrollOffset;
        layout.wheelStep = options.wheelStep;
        layout.showScrollbar = options.showScrollbar;
        layout.stickToBottom = options.stickToBottom;
        layout.scrollArea = true;
        m_layoutStack.push_back(layout);
        m_idStack.push_back(id);
        return true;
    }

    void Context::EndScrollArea()
    {
        if (m_layoutStack.size() <= 1 || !m_layoutStack.back().scrollArea)
        {
            return;
        }

        const LayoutState layout = m_layoutStack.back();
        m_layoutStack.pop_back();
        if (m_idStack.size() > 1)
        {
            m_idStack.pop_back();
        }

        auto found = m_elements.find(layout.scrollOwner);
        if (found == m_elements.end())
        {
            return;
        }
        ElementState& element = found->second;
        const f32 rawExtent =
            layout.cursor.y + layout.scrollOffset - layout.content.y - m_theme.spacing;
        element.contentExtent = (std::max)(0.0f, rawExtent + 8.0f);
        const f32 maximumOffset =
            (std::max)(0.0f, element.contentExtent - layout.viewport.height);
        if (layout.stickToBottom && element.followsEnd)
        {
            element.scrollOffset = maximumOffset;
        }
        else
        {
            element.scrollOffset = (std::max)(
                0.0f,
                (std::min)(element.scrollOffset, maximumOffset));
        }

        if (layout.showScrollbar && element.contentExtent > layout.viewport.height)
        {
            const Rect clip = layout.viewport.Intersect(m_layoutStack.back().clip);
            const Rect track{
                layout.viewport.x + layout.viewport.width - 8.0f,
                layout.viewport.y + 3.0f,
                4.0f,
                layout.viewport.height - 6.0f};
            const f32 thumbHeight = (std::max)(
                12.0f,
                track.height * layout.viewport.height / element.contentExtent);
            const f32 travel = (std::max)(0.0f, track.height - thumbHeight);
            const f32 fraction = maximumOffset > 0.0f
                ? element.scrollOffset / maximumOffset
                : 0.0f;
            DrawSolid(track, m_theme.border, clip);
            DrawSolid(
                {track.x, track.y + travel * Clamp01(fraction), track.width, thumbHeight},
                m_theme.accent,
                clip);
        }
    }

    bool Context::Button(std::string_view label, const ItemOptions& options)
    {
        const Rect rect = Allocate(options, m_theme.defaultRowHeight);
        if (!rect.IsValid())
        {
            return false;
        }
        const WidgetId id = MakeId(label);
        const Rect clip = m_layoutStack.back().clip;
        const bool enabled = options.enabled;
        const bool hovered = enabled && IsHovered(rect);
        const bool keyboard = enabled && IsKeyboardActivate(id);
        const bool clicked = enabled && m_input &&
            ((hovered && m_input->WasMouseButtonPressed(MouseButton::Left)) || keyboard);
        if (clicked)
        {
            m_activeId = id;
            m_focusedId = id;
        }

        RecordElement(id, ElementKind::Button, rect, clip, enabled, true, true);
        const Color fill = !enabled ? m_theme.disabled
            : m_activeId == id ? m_theme.pressed
            : hovered ? m_theme.hovered
            : m_theme.background;
        DrawSolid(rect, fill, clip);
        DrawBorder(rect, m_focusedId == id ? m_theme.accent : m_theme.border, clip);
        const Text::TextMetrics metrics = Text::Measure(m_font, label, m_theme.textScale);
        DrawText(
            label,
            Math::Vec2(
                rect.x + (rect.width - metrics.width) * 0.5f,
                rect.y + (rect.height - metrics.height) * 0.5f),
            enabled ? m_theme.text : m_theme.disabled,
            clip);
        return clicked;
    }

    bool Context::Checkbox(
        std::string_view label,
        bool& value,
        const ItemOptions& options)
    {
        const Rect rect = Allocate(options, m_theme.defaultRowHeight);
        if (!rect.IsValid())
        {
            return false;
        }
        const WidgetId id = MakeId(label);
        const Rect clip = m_layoutStack.back().clip;
        const bool hovered = options.enabled && IsHovered(rect);
        const bool changed = options.enabled && m_input &&
            ((hovered && m_input->WasMouseButtonPressed(MouseButton::Left)) ||
             IsKeyboardActivate(id));
        if (changed)
        {
            value = !value;
            m_activeId = id;
            m_focusedId = id;
        }
        RecordElement(id, ElementKind::Checkbox, rect, clip, options.enabled, true, true);

        const f32 boxSize = (std::min)(rect.height - 4.0f, 16.0f);
        const Rect box{rect.x + 2.0f, rect.y + (rect.height - boxSize) * 0.5f, boxSize, boxSize};
        DrawSolid(box, hovered ? m_theme.hovered : m_theme.background, clip);
        DrawBorder(box, m_focusedId == id ? m_theme.accent : m_theme.border, clip);
        if (value)
        {
            DrawSolid(
                {box.x + 3.0f, box.y + 3.0f, box.width - 6.0f, box.height - 6.0f},
                m_theme.accent,
                clip);
        }
        DrawText(
            label,
            Math::Vec2(box.x + box.width + 7.0f, rect.y + 5.0f),
            options.enabled ? m_theme.text : m_theme.disabled,
            clip);
        return changed;
    }

    bool Context::SliderFloat(
        std::string_view label,
        f32& value,
        f32 minimum,
        f32 maximum,
        const ItemOptions& options)
    {
        if (!std::isfinite(minimum) || !std::isfinite(maximum) || maximum <= minimum)
        {
            return false;
        }
        const Rect rect = Allocate(options, m_theme.defaultRowHeight + 10.0f);
        if (!rect.IsValid())
        {
            return false;
        }

        const WidgetId id = MakeId(label);
        const Rect clip = m_layoutStack.back().clip;
        const bool hovered = options.enabled && IsHovered(rect);
        if (options.enabled && hovered && m_input &&
            m_input->WasMouseButtonPressed(MouseButton::Left))
        {
            m_activeId = id;
            m_focusedId = id;
        }
        if (m_activeId == id && m_input && !m_input->IsMouseButtonDown(MouseButton::Left))
        {
            m_activeId = 0;
        }

        const Rect track{rect.x, rect.y + 16.0f, rect.width, 8.0f};
        bool changed = false;
        if (options.enabled && m_activeId == id && m_input && m_input->HasMousePosition())
        {
            const f32 fraction = Clamp01(
                (m_input->GetMousePosition().x - track.x) /
                (std::max)(track.width, 1.0f));
            const f32 replacement = minimum + (maximum - minimum) * fraction;
            changed = replacement != value;
            value = replacement;
        }
        if (options.enabled && m_focusedId == id && m_input)
        {
            const f32 step = (maximum - minimum) * 0.01f;
            if (m_input->WasKeyPressed(Key::Left) || m_input->WasKeyPressed(Key::Down))
            {
                value = (std::max)(minimum, value - step);
                changed = true;
            }
            if (m_input->WasKeyPressed(Key::Right) || m_input->WasKeyPressed(Key::Up))
            {
                value = (std::min)(maximum, value + step);
                changed = true;
            }
        }
        value = (std::max)(minimum, (std::min)(value, maximum));

        RecordElement(id, ElementKind::Slider, rect, clip, options.enabled, true, true);
        DrawText(label, Math::Vec2(rect.x, rect.y), m_theme.mutedText, clip);
        DrawSolid(track, m_theme.background, clip);
        DrawBorder(track, m_focusedId == id ? m_theme.accent : m_theme.border, clip);
        const f32 fraction = (value - minimum) / (maximum - minimum);
        DrawSolid(
            {track.x + 1.0f, track.y + 1.0f,
             (track.width - 2.0f) * Clamp01(fraction), track.height - 2.0f},
            options.enabled ? m_theme.accent : m_theme.disabled,
            clip);
        return changed;
    }

    void Context::ProgressBar(
        std::string_view label,
        f32 fraction,
        const ItemOptions& options)
    {
        const Rect rect = Allocate(options, m_theme.defaultRowHeight);
        if (!rect.IsValid())
        {
            return;
        }
        const WidgetId id = MakeId(label);
        const Rect clip = m_layoutStack.back().clip;
        RecordElement(id, ElementKind::Progress, rect, clip, options.enabled, false, false);
        DrawSolid(rect, m_theme.background, clip);
        DrawBorder(rect, m_theme.border, clip);
        DrawSolid(
            {rect.x + 1.0f, rect.y + 1.0f,
             (rect.width - 2.0f) * Clamp01(fraction), rect.height - 2.0f},
            options.enabled ? m_theme.accent : m_theme.disabled,
            clip);
        DrawText(
            label,
            Math::Vec2(rect.x + 5.0f, rect.y + 5.0f),
            m_theme.text,
            clip);
    }

    void Context::Image(
        std::string_view idText,
        TextureId texture,
        const Math::Vec2& size,
        const Rect& uv)
    {
        ItemOptions options;
        options.width = size.x;
        options.height = size.y;
        const Rect rect = Allocate(options, size.y);
        if (!rect.IsValid() || texture == 0)
        {
            return;
        }
        const WidgetId id = MakeId(idText);
        const Rect clip = m_layoutStack.back().clip;
        RecordElement(id, ElementKind::Image, rect, clip, true, false, false);
        m_drawList.AddQuad(rect, uv, Color::White, texture, clip);
    }

    WidgetId Context::MakeId(std::string_view label) const
    {
        return HashString(m_idStack.empty() ? kFnvOffset : m_idStack.back(), label);
    }

    WidgetId Context::CurrentParent() const
    {
        return m_layoutStack.empty() ? 0 : m_layoutStack.back().owner;
    }

    Rect Context::Allocate(const ItemOptions& options, f32 defaultHeight)
    {
        if (m_layoutStack.empty())
        {
            return {};
        }

        LayoutState& layout = m_layoutStack.back();
        const f32 height = options.height > 0.0f ? options.height : defaultHeight;
        if (height <= 0.0f)
        {
            return {};
        }

        Rect rect;
        if (layout.flow == FlowDirection::Vertical)
        {
            rect.x = layout.content.x;
            rect.y = layout.cursor.y;
            rect.width = options.width > 0.0f
                ? (std::min)(options.width, layout.content.width)
                : layout.content.width;
            rect.height = height;
            layout.cursor.y += height + m_theme.spacing;
        }
        else
        {
            rect.x = layout.cursor.x;
            rect.y = layout.content.y;
            const f32 remaining =
                (std::max)(0.0f, layout.content.x + layout.content.width - layout.cursor.x);
            rect.width = options.width > 0.0f
                ? (std::min)(options.width, remaining)
                : (std::min)(120.0f, remaining);
            rect.height = (std::min)(height, layout.rowHeight);
            layout.cursor.x += rect.width + m_theme.spacing;
        }
        return rect;
    }

    bool Context::IsHovered(const Rect& rect) const
    {
        if (!m_input || !m_input->HasFocus() || !m_input->HasMousePosition())
        {
            return false;
        }
        const Rect hitRect = m_layoutStack.empty()
            ? rect
            : rect.Intersect(m_layoutStack.back().clip);
        return hitRect.Contains(Math::Vec2(
            m_input->GetMousePosition().x,
            m_input->GetMousePosition().y));
    }

    bool Context::IsKeyboardActivate(WidgetId id) const
    {
        return m_input && m_focusedId == id &&
            (m_input->WasKeyPressed(Key::Enter) || m_input->WasKeyPressed(Key::Space));
    }

    void Context::RecordElement(
        WidgetId id,
        ElementKind kind,
        const Rect& rect,
        const Rect& clip,
        bool enabled,
        bool interactive,
        bool blocksPointer)
    {
        const Rect visibleRect = rect.Intersect(clip);
        if (id == 0 || !rect.IsValid() || !visibleRect.IsValid())
        {
            return;
        }
        ElementState& element = m_elements[id];
        element.id = id;
        element.parent = CurrentParent();
        element.kind = kind;
        element.rect = visibleRect;
        element.clip = clip;
        element.visible = true;
        element.enabled = enabled;
        element.interactive = interactive;
        element.blocksPointer = blocksPointer;
        element.visitedFrame = m_frameIndex;
        m_drawOrder.push_back(id);
        if (interactive)
        {
            m_interactiveOrder.push_back(id);
        }
        if (IsHovered(rect))
        {
            m_hotId = id;
        }
    }

    void Context::DrawSolid(const Rect& rect, const Color& color, const Rect& clip)
    {
        const Math::Vec2 uv = m_font.whitePixelUv;
        m_drawList.AddQuad(
            rect,
            {uv.x, uv.y, 0.0f, 0.0f},
            color,
            DebugFontTextureId,
            clip);
    }

    void Context::DrawBorder(const Rect& rect, const Color& color, const Rect& clip)
    {
        const f32 thickness = (std::max)(1.0f, m_theme.borderWidth);
        DrawSolid({rect.x, rect.y, rect.width, thickness}, color, clip);
        DrawSolid(
            {rect.x, rect.y + rect.height - thickness, rect.width, thickness},
            color,
            clip);
        DrawSolid({rect.x, rect.y, thickness, rect.height}, color, clip);
        DrawSolid(
            {rect.x + rect.width - thickness, rect.y, thickness, rect.height},
            color,
            clip);
    }

    void Context::DrawText(
        std::string_view text,
        const Math::Vec2& position,
        const Color& color,
        const Rect& clip)
    {
        std::vector<Text::GlyphQuad> quads;
        quads.reserve(text.size());
        Text::BuildGlyphQuads(m_font, text, position, m_theme.textScale, quads);
        for (const Text::GlyphQuad& quad : quads)
        {
            m_drawList.AddQuad(
                {quad.minimum.x, quad.minimum.y,
                 quad.maximum.x - quad.minimum.x,
                 quad.maximum.y - quad.minimum.y},
                {quad.uvMinimum.x, quad.uvMinimum.y,
                 quad.uvMaximum.x - quad.uvMinimum.x,
                 quad.uvMaximum.y - quad.uvMinimum.y},
                color,
                DebugFontTextureId,
                clip);
        }
    }

    void Context::DrawTextRight(
        std::string_view text,
        f32 right,
        f32 y,
        const Color& color,
        const Rect& clip)
    {
        const Text::TextMetrics metrics = Text::Measure(m_font, text, m_theme.textScale);
        DrawText(text, Math::Vec2(right - metrics.width, y), color, clip);
    }

    Text::LayoutResult Context::BuildWrappedText(
        std::string_view text,
        const Math::Vec2& position,
        f32 maximumWidth) const
    {
        Text::LayoutOptions options;
        options.scale = m_theme.textScale;
        options.maximumWidth = (std::max)(0.0f, maximumWidth);
        options.wrap = Text::WrapMode::Word;
        options.lineSpacing = 1.0f;
        return Text::Layout(m_font, text, position, options);
    }

    void Context::DrawTextLayout(
        const Text::LayoutResult& layout,
        const Color& color,
        const Rect& clip)
    {
        for (const Text::GlyphQuad& quad : layout.glyphs)
        {
            m_drawList.AddQuad(
                {quad.minimum.x, quad.minimum.y,
                 quad.maximum.x - quad.minimum.x,
                 quad.maximum.y - quad.minimum.y},
                {quad.uvMinimum.x, quad.uvMinimum.y,
                 quad.uvMaximum.x - quad.uvMinimum.x,
                 quad.uvMaximum.y - quad.uvMinimum.y},
                color,
                DebugFontTextureId,
                clip);
        }
    }

    void Context::AdvanceKeyboardFocus()
    {
        std::vector<WidgetId> previous;
        previous.reserve(m_elements.size());
        for (WidgetId id : m_drawOrder)
        {
            const auto found = m_elements.find(id);
            if (found != m_elements.end() && found->second.interactive &&
                found->second.visible && found->second.enabled)
            {
                previous.push_back(id);
            }
        }
        if (previous.empty())
        {
            m_focusedId = 0;
            return;
        }
        auto found = std::find(previous.begin(), previous.end(), m_focusedId);
        if (found == previous.end())
        {
            m_focusedId = previous.front();
            return;
        }
        ++found;
        m_focusedId = found == previous.end() ? previous.front() : *found;
    }

    void Context::ResetFrameState()
    {
        m_drawList.Clear();
        m_drawOrder.clear();
        m_interactiveOrder.clear();
        m_layoutStack.clear();
        m_hotId = 0;
        if (m_idStack.empty())
        {
            m_idStack.push_back(kFnvOffset);
        }
        else
        {
            m_idStack.resize(1);
            m_idStack[0] = kFnvOffset;
        }
    }

    ContextStats Context::GetStats() const
    {
        ContextStats stats;
        stats.retainedElements = static_cast<u32>(m_elements.size());
        for (const auto& [id, element] : m_elements)
        {
            (void)id;
            if (element.visible)
            {
                ++stats.visibleElements;
            }
            if (element.interactive)
            {
                ++stats.interactiveElements;
            }
        }
        stats.vertices = static_cast<u32>(m_drawList.GetVertices().size());
        stats.indices = static_cast<u32>(m_drawList.GetIndices().size());
        stats.batches = static_cast<u32>(m_drawList.GetBatches().size());
        return stats;
    }
} // namespace Pyramid::UI
