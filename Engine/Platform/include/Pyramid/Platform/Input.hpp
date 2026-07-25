#pragma once

#include <Pyramid/Core/Prerequisites.hpp>

#include <array>
#include <cstddef>

namespace Pyramid
{
    /**
     * @brief Platform-neutral keyboard keys understood by Pyramid.
     */
    enum class Key : u16
    {
        Unknown = 0,
        Space,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Semicolon,
        Equal,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,
        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,
        F21,
        F22,
        F23,
        F24,
        Keypad0,
        Keypad1,
        Keypad2,
        Keypad3,
        Keypad4,
        Keypad5,
        Keypad6,
        Keypad7,
        Keypad8,
        Keypad9,
        KeypadDecimal,
        KeypadDivide,
        KeypadMultiply,
        KeypadSubtract,
        KeypadAdd,
        KeypadEnter,
        LeftShift,
        LeftControl,
        LeftAlt,
        LeftSuper,
        RightShift,
        RightControl,
        RightAlt,
        RightSuper,
        Menu,
        Count
    };

    enum class MouseButton : u8
    {
        Left = 0,
        Right,
        Middle,
        X1,
        X2,
        Count
    };

    struct MousePosition
    {
        f32 x = 0.0f;
        f32 y = 0.0f;
    };

    struct MouseDelta
    {
        f32 x = 0.0f;
        f32 y = 0.0f;
    };

    /**
     * @brief Per-window input state with one-frame transition tracking.
     *
     * Platform adapters call BeginFrame() before pumping native messages, then
     * feed key, button, pointer, wheel, and focus events into this object.
     * Game code only needs the query methods.
     */
    class InputState final
    {
    public:
        InputState() = default;

        void BeginFrame();
        void SetFocused(bool focused);
        void ProcessKey(Key key, bool down);
        void ProcessMouseButton(MouseButton button, bool down);
        void ProcessMouseMove(f32 x, f32 y);
        void ProcessMouseWheel(f32 verticalSteps, f32 horizontalSteps = 0.0f);
        void ReleaseMouseButtons();
        void Reset(bool emitReleaseEvents = true);

        [[nodiscard]] bool HasFocus() const { return m_focused; }

        [[nodiscard]] bool IsKeyDown(Key key) const;
        [[nodiscard]] bool WasKeyPressed(Key key) const;
        [[nodiscard]] bool WasKeyReleased(Key key) const;

        [[nodiscard]] bool IsMouseButtonDown(MouseButton button) const;
        [[nodiscard]] bool WasMouseButtonPressed(MouseButton button) const;
        [[nodiscard]] bool WasMouseButtonReleased(MouseButton button) const;

        [[nodiscard]] MousePosition GetMousePosition() const { return m_mousePosition; }
        [[nodiscard]] MouseDelta GetMouseDelta() const { return m_mouseDelta; }
        [[nodiscard]] f32 GetMouseWheelDelta() const { return m_mouseWheelDelta; }
        [[nodiscard]] f32 GetMouseHorizontalWheelDelta() const { return m_mouseHorizontalWheelDelta; }

    private:
        static constexpr std::size_t KeyCount = static_cast<std::size_t>(Key::Count);
        static constexpr std::size_t MouseButtonCount = static_cast<std::size_t>(MouseButton::Count);

        [[nodiscard]] static bool IsValidKey(Key key);
        [[nodiscard]] static bool IsValidMouseButton(MouseButton button);

        std::array<bool, KeyCount> m_keysDown{};
        std::array<bool, KeyCount> m_keysPressed{};
        std::array<bool, KeyCount> m_keysReleased{};
        std::array<bool, MouseButtonCount> m_mouseButtonsDown{};
        std::array<bool, MouseButtonCount> m_mouseButtonsPressed{};
        std::array<bool, MouseButtonCount> m_mouseButtonsReleased{};

        MousePosition m_mousePosition{};
        MouseDelta m_mouseDelta{};
        f32 m_mouseWheelDelta = 0.0f;
        f32 m_mouseHorizontalWheelDelta = 0.0f;
        bool m_hasMousePosition = false;
        bool m_focused = false;
    };
} // namespace Pyramid
