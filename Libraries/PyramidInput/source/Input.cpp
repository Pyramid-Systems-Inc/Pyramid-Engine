#include <Pyramid/Platform/Input.hpp>
#include <utility>


namespace Pyramid
{
    void InputState::BeginFrame()
    {
        m_keysPressed.fill(false);
        m_keysReleased.fill(false);
        m_mouseButtonsPressed.fill(false);
        m_mouseButtonsReleased.fill(false);
        m_mouseDelta = {};
        m_mouseWheelDelta = 0.0f;
        m_mouseHorizontalWheelDelta = 0.0f;
        m_textInputEvents.clear();
    }

    void InputState::SetFocused(bool focused)
    {
        if (m_focused == focused)
        {
            return;
        }

        m_focused = focused;
        m_hasMousePosition = false;
        m_mouseDelta = {};

        if (!focused)
        {
            Reset(true);
            m_focused = false;
        }
    }

    void InputState::ProcessKey(Key key, bool down)
    {
        if (!IsValidKey(key))
        {
            return;
        }

        const std::size_t index = static_cast<std::size_t>(key);
        if (down)
        {
            if (!m_keysDown[index])
            {
                m_keysPressed[index] = true;
            }
        }
        else if (m_keysDown[index])
        {
            m_keysReleased[index] = true;
        }

        m_keysDown[index] = down;
    }

    void InputState::ProcessMouseButton(MouseButton button, bool down)
    {
        if (!IsValidMouseButton(button))
        {
            return;
        }

        const std::size_t index = static_cast<std::size_t>(button);
        if (down)
        {
            if (!m_mouseButtonsDown[index])
            {
                m_mouseButtonsPressed[index] = true;
            }
        }
        else if (m_mouseButtonsDown[index])
        {
            m_mouseButtonsReleased[index] = true;
        }

        m_mouseButtonsDown[index] = down;
    }

    void InputState::ProcessMouseMove(f32 x, f32 y)
    {
        if (m_hasMousePosition)
        {
            m_mouseDelta.x += x - m_mousePosition.x;
            m_mouseDelta.y += y - m_mousePosition.y;
        }

        m_mousePosition = {x, y};
        m_hasMousePosition = true;
    }

    void InputState::ProcessMouseWheel(f32 verticalSteps, f32 horizontalSteps)
    {
        m_mouseWheelDelta += verticalSteps;
        m_mouseHorizontalWheelDelta += horizontalSteps;
    }

    void InputState::ProcessTextCodepoint(char32_t codepoint)
    {
        if (!m_focused)
        {
            return;
        }
        if (codepoint > 0x10ffffU || (codepoint >= 0xd800U && codepoint <= 0xdfffU))
        {
            codepoint = U'\uFFFD';
        }
        TextInputEvent event;
        event.type = TextInputEventType::Commit;
        event.text.push_back(codepoint);
        m_textInputEvents.push_back(std::move(event));
    }

    void InputState::ProcessUtf16CodeUnit(char16_t codeUnit)
    {
        if (!m_focused)
        {
            m_pendingHighSurrogate = 0;
            return;
        }

        if (codeUnit >= 0xd800U && codeUnit <= 0xdbffU)
        {
            if (m_pendingHighSurrogate != 0)
            {
                ProcessTextCodepoint(U'\uFFFD');
            }
            m_pendingHighSurrogate = codeUnit;
            return;
        }

        if (codeUnit >= 0xdc00U && codeUnit <= 0xdfffU)
        {
            if (m_pendingHighSurrogate == 0)
            {
                ProcessTextCodepoint(U'\uFFFD');
                return;
            }
            const char32_t high = static_cast<char32_t>(m_pendingHighSurrogate) - 0xd800U;
            const char32_t low = static_cast<char32_t>(codeUnit) - 0xdc00U;
            m_pendingHighSurrogate = 0;
            ProcessTextCodepoint(0x10000U + (high << 10U) + low);
            return;
        }

        if (m_pendingHighSurrogate != 0)
        {
            m_pendingHighSurrogate = 0;
            ProcessTextCodepoint(U'\uFFFD');
        }
        ProcessTextCodepoint(static_cast<char32_t>(codeUnit));
    }

    void InputState::ProcessTextEvent(TextInputEvent event)
    {
        if (!m_focused ||
            (event.type == TextInputEventType::Commit && event.text.empty()))
        {
            return;
        }
        for (char32_t& codepoint : event.text)
        {
            if (codepoint > 0x10ffffU || (codepoint >= 0xd800U && codepoint <= 0xdfffU))
            {
                codepoint = U'\uFFFD';
            }
        }
        m_textInputEvents.push_back(std::move(event));
    }

    void InputState::ReleaseMouseButtons()
    {
        for (std::size_t index = 0; index < MouseButtonCount; ++index)
        {
            if (m_mouseButtonsDown[index])
            {
                m_mouseButtonsReleased[index] = true;
                m_mouseButtonsDown[index] = false;
            }
        }
    }

    void InputState::Reset(bool emitReleaseEvents)
    {
        for (std::size_t index = 0; index < KeyCount; ++index)
        {
            if (emitReleaseEvents && m_keysDown[index])
            {
                m_keysReleased[index] = true;
            }
            m_keysDown[index] = false;
            m_keysPressed[index] = false;
        }

        for (std::size_t index = 0; index < MouseButtonCount; ++index)
        {
            if (emitReleaseEvents && m_mouseButtonsDown[index])
            {
                m_mouseButtonsReleased[index] = true;
            }
            m_mouseButtonsDown[index] = false;
            m_mouseButtonsPressed[index] = false;
        }

        if (!emitReleaseEvents)
        {
            m_keysReleased.fill(false);
            m_mouseButtonsReleased.fill(false);
        }

        m_mouseDelta = {};
        m_mouseWheelDelta = 0.0f;
        m_mouseHorizontalWheelDelta = 0.0f;
        m_hasMousePosition = false;
        m_textInputEvents.clear();
        m_pendingHighSurrogate = 0;
    }

    bool InputState::IsKeyDown(Key key) const
    {
        return IsValidKey(key) && m_keysDown[static_cast<std::size_t>(key)];
    }

    bool InputState::WasKeyPressed(Key key) const
    {
        return IsValidKey(key) && m_keysPressed[static_cast<std::size_t>(key)];
    }

    bool InputState::WasKeyReleased(Key key) const
    {
        return IsValidKey(key) && m_keysReleased[static_cast<std::size_t>(key)];
    }

    bool InputState::IsMouseButtonDown(MouseButton button) const
    {
        return IsValidMouseButton(button) && m_mouseButtonsDown[static_cast<std::size_t>(button)];
    }

    bool InputState::WasMouseButtonPressed(MouseButton button) const
    {
        return IsValidMouseButton(button) && m_mouseButtonsPressed[static_cast<std::size_t>(button)];
    }

    bool InputState::WasMouseButtonReleased(MouseButton button) const
    {
        return IsValidMouseButton(button) && m_mouseButtonsReleased[static_cast<std::size_t>(button)];
    }

    bool InputState::IsValidKey(Key key)
    {
        const std::size_t index = static_cast<std::size_t>(key);
        return key != Key::Unknown && index < KeyCount;
    }

    bool InputState::IsValidMouseButton(MouseButton button)
    {
        return static_cast<std::size_t>(button) < MouseButtonCount;
    }
} // namespace Pyramid
