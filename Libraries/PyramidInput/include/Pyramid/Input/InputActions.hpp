#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Platform/Input.hpp>

#include <array>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Pyramid
{

    /**
     * @brief Physical controls reserved before named action evaluation.
     *
     * UI and editor layers use this mask to prevent an already handled pointer
     * or keyboard event from leaking into lower-level gameplay contexts.
     */
    class InputConsumptionMask final
    {
    public:
        void Clear();
        void Merge(const InputConsumptionMask& other);

        void ConsumeKey(Key key);
        void ConsumeMouseButton(MouseButton button);
        void ConsumeMouseDelta();
        void ConsumeMouseWheel();
        void ConsumeAllMouse();

        [[nodiscard]] bool IsKeyConsumed(Key key) const;
        [[nodiscard]] bool IsMouseButtonConsumed(MouseButton button) const;
        [[nodiscard]] bool IsMouseDeltaXConsumed() const { return m_mouseDeltaX; }
        [[nodiscard]] bool IsMouseDeltaYConsumed() const { return m_mouseDeltaY; }
        [[nodiscard]] bool IsMouseWheelConsumed() const { return m_mouseWheel; }
        [[nodiscard]] bool IsMouseHorizontalWheelConsumed() const
        {
            return m_mouseHorizontalWheel;
        }
        [[nodiscard]] bool HasAnyConsumption() const;

    private:
        static constexpr std::size_t KeyCount = static_cast<std::size_t>(Key::Count);
        static constexpr std::size_t MouseButtonCount =
            static_cast<std::size_t>(MouseButton::Count);

        std::array<bool, KeyCount> m_keys{};
        std::array<bool, MouseButtonCount> m_mouseButtons{};
        bool m_mouseDeltaX = false;
        bool m_mouseDeltaY = false;
        bool m_mouseWheel = false;
        bool m_mouseHorizontalWheel = false;
    };

    /**
     * @brief Shape of the value produced by an input action.
     */
    enum class InputActionType : u8
    {
        Button = 0,
        Axis1D,
        Axis2D
    };

    /**
     * @brief Axis selected by a binding on a two-dimensional action.
     */
    enum class InputAxisComponent : u8
    {
        X = 0,
        Y
    };

    /**
     * @brief Platform-neutral physical source read from InputState.
     */
    enum class InputBindingSource : u8
    {
        Key = 0,
        MouseButton,
        MouseDeltaX,
        MouseDeltaY,
        MouseWheel,
        MouseHorizontalWheel
    };

    struct InputActionVector2
    {
        f32 x = 0.0f;
        f32 y = 0.0f;
    };

    /**
     * @brief One physical input binding for a named action.
     *
     * A binding can optionally require one keyboard key and/or one mouse
     * button to be held. This supports generic chords such as Alt+Enter and
     * gated analog input such as right-button mouse dragging without exposing
     * native platform codes.
     */
    struct InputBinding
    {
        InputBindingSource source = InputBindingSource::Key;
        InputAxisComponent component = InputAxisComponent::X;
        Key key = Key::Unknown;
        MouseButton mouseButton = MouseButton::Count;
        Key requiredKey = Key::Unknown;
        MouseButton requiredMouseButton = MouseButton::Count;
        f32 scale = 1.0f;

        [[nodiscard]] static InputBinding KeyBinding(
            Key key,
            f32 scale = 1.0f,
            InputAxisComponent component = InputAxisComponent::X);
        [[nodiscard]] static InputBinding MouseButtonBinding(
            MouseButton button,
            f32 scale = 1.0f,
            InputAxisComponent component = InputAxisComponent::X);
        [[nodiscard]] static InputBinding MouseDeltaXBinding(
            f32 scale = 1.0f,
            InputAxisComponent component = InputAxisComponent::X);
        [[nodiscard]] static InputBinding MouseDeltaYBinding(
            f32 scale = 1.0f,
            InputAxisComponent component = InputAxisComponent::Y);
        [[nodiscard]] static InputBinding MouseWheelBinding(
            f32 scale = 1.0f,
            InputAxisComponent component = InputAxisComponent::X);
        [[nodiscard]] static InputBinding MouseHorizontalWheelBinding(
            f32 scale = 1.0f,
            InputAxisComponent component = InputAxisComponent::X);

        InputBinding& RequireKey(Key modifier);
        InputBinding& RequireMouseButton(MouseButton modifier);

        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] bool IsDigital() const;
    };

    /**
     * @brief Evaluated state of one action for the current frame.
     */
    struct InputActionState
    {
        InputActionType type = InputActionType::Button;
        InputActionVector2 value{};
        bool active = false;
        bool pressed = false;
        bool released = false;

        [[nodiscard]] f32 GetValue() const { return value.x; }
        [[nodiscard]] InputActionVector2 GetValue2D() const { return value; }
        [[nodiscard]] bool IsDown() const { return active; }
        [[nodiscard]] bool WasPressed() const { return pressed; }
        [[nodiscard]] bool WasReleased() const { return released; }
    };

    /**
     * @brief Named, prioritized collection of input actions and bindings.
     *
     * Contexts are engine-generic. Games can create gameplay, UI, editor,
     * vehicle, console, photo-mode, or any other context without changing the
     * platform backend. Higher-priority consuming contexts prevent active
     * controls from reaching lower-priority contexts.
     */
    class InputContext final
    {
    public:
        [[nodiscard]] const std::string& GetName() const { return m_name; }
        [[nodiscard]] i32 GetPriority() const { return m_priority; }
        void SetPriority(i32 priority) { m_priority = priority; }

        [[nodiscard]] bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        [[nodiscard]] bool ConsumesInput() const { return m_consumeInput; }
        void SetConsumesInput(bool consumeInput) { m_consumeInput = consumeInput; }

        [[nodiscard]] bool AddAction(std::string name, InputActionType type);
        [[nodiscard]] bool RemoveAction(std::string_view name);
        [[nodiscard]] bool HasAction(std::string_view name) const;
        [[nodiscard]] std::size_t GetActionCount() const { return m_actions.size(); }

        [[nodiscard]] bool AddBinding(std::string_view actionName, const InputBinding& binding);
        [[nodiscard]] bool Rebind(
            std::string_view actionName,
            std::size_t bindingIndex,
            const InputBinding& replacement);
        [[nodiscard]] bool RemoveBinding(std::string_view actionName, std::size_t bindingIndex);
        [[nodiscard]] bool ClearBindings(std::string_view actionName);
        [[nodiscard]] std::size_t GetBindingCount(std::string_view actionName) const;
        [[nodiscard]] const InputBinding* GetBinding(
            std::string_view actionName,
            std::size_t bindingIndex) const;

        [[nodiscard]] const InputActionState* FindActionState(std::string_view name) const;

    private:
        friend class InputActionSystem;

        struct Action
        {
            std::string name;
            InputActionState state{};
            std::vector<InputBinding> bindings;
            bool previousActive = false;
        };

        InputContext(std::string name, i32 priority, bool consumeInput, u64 insertionOrder);

        [[nodiscard]] Action* FindAction(std::string_view name);
        [[nodiscard]] const Action* FindAction(std::string_view name) const;
        void ResetRuntimeState(bool emitRelease);

        std::string m_name;
        i32 m_priority = 0;
        bool m_enabled = true;
        bool m_consumeInput = true;
        u64 m_insertionOrder = 0;
        std::vector<Action> m_actions;
    };

    /**
     * @brief Evaluates named actions from the current platform input snapshot.
     */
    class InputActionSystem final
    {
    public:
        InputActionSystem() = default;
        InputActionSystem(const InputActionSystem&) = delete;
        InputActionSystem& operator=(const InputActionSystem&) = delete;

        [[nodiscard]] InputContext* CreateContext(
            std::string name,
            i32 priority = 0,
            bool consumeInput = true);
        [[nodiscard]] bool RemoveContext(std::string_view name);
        void ClearContexts();

        [[nodiscard]] InputContext* FindContext(std::string_view name);
        [[nodiscard]] const InputContext* FindContext(std::string_view name) const;
        [[nodiscard]] std::size_t GetContextCount() const { return m_contexts.size(); }

        /**
         * @brief Evaluate every enabled context from one InputState snapshot.
         *
         * Call exactly once after native messages are processed and before game
         * update logic reads action states. Game performs this automatically.
         */
        void Update(const InputState& input);
        void Update(const InputState& input, const InputConsumptionMask& consumed);

        [[nodiscard]] const InputActionState* FindActionState(
            std::string_view contextName,
            std::string_view actionName) const;

        /**
         * @brief Find an action in the highest-priority enabled context.
         */
        [[nodiscard]] const InputActionState* FindActionState(
            std::string_view actionName) const;

        [[nodiscard]] bool IsActionDown(
            std::string_view contextName,
            std::string_view actionName) const;
        [[nodiscard]] bool WasActionPressed(
            std::string_view contextName,
            std::string_view actionName) const;
        [[nodiscard]] bool WasActionReleased(
            std::string_view contextName,
            std::string_view actionName) const;
        [[nodiscard]] f32 GetActionValue(
            std::string_view contextName,
            std::string_view actionName) const;
        [[nodiscard]] InputActionVector2 GetActionValue2D(
            std::string_view contextName,
            std::string_view actionName) const;

    private:
        std::vector<std::unique_ptr<InputContext>> m_contexts;
        u64 m_nextInsertionOrder = 1;
    };
} // namespace Pyramid
