#include <Pyramid/UI/UI.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace Pyramid::UI
{
    namespace
    {
        struct EditLine
        {
            std::size_t start = 0;
            std::size_t end = 0;
            std::vector<f32> carets;
            f32 width = 0.0f;
        };

        struct EditLayout
        {
            std::vector<EditLine> lines;
            f32 lineHeight = 0.0f;
        };

        std::u32string DisplayText(const Text::TextBuffer& buffer, bool password)
        {
            if (!password)
            {
                return buffer.GetText();
            }
            std::u32string masked;
            masked.reserve(buffer.Size());
            for (char32_t codepoint : buffer.GetText())
            {
                masked.push_back(codepoint == U'\n' ? U'\n' : U'*');
            }
            return masked;
        }

        EditLayout BuildEditLayout(
            const Text::FontAtlas& font,
            std::u32string_view text,
            f32 scale)
        {
            EditLayout result;
            result.lineHeight = font.lineHeight * scale;
            std::size_t lineStart = 0;
            while (lineStart <= text.size())
            {
                std::size_t lineEnd = lineStart;
                while (lineEnd < text.size() && text[lineEnd] != U'\n')
                {
                    ++lineEnd;
                }

                EditLine line;
                line.start = lineStart;
                line.end = lineEnd;
                line.carets.reserve(lineEnd - lineStart + 1U);
                line.carets.push_back(0.0f);
                f32 x = 0.0f;
                char32_t previous = 0;
                for (std::size_t index = lineStart; index < lineEnd; ++index)
                {
                    const char32_t codepoint = text[index];
                    if (previous != 0)
                    {
                        x += font.GetKerning(previous, codepoint) * scale;
                    }
                    x += font.GetGlyph(codepoint).advance * scale;
                    line.carets.push_back(x);
                    previous = codepoint;
                }
                line.width = x;
                result.lines.push_back(std::move(line));

                if (lineEnd >= text.size())
                {
                    break;
                }
                lineStart = lineEnd + 1U;
            }
            if (result.lines.empty())
            {
                result.lines.push_back({});
            }
            return result;
        }

        std::pair<std::size_t, std::size_t> LocateCursor(
            const EditLayout& layout,
            std::size_t cursor)
        {
            for (std::size_t lineIndex = 0; lineIndex < layout.lines.size(); ++lineIndex)
            {
                const EditLine& line = layout.lines[lineIndex];
                if (cursor <= line.end || lineIndex + 1U == layout.lines.size())
                {
                    const std::size_t local = cursor <= line.start
                        ? 0U
                        : (std::min)(cursor - line.start, line.end - line.start);
                    return {lineIndex, local};
                }
            }
            return {layout.lines.size() - 1U, layout.lines.back().carets.size() - 1U};
        }

        f32 CaretX(const EditLine& line, std::size_t cursor)
        {
            const std::size_t local = cursor <= line.start
                ? 0
                : (std::min)(cursor - line.start, line.carets.size() - 1U);
            return line.carets[local];
        }

        bool IsControlDown(const InputState& input)
        {
            return input.IsKeyDown(Key::LeftControl) || input.IsKeyDown(Key::RightControl);
        }

        bool IsShiftDown(const InputState& input)
        {
            return input.IsKeyDown(Key::LeftShift) || input.IsKeyDown(Key::RightShift);
        }

        Color WithAlpha(const Color& color, f32 alpha)
        {
            return Color(color.r, color.g, color.b, alpha);
        }
    } // namespace

    TextEditResult Context::TextField(
        std::string_view label,
        std::string& utf8Value,
        const TextFieldOptions& options)
    {
        return EditText(label, utf8Value, options, false, 0.0f);
    }

    TextEditResult Context::PasswordField(
        std::string_view label,
        std::string& utf8Value,
        TextFieldOptions options)
    {
        options.password = true;
        return EditText(label, utf8Value, options, false, 0.0f);
    }

    TextEditResult Context::SearchField(
        std::string_view label,
        std::string& utf8Value,
        TextFieldOptions options)
    {
        if (options.placeholder.empty())
        {
            options.placeholder = "Search";
        }
        return EditText(label, utf8Value, options, false, 0.0f);
    }

    TextEditResult Context::MultilineTextArea(
        std::string_view label,
        std::string& utf8Value,
        const TextAreaOptions& options)
    {
        TextFieldOptions converted;
        converted.placeholder = options.placeholder;
        converted.maximumCharacters = options.maximumCharacters;
        converted.enabled = options.enabled;
        converted.readOnly = options.readOnly;
        converted.submitOnEnter = options.submitOnControlEnter;
        converted.hasError = options.hasError;
        return EditText(label, utf8Value, converted, true, options.height);
    }

    TextEditResult Context::EditText(
        std::string_view label,
        std::string& utf8Value,
        const TextFieldOptions& options,
        bool multiline,
        f32 multilineHeight)
    {
        TextEditResult result;
        if (!m_frameActive || m_layoutStack.empty())
        {
            return result;
        }

        const f32 editorHeight = multiline
            ? (std::max)(64.0f, multilineHeight)
            : (options.height > 0.0f ? options.height : m_theme.defaultRowHeight + 8.0f);
        ItemOptions item;
        item.height = editorHeight + 17.0f;
        item.enabled = options.enabled;
        const Rect rect = Allocate(item, item.height);
        if (!rect.IsValid())
        {
            return result;
        }

        const WidgetId id = MakeId(label);
        const Rect clip = m_layoutStack.back().clip;
        const Rect fieldRect{
            rect.x,
            rect.y + 17.0f,
            rect.width,
            (std::max)(0.0f, rect.height - 17.0f)};
        const Rect fieldClip = fieldRect.Intersect(clip);
        TextEditState& state = m_textEditStates[id];
        const bool focusedBefore = m_focusedId == id;

        if (!state.initialized || (!focusedBefore && state.synchronizedValue != utf8Value))
        {
            state.buffer.SetText(Text::DecodeUtf8(utf8Value).text);
            state.synchronizedValue = utf8Value;
            state.initialized = true;
            state.horizontalOffset = 0.0f;
            state.verticalOffset = 0.0f;
        }
        const std::u32string textBeforeConfiguration = state.buffer.GetText();
        state.buffer.SetMaximumCharacters(options.maximumCharacters);
        state.buffer.SetSingleLine(!multiline);
        state.buffer.SetReadOnly(options.readOnly);
        bool changed = state.buffer.GetText() != textBeforeConfiguration;

        const bool focusedAtStart = m_focusedId == id;
        if (focusedAtStart && !state.wasFocused)
        {
            state.focusSnapshot = state.buffer.GetText();
            state.caretTimer = 0.0f;
        }

        const bool hovered = options.enabled && fieldClip.IsValid() && IsHovered(fieldRect);
        if (hovered && m_input && m_input->WasMouseButtonPressed(MouseButton::Left))
        {
            const bool newFocus = m_focusedId != id;
            m_focusedId = id;
            m_activeId = id;
            state.dragging = true;
            if (newFocus)
            {
                state.focusSnapshot = state.buffer.GetText();
                state.clickCount = 0;
                if (options.selectAllOnFocus)
                {
                    state.buffer.SelectAll();
                }
            }

            const Math::Vec2 pointer(
                m_input->GetMousePosition().x,
                m_input->GetMousePosition().y);
            const f32 dx = pointer.x - state.lastClickPosition.x;
            const f32 dy = pointer.y - state.lastClickPosition.y;
            if (m_elapsedTime - state.lastClickTime <= 0.42f && dx * dx + dy * dy <= 25.0f)
            {
                state.clickCount = state.clickCount % 3U + 1U;
            }
            else
            {
                state.clickCount = 1;
            }
            state.lastClickTime = m_elapsedTime;
            state.lastClickPosition = pointer;

            const std::size_t hit = HitTestText(
                state, fieldRect, pointer, options.password, multiline);
            if (state.clickCount == 2U)
            {
                (void)state.buffer.SelectWordAt(hit);
                state.dragAnchor = state.buffer.GetSelection().begin;
            }
            else if (state.clickCount == 3U)
            {
                (void)state.buffer.SelectLineAt(hit);
                state.dragAnchor = state.buffer.GetSelection().begin;
            }
            else if (!options.selectAllOnFocus || !newFocus)
            {
                state.buffer.SetCursor(hit);
                state.dragAnchor = hit;
            }
            state.caretTimer = 0.0f;
        }

        if (m_activeId == id && state.dragging && m_input)
        {
            if (m_input->IsMouseButtonDown(MouseButton::Left) && state.clickCount == 1U &&
                m_input->HasMousePosition())
            {
                const Math::Vec2 pointer(
                    m_input->GetMousePosition().x,
                    m_input->GetMousePosition().y);
                const std::size_t hit = HitTestText(
                    state, fieldRect, pointer, options.password, multiline);
                state.buffer.SetSelection({state.dragAnchor, hit});
                state.caretTimer = 0.0f;
            }
            else if (!m_input->IsMouseButtonDown(MouseButton::Left))
            {
                state.dragging = false;
                m_activeId = 0;
            }
        }

        const bool focused = m_focusedId == id;
        if (focused && options.enabled && m_input)
        {
            const bool control = IsControlDown(*m_input);
            const bool shift = IsShiftDown(*m_input);

            if (control && m_input->WasKeyPressed(Key::A))
            {
                state.buffer.SelectAll();
                state.caretTimer = 0.0f;
            }
            if (control && m_input->WasKeyPressed(Key::C) && state.buffer.HasSelection())
            {
                if (m_clipboard)
                {
                    std::string error;
                    const bool copied = m_clipboard->SetText(state.buffer.GetSelectedText(), &error);
                    m_textEditDebug.clipboardStatus = copied ? "Copied" : error;
                }
                else
                {
                    m_textEditDebug.clipboardStatus = "Clipboard unavailable";
                }
            }
            if (control && m_input->WasKeyPressed(Key::X) && state.buffer.HasSelection() &&
                !options.readOnly)
            {
                if (m_clipboard)
                {
                    std::string error;
                    if (m_clipboard->SetText(state.buffer.GetSelectedText(), &error))
                    {
                        changed = state.buffer.DeleteSelection() || changed;
                        m_textEditDebug.clipboardStatus = "Cut";
                    }
                    else
                    {
                        m_textEditDebug.clipboardStatus = error;
                    }
                }
                else
                {
                    m_textEditDebug.clipboardStatus = "Clipboard unavailable";
                }
            }
            if (control && m_input->WasKeyPressed(Key::V) && !options.readOnly)
            {
                if (m_clipboard)
                {
                    const ClipboardTextResult pasted = m_clipboard->GetText();
                    if (pasted.success)
                    {
                        changed = state.buffer.Insert(pasted.text) || changed;
                        m_textEditDebug.clipboardStatus = "Pasted";
                    }
                    else
                    {
                        m_textEditDebug.clipboardStatus = pasted.error;
                    }
                }
                else
                {
                    m_textEditDebug.clipboardStatus = "Clipboard unavailable";
                }
            }

            for (const TextInputEvent& event : m_input->GetTextInputEvents())
            {
                if (event.type != TextInputEventType::Commit)
                {
                    continue;
                }
                std::u32string accepted;
                accepted.reserve(event.text.size());
                for (char32_t codepoint : event.text)
                {
                    if (codepoint == U'\r' || codepoint == U'\n' || codepoint == U'\t')
                    {
                        continue;
                    }
                    if (codepoint >= 0x20U && codepoint != 0x7fU)
                    {
                        accepted.push_back(codepoint);
                    }
                }
                if (!accepted.empty())
                {
                    changed = state.buffer.Insert(accepted) || changed;
                }
            }

            if (m_input->WasKeyPressed(Key::Backspace))
            {
                changed = state.buffer.Backspace() || changed;
            }
            if (m_input->WasKeyPressed(Key::Delete))
            {
                changed = state.buffer.DeleteForward() || changed;
            }
            if (m_input->WasKeyPressed(Key::Left))
            {
                (void)state.buffer.MoveCursor(
                    control ? Text::CursorMove::PreviousWord : Text::CursorMove::PreviousCodepoint,
                    shift);
                state.caretTimer = 0.0f;
            }
            if (m_input->WasKeyPressed(Key::Right))
            {
                (void)state.buffer.MoveCursor(
                    control ? Text::CursorMove::NextWord : Text::CursorMove::NextCodepoint,
                    shift);
                state.caretTimer = 0.0f;
            }
            if (m_input->WasKeyPressed(Key::Home))
            {
                (void)state.buffer.MoveCursor(
                    control ? Text::CursorMove::DocumentStart : Text::CursorMove::LineStart,
                    shift);
                state.caretTimer = 0.0f;
            }
            if (m_input->WasKeyPressed(Key::End))
            {
                (void)state.buffer.MoveCursor(
                    control ? Text::CursorMove::DocumentEnd : Text::CursorMove::LineEnd,
                    shift);
                state.caretTimer = 0.0f;
            }
            if (multiline && m_input->WasKeyPressed(Key::Up))
            {
                (void)state.buffer.MoveCursor(Text::CursorMove::LineUp, shift);
                state.caretTimer = 0.0f;
            }
            if (multiline && m_input->WasKeyPressed(Key::Down))
            {
                (void)state.buffer.MoveCursor(Text::CursorMove::LineDown, shift);
                state.caretTimer = 0.0f;
            }
            if (m_input->WasKeyPressed(Key::Enter))
            {
                if (!multiline || (control && options.submitOnEnter))
                {
                    result.submitted = true;
                }
                else if (!options.readOnly)
                {
                    changed = state.buffer.Insert(U"\n") || changed;
                }
            }
            if (m_input->WasKeyPressed(Key::Escape))
            {
                if (state.buffer.GetText() != state.focusSnapshot)
                {
                    state.buffer.SetText(state.focusSnapshot);
                    changed = true;
                }
                m_focusedId = 0;
                m_activeId = 0;
                state.dragging = false;
                result.cancelled = true;
            }
        }

        if (multiline && hovered && m_input && m_input->GetMouseWheelDelta() != 0.0f)
        {
            state.verticalOffset = (std::max)(
                0.0f,
                state.verticalOffset - m_input->GetMouseWheelDelta() *
                    m_font.lineHeight * m_theme.textScale * 3.0f);
        }

        if (changed)
        {
            utf8Value = state.buffer.GetUtf8();
            state.synchronizedValue = utf8Value;
            state.caretTimer = 0.0f;
        }
        else if (state.synchronizedValue != utf8Value && m_focusedId != id)
        {
            state.synchronizedValue = utf8Value;
        }

        RecordElement(
            id,
            multiline ? ElementKind::TextArea : ElementKind::TextField,
            rect,
            clip,
            options.enabled,
            true,
            true);
        DrawText(label, Math::Vec2(rect.x, rect.y),
            options.enabled ? m_theme.mutedText : m_theme.disabled, clip);
        DrawEditableText(
            id,
            state,
            fieldRect,
            fieldClip,
            options.placeholder,
            options.password,
            multiline,
            options.enabled,
            options.hasError);

        result.changed = changed;
        result.focused = m_focusedId == id;
        state.wasFocused = result.focused;
        if (result.focused)
        {
            m_textEditDebug.widget = id;
            m_textEditDebug.cursor = state.buffer.GetCursor();
            m_textEditDebug.selection = state.buffer.GetSelection();
        }
        return result;
    }

    void Context::DrawEditableText(
        WidgetId id,
        TextEditState& state,
        const Rect& rect,
        const Rect& clip,
        std::string_view placeholder,
        bool password,
        bool multiline,
        bool enabled,
        bool hasError)
    {
        DrawSolid(rect, m_theme.background, clip);
        const Color border = hasError
            ? Color(0.9f, 0.25f, 0.25f, 1.0f)
            : (m_focusedId == id ? m_theme.accent : m_theme.border);
        DrawBorder(rect, border, clip);

        const Rect content{
            rect.x + 5.0f,
            rect.y + 4.0f,
            (std::max)(0.0f, rect.width - 10.0f),
            (std::max)(0.0f, rect.height - 8.0f)};
        const std::u32string display = DisplayText(state.buffer, password);
        const EditLayout layout = BuildEditLayout(m_font, display, m_theme.textScale);
        const auto [caretLineIndex, unusedLocal] = LocateCursor(layout, state.buffer.GetCursor());
        (void)unusedLocal;
        const EditLine& caretLine = layout.lines[caretLineIndex];
        const f32 caretX = CaretX(caretLine, state.buffer.GetCursor());
        const f32 caretY = static_cast<f32>(caretLineIndex) * layout.lineHeight;

        if (!multiline)
        {
            if (caretX - state.horizontalOffset > content.width - 2.0f)
            {
                state.horizontalOffset = caretX - content.width + 2.0f;
            }
            if (caretX - state.horizontalOffset < 0.0f)
            {
                state.horizontalOffset = caretX;
            }
            state.horizontalOffset = (std::max)(0.0f, state.horizontalOffset);
        }
        else
        {
            if (caretY - state.verticalOffset + layout.lineHeight > content.height)
            {
                state.verticalOffset = caretY + layout.lineHeight - content.height;
            }
            if (caretY - state.verticalOffset < 0.0f)
            {
                state.verticalOffset = caretY;
            }
            const f32 maximumOffset = (std::max)(
                0.0f,
                static_cast<f32>(layout.lines.size()) * layout.lineHeight - content.height);
            state.verticalOffset = (std::min)(state.verticalOffset, maximumOffset);
        }

        const Text::TextRange selection = state.buffer.GetSelection();
        if (!selection.Empty())
        {
            for (std::size_t lineIndex = 0; lineIndex < layout.lines.size(); ++lineIndex)
            {
                const EditLine& line = layout.lines[lineIndex];
                const std::size_t begin = (std::max)(selection.begin, line.start);
                const std::size_t end = (std::min)(selection.end, line.end);
                if (end <= begin)
                {
                    continue;
                }
                const f32 left = CaretX(line, begin);
                const f32 right = CaretX(line, end);
                DrawSolid(
                    {content.x + left - state.horizontalOffset,
                     content.y + static_cast<f32>(lineIndex) * layout.lineHeight - state.verticalOffset,
                     (std::max)(1.0f, right - left),
                     layout.lineHeight},
                    WithAlpha(m_theme.accent, 0.42f),
                    content.Intersect(clip));
            }
        }

        if (display.empty() && !placeholder.empty())
        {
            DrawText(
                placeholder,
                Math::Vec2(content.x, content.y),
                enabled ? m_theme.mutedText : m_theme.disabled,
                content.Intersect(clip));
        }
        else
        {
            for (std::size_t lineIndex = 0; lineIndex < layout.lines.size(); ++lineIndex)
            {
                const EditLine& line = layout.lines[lineIndex];
                const std::u32string_view lineText(
                    display.data() + line.start,
                    line.end - line.start);
                DrawText(
                    Text::EncodeUtf8(lineText),
                    Math::Vec2(
                        content.x - state.horizontalOffset,
                        content.y + static_cast<f32>(lineIndex) * layout.lineHeight -
                            state.verticalOffset),
                    enabled ? m_theme.text : m_theme.disabled,
                    content.Intersect(clip));
            }
        }

        state.caretTimer += (std::max)(0.0f, m_frame.deltaTime);
        if (m_focusedId == id && std::fmod(state.caretTimer, 1.0f) < 0.55f)
        {
            DrawSolid(
                {content.x + caretX - state.horizontalOffset,
                 content.y + caretY - state.verticalOffset,
                 1.5f,
                 layout.lineHeight},
                enabled ? m_theme.text : m_theme.disabled,
                content.Intersect(clip));
        }
    }

    std::size_t Context::HitTestText(
        const TextEditState& state,
        const Rect& contentRect,
        const Math::Vec2& pointer,
        bool password,
        bool multiline) const
    {
        const Rect content{
            contentRect.x + 5.0f,
            contentRect.y + 4.0f,
            (std::max)(0.0f, contentRect.width - 10.0f),
            (std::max)(0.0f, contentRect.height - 8.0f)};
        const std::u32string display = DisplayText(state.buffer, password);
        const EditLayout layout = BuildEditLayout(m_font, display, m_theme.textScale);
        std::size_t lineIndex = 0;
        if (multiline && layout.lineHeight > 0.0f)
        {
            const f32 localY = pointer.y - content.y + state.verticalOffset;
            lineIndex = static_cast<std::size_t>((std::max)(0.0f, localY) / layout.lineHeight);
            lineIndex = (std::min)(lineIndex, layout.lines.size() - 1U);
        }
        const EditLine& line = layout.lines[lineIndex];
        const f32 localX = pointer.x - content.x + state.horizontalOffset;
        std::size_t local = 0;
        for (std::size_t index = 1; index < line.carets.size(); ++index)
        {
            const f32 midpoint = (line.carets[index - 1U] + line.carets[index]) * 0.5f;
            if (localX < midpoint)
            {
                break;
            }
            local = index;
        }
        return line.start + local;
    }
} // namespace Pyramid::UI
