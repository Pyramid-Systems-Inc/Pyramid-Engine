#include <Pyramid/Input/InputActions.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace Pyramid
{
    namespace
    {
        constexpr f32 kActionEpsilon = 0.000001f;
        constexpr std::size_t kKeyCount = static_cast<std::size_t>(Key::Count);
        constexpr std::size_t kMouseButtonCount = static_cast<std::size_t>(MouseButton::Count);

        [[nodiscard]] bool IsValidKey(Key key)
        {
            return key != Key::Unknown &&
                static_cast<std::size_t>(key) < kKeyCount;
        }

        [[nodiscard]] bool IsValidMouseButton(MouseButton button)
        {
            return static_cast<std::size_t>(button) < kMouseButtonCount;
        }

        [[nodiscard]] f32 ClampDigitalAxis(f32 value)
        {
            return (std::max)(-1.0f, (std::min)(value, 1.0f));
        }

        struct ConsumedControls
        {
            std::array<bool, kKeyCount> keys{};
            std::array<bool, kMouseButtonCount> mouseButtons{};
            bool mouseDeltaX = false;
            bool mouseDeltaY = false;
            bool mouseWheel = false;
            bool mouseHorizontalWheel = false;
        };

        ConsumedControls ToConsumedControls(const InputConsumptionMask& mask)
        {
            ConsumedControls consumed;
            for (std::size_t index = 0; index < kKeyCount; ++index)
            {
                consumed.keys[index] = mask.IsKeyConsumed(static_cast<Key>(index));
            }
            for (std::size_t index = 0; index < kMouseButtonCount; ++index)
            {
                consumed.mouseButtons[index] =
                    mask.IsMouseButtonConsumed(static_cast<MouseButton>(index));
            }
            consumed.mouseDeltaX = mask.IsMouseDeltaXConsumed();
            consumed.mouseDeltaY = mask.IsMouseDeltaYConsumed();
            consumed.mouseWheel = mask.IsMouseWheelConsumed();
            consumed.mouseHorizontalWheel = mask.IsMouseHorizontalWheelConsumed();
            return consumed;
        }

        struct BindingSample
        {
            f32 value = 0.0f;
            bool down = false;
            bool pressed = false;
            bool released = false;
            bool available = true;

            [[nodiscard]] bool HasActivity() const
            {
                return down || pressed || released || std::fabs(value) > kActionEpsilon;
            }
        };

        [[nodiscard]] bool IsSourceConsumed(
            const InputBinding& binding,
            const ConsumedControls& consumed)
        {
            switch (binding.source)
            {
                case InputBindingSource::Key:
                    return consumed.keys[static_cast<std::size_t>(binding.key)];
                case InputBindingSource::MouseButton:
                    return consumed.mouseButtons[static_cast<std::size_t>(binding.mouseButton)];
                case InputBindingSource::MouseDeltaX:
                    return consumed.mouseDeltaX;
                case InputBindingSource::MouseDeltaY:
                    return consumed.mouseDeltaY;
                case InputBindingSource::MouseWheel:
                    return consumed.mouseWheel;
                case InputBindingSource::MouseHorizontalWheel:
                    return consumed.mouseHorizontalWheel;
            }
            return true;
        }

        [[nodiscard]] bool AreModifiersAvailable(
            const InputBinding& binding,
            const ConsumedControls& consumed)
        {
            if (IsValidKey(binding.requiredKey) &&
                consumed.keys[static_cast<std::size_t>(binding.requiredKey)])
            {
                return false;
            }

            if (IsValidMouseButton(binding.requiredMouseButton) &&
                consumed.mouseButtons[static_cast<std::size_t>(binding.requiredMouseButton)])
            {
                return false;
            }

            return true;
        }

        [[nodiscard]] bool AreModifiersHeld(const InputBinding& binding, const InputState& input)
        {
            if (IsValidKey(binding.requiredKey) && !input.IsKeyDown(binding.requiredKey))
            {
                return false;
            }

            if (IsValidMouseButton(binding.requiredMouseButton) &&
                !input.IsMouseButtonDown(binding.requiredMouseButton))
            {
                return false;
            }

            return true;
        }

        [[nodiscard]] BindingSample SampleBinding(
            const InputBinding& binding,
            const InputState& input,
            const ConsumedControls& consumed)
        {
            BindingSample sample;
            sample.available = !IsSourceConsumed(binding, consumed) &&
                AreModifiersAvailable(binding, consumed) &&
                AreModifiersHeld(binding, input);
            if (!sample.available)
            {
                return sample;
            }

            switch (binding.source)
            {
                case InputBindingSource::Key:
                    sample.down = input.IsKeyDown(binding.key);
                    sample.pressed = input.WasKeyPressed(binding.key);
                    sample.released = input.WasKeyReleased(binding.key);
                    sample.value = sample.down ? binding.scale : 0.0f;
                    break;
                case InputBindingSource::MouseButton:
                    sample.down = input.IsMouseButtonDown(binding.mouseButton);
                    sample.pressed = input.WasMouseButtonPressed(binding.mouseButton);
                    sample.released = input.WasMouseButtonReleased(binding.mouseButton);
                    sample.value = sample.down ? binding.scale : 0.0f;
                    break;
                case InputBindingSource::MouseDeltaX:
                    sample.value = input.GetMouseDelta().x * binding.scale;
                    sample.down = std::fabs(sample.value) > kActionEpsilon;
                    break;
                case InputBindingSource::MouseDeltaY:
                    sample.value = input.GetMouseDelta().y * binding.scale;
                    sample.down = std::fabs(sample.value) > kActionEpsilon;
                    break;
                case InputBindingSource::MouseWheel:
                    sample.value = input.GetMouseWheelDelta() * binding.scale;
                    sample.down = std::fabs(sample.value) > kActionEpsilon;
                    break;
                case InputBindingSource::MouseHorizontalWheel:
                    sample.value = input.GetMouseHorizontalWheelDelta() * binding.scale;
                    sample.down = std::fabs(sample.value) > kActionEpsilon;
                    break;
            }

            return sample;
        }


        void MergeConsumedControls(ConsumedControls& destination, const ConsumedControls& source)
        {
            for (std::size_t index = 0; index < kKeyCount; ++index)
            {
                destination.keys[index] = destination.keys[index] || source.keys[index];
            }
            for (std::size_t index = 0; index < kMouseButtonCount; ++index)
            {
                destination.mouseButtons[index] =
                    destination.mouseButtons[index] || source.mouseButtons[index];
            }
            destination.mouseDeltaX = destination.mouseDeltaX || source.mouseDeltaX;
            destination.mouseDeltaY = destination.mouseDeltaY || source.mouseDeltaY;
            destination.mouseWheel = destination.mouseWheel || source.mouseWheel;
            destination.mouseHorizontalWheel =
                destination.mouseHorizontalWheel || source.mouseHorizontalWheel;
        }
        void ConsumeBinding(const InputBinding& binding, ConsumedControls& consumed)
        {
            switch (binding.source)
            {
                case InputBindingSource::Key:
                    consumed.keys[static_cast<std::size_t>(binding.key)] = true;
                    break;
                case InputBindingSource::MouseButton:
                    consumed.mouseButtons[static_cast<std::size_t>(binding.mouseButton)] = true;
                    break;
                case InputBindingSource::MouseDeltaX:
                    consumed.mouseDeltaX = true;
                    break;
                case InputBindingSource::MouseDeltaY:
                    consumed.mouseDeltaY = true;
                    break;
                case InputBindingSource::MouseWheel:
                    consumed.mouseWheel = true;
                    break;
                case InputBindingSource::MouseHorizontalWheel:
                    consumed.mouseHorizontalWheel = true;
                    break;
            }

            if (IsValidKey(binding.requiredKey))
            {
                consumed.keys[static_cast<std::size_t>(binding.requiredKey)] = true;
            }
            if (IsValidMouseButton(binding.requiredMouseButton))
            {
                consumed.mouseButtons[static_cast<std::size_t>(binding.requiredMouseButton)] = true;
            }
        }
    }

    void InputConsumptionMask::Clear()
    {
        m_keys.fill(false);
        m_mouseButtons.fill(false);
        m_mouseDeltaX = false;
        m_mouseDeltaY = false;
        m_mouseWheel = false;
        m_mouseHorizontalWheel = false;
    }

    void InputConsumptionMask::Merge(const InputConsumptionMask& other)
    {
        for (std::size_t index = 0; index < m_keys.size(); ++index)
        {
            m_keys[index] = m_keys[index] || other.m_keys[index];
        }
        for (std::size_t index = 0; index < m_mouseButtons.size(); ++index)
        {
            m_mouseButtons[index] = m_mouseButtons[index] || other.m_mouseButtons[index];
        }
        m_mouseDeltaX = m_mouseDeltaX || other.m_mouseDeltaX;
        m_mouseDeltaY = m_mouseDeltaY || other.m_mouseDeltaY;
        m_mouseWheel = m_mouseWheel || other.m_mouseWheel;
        m_mouseHorizontalWheel =
            m_mouseHorizontalWheel || other.m_mouseHorizontalWheel;
    }

    void InputConsumptionMask::ConsumeKey(Key key)
    {
        const std::size_t index = static_cast<std::size_t>(key);
        if (key != Key::Unknown && index < m_keys.size())
        {
            m_keys[index] = true;
        }
    }

    void InputConsumptionMask::ConsumeMouseButton(MouseButton button)
    {
        const std::size_t index = static_cast<std::size_t>(button);
        if (index < m_mouseButtons.size())
        {
            m_mouseButtons[index] = true;
        }
    }

    void InputConsumptionMask::ConsumeMouseDelta()
    {
        m_mouseDeltaX = true;
        m_mouseDeltaY = true;
    }

    void InputConsumptionMask::ConsumeMouseWheel()
    {
        m_mouseWheel = true;
        m_mouseHorizontalWheel = true;
    }

    void InputConsumptionMask::ConsumeAllMouse()
    {
        m_mouseButtons.fill(true);
        ConsumeMouseDelta();
        ConsumeMouseWheel();
    }

    bool InputConsumptionMask::IsKeyConsumed(Key key) const
    {
        const std::size_t index = static_cast<std::size_t>(key);
        return key != Key::Unknown && index < m_keys.size() && m_keys[index];
    }

    bool InputConsumptionMask::IsMouseButtonConsumed(MouseButton button) const
    {
        const std::size_t index = static_cast<std::size_t>(button);
        return index < m_mouseButtons.size() && m_mouseButtons[index];
    }

    bool InputConsumptionMask::HasAnyMouseConsumption() const
    {
        return std::any_of(
                   m_mouseButtons.begin(),
                   m_mouseButtons.end(),
                   [](bool value) { return value; }) ||
            m_mouseDeltaX || m_mouseDeltaY || m_mouseWheel || m_mouseHorizontalWheel;
    }

    bool InputConsumptionMask::HasAnyConsumption() const
    {
        return std::any_of(m_keys.begin(), m_keys.end(), [](bool value) { return value; }) ||
            HasAnyMouseConsumption();
    }

    InputBinding InputBinding::KeyBinding(Key boundKey, f32 boundScale, InputAxisComponent axis)
    {
        InputBinding binding;
        binding.source = InputBindingSource::Key;
        binding.component = axis;
        binding.key = boundKey;
        binding.scale = boundScale;
        return binding;
    }

    InputBinding InputBinding::MouseButtonBinding(
        MouseButton button,
        f32 boundScale,
        InputAxisComponent axis)
    {
        InputBinding binding;
        binding.source = InputBindingSource::MouseButton;
        binding.component = axis;
        binding.mouseButton = button;
        binding.scale = boundScale;
        return binding;
    }

    InputBinding InputBinding::MouseDeltaXBinding(f32 boundScale, InputAxisComponent axis)
    {
        InputBinding binding;
        binding.source = InputBindingSource::MouseDeltaX;
        binding.component = axis;
        binding.scale = boundScale;
        return binding;
    }

    InputBinding InputBinding::MouseDeltaYBinding(f32 boundScale, InputAxisComponent axis)
    {
        InputBinding binding;
        binding.source = InputBindingSource::MouseDeltaY;
        binding.component = axis;
        binding.scale = boundScale;
        return binding;
    }

    InputBinding InputBinding::MouseWheelBinding(f32 boundScale, InputAxisComponent axis)
    {
        InputBinding binding;
        binding.source = InputBindingSource::MouseWheel;
        binding.component = axis;
        binding.scale = boundScale;
        return binding;
    }

    InputBinding InputBinding::MouseHorizontalWheelBinding(
        f32 boundScale,
        InputAxisComponent axis)
    {
        InputBinding binding;
        binding.source = InputBindingSource::MouseHorizontalWheel;
        binding.component = axis;
        binding.scale = boundScale;
        return binding;
    }

    InputBinding& InputBinding::RequireKey(Key modifier)
    {
        requiredKey = modifier;
        return *this;
    }

    InputBinding& InputBinding::RequireMouseButton(MouseButton modifier)
    {
        requiredMouseButton = modifier;
        return *this;
    }

    bool InputBinding::IsValid() const
    {
        if ((component != InputAxisComponent::X && component != InputAxisComponent::Y) ||
            !std::isfinite(scale) || std::fabs(scale) <= kActionEpsilon)
        {
            return false;
        }
        if (requiredKey != Key::Unknown && !IsValidKey(requiredKey))
        {
            return false;
        }
        if (requiredMouseButton != MouseButton::Count &&
            !IsValidMouseButton(requiredMouseButton))
        {
            return false;
        }

        switch (source)
        {
            case InputBindingSource::Key:
                return IsValidKey(key);
            case InputBindingSource::MouseButton:
                return IsValidMouseButton(mouseButton);
            case InputBindingSource::MouseDeltaX:
            case InputBindingSource::MouseDeltaY:
            case InputBindingSource::MouseWheel:
            case InputBindingSource::MouseHorizontalWheel:
                return true;
        }
        return false;
    }

    bool InputBinding::IsDigital() const
    {
        return source == InputBindingSource::Key || source == InputBindingSource::MouseButton;
    }

    InputContext::InputContext(
        std::string contextName,
        i32 priority,
        bool consumeInput,
        u64 insertionOrder)
        : m_name(std::move(contextName))
        , m_priority(priority)
        , m_consumeInput(consumeInput)
        , m_insertionOrder(insertionOrder)
    {
    }

    bool InputContext::AddAction(std::string name, InputActionType type)
    {
        if ((type != InputActionType::Button &&
             type != InputActionType::Axis1D &&
             type != InputActionType::Axis2D) ||
            name.empty() || FindAction(name))
        {
            return false;
        }

        Action action;
        action.name = std::move(name);
        action.state.type = type;
        m_actions.push_back(std::move(action));
        return true;
    }

    bool InputContext::RemoveAction(std::string_view name)
    {
        const auto iterator = std::find_if(
            m_actions.begin(),
            m_actions.end(),
            [name](const Action& action) { return action.name == name; });
        if (iterator == m_actions.end())
        {
            return false;
        }
        m_actions.erase(iterator);
        return true;
    }

    bool InputContext::HasAction(std::string_view name) const
    {
        return FindAction(name) != nullptr;
    }

    bool InputContext::AddBinding(std::string_view actionName, const InputBinding& binding)
    {
        Action* action = FindAction(actionName);
        if (!action || !binding.IsValid())
        {
            return false;
        }
        if ((action->state.type == InputActionType::Button && !binding.IsDigital()) ||
            (action->state.type == InputActionType::Axis1D &&
             binding.component != InputAxisComponent::X))
        {
            return false;
        }

        action->bindings.push_back(binding);
        return true;
    }

    bool InputContext::Rebind(
        std::string_view actionName,
        std::size_t bindingIndex,
        const InputBinding& replacement)
    {
        Action* action = FindAction(actionName);
        if (!action || bindingIndex >= action->bindings.size() || !replacement.IsValid())
        {
            return false;
        }
        if ((action->state.type == InputActionType::Button && !replacement.IsDigital()) ||
            (action->state.type == InputActionType::Axis1D &&
             replacement.component != InputAxisComponent::X))
        {
            return false;
        }

        action->bindings[bindingIndex] = replacement;
        return true;
    }

    bool InputContext::RemoveBinding(std::string_view actionName, std::size_t bindingIndex)
    {
        Action* action = FindAction(actionName);
        if (!action || bindingIndex >= action->bindings.size())
        {
            return false;
        }
        action->bindings.erase(action->bindings.begin() + static_cast<std::ptrdiff_t>(bindingIndex));
        return true;
    }

    bool InputContext::ClearBindings(std::string_view actionName)
    {
        Action* action = FindAction(actionName);
        if (!action)
        {
            return false;
        }
        action->bindings.clear();
        return true;
    }

    std::size_t InputContext::GetBindingCount(std::string_view actionName) const
    {
        const Action* action = FindAction(actionName);
        return action ? action->bindings.size() : 0;
    }

    const InputBinding* InputContext::GetBinding(
        std::string_view actionName,
        std::size_t bindingIndex) const
    {
        const Action* action = FindAction(actionName);
        if (!action || bindingIndex >= action->bindings.size())
        {
            return nullptr;
        }
        return &action->bindings[bindingIndex];
    }

    const InputActionState* InputContext::FindActionState(std::string_view name) const
    {
        const Action* action = FindAction(name);
        return action ? &action->state : nullptr;
    }

    InputContext::Action* InputContext::FindAction(std::string_view name)
    {
        const auto iterator = std::find_if(
            m_actions.begin(),
            m_actions.end(),
            [name](const Action& action) { return action.name == name; });
        return iterator == m_actions.end() ? nullptr : &*iterator;
    }

    const InputContext::Action* InputContext::FindAction(std::string_view name) const
    {
        const auto iterator = std::find_if(
            m_actions.begin(),
            m_actions.end(),
            [name](const Action& action) { return action.name == name; });
        return iterator == m_actions.end() ? nullptr : &*iterator;
    }

    void InputContext::ResetRuntimeState(bool emitRelease)
    {
        for (Action& action : m_actions)
        {
            const bool wasActive = action.previousActive;
            action.state.value = {};
            action.state.active = false;
            action.state.pressed = false;
            action.state.released = emitRelease && wasActive;
            action.previousActive = false;
        }
    }

    InputContext* InputActionSystem::CreateContext(
        std::string name,
        i32 priority,
        bool consumeInput)
    {
        if (name.empty() || FindContext(name))
        {
            return nullptr;
        }

        auto context = std::unique_ptr<InputContext>(
            new InputContext(std::move(name), priority, consumeInput, m_nextInsertionOrder++));
        InputContext* result = context.get();
        m_contexts.push_back(std::move(context));
        return result;
    }

    bool InputActionSystem::RemoveContext(std::string_view name)
    {
        const auto iterator = std::find_if(
            m_contexts.begin(),
            m_contexts.end(),
            [name](const std::unique_ptr<InputContext>& context)
            {
                return context->GetName() == name;
            });
        if (iterator == m_contexts.end())
        {
            return false;
        }
        m_contexts.erase(iterator);
        return true;
    }

    void InputActionSystem::ClearContexts()
    {
        m_contexts.clear();
    }

    InputContext* InputActionSystem::FindContext(std::string_view name)
    {
        const auto iterator = std::find_if(
            m_contexts.begin(),
            m_contexts.end(),
            [name](const std::unique_ptr<InputContext>& context)
            {
                return context->GetName() == name;
            });
        return iterator == m_contexts.end() ? nullptr : iterator->get();
    }

    const InputContext* InputActionSystem::FindContext(std::string_view name) const
    {
        const auto iterator = std::find_if(
            m_contexts.begin(),
            m_contexts.end(),
            [name](const std::unique_ptr<InputContext>& context)
            {
                return context->GetName() == name;
            });
        return iterator == m_contexts.end() ? nullptr : iterator->get();
    }

    void InputActionSystem::Update(const InputState& input)
    {
        Update(input, InputConsumptionMask{});
    }

    void InputActionSystem::Update(
        const InputState& input,
        const InputConsumptionMask& initialConsumption)
    {
        std::vector<InputContext*> orderedContexts;
        orderedContexts.reserve(m_contexts.size());
        for (const auto& context : m_contexts)
        {
            orderedContexts.push_back(context.get());
        }

        std::stable_sort(
            orderedContexts.begin(),
            orderedContexts.end(),
            [](const InputContext* left, const InputContext* right)
            {
                if (left->m_priority != right->m_priority)
                {
                    return left->m_priority > right->m_priority;
                }
                return left->m_insertionOrder < right->m_insertionOrder;
            });

        ConsumedControls consumed = ToConsumedControls(initialConsumption);
        for (InputContext* context : orderedContexts)
        {
            if (!context->m_enabled)
            {
                context->ResetRuntimeState(true);
                continue;
            }

            ConsumedControls contextConsumption;
            for (InputContext::Action& action : context->m_actions)
            {
                f32 digitalPositiveX = 0.0f;
                f32 digitalNegativeX = 0.0f;
                f32 digitalPositiveY = 0.0f;
                f32 digitalNegativeY = 0.0f;
                f32 analogX = 0.0f;
                f32 analogY = 0.0f;
                bool anyDown = false;
                bool anyPressed = false;
                bool anyReleased = false;

                for (const InputBinding& binding : action.bindings)
                {
                    const BindingSample sample = SampleBinding(binding, input, consumed);
                    if (!sample.available)
                    {
                        continue;
                    }

                    anyDown = anyDown || sample.down;
                    anyPressed = anyPressed || sample.pressed;
                    anyReleased = anyReleased || sample.released;

                    if (action.state.type == InputActionType::Button)
                    {
                        // Button value is derived from aggregate down state below.
                    }
                    else if (binding.IsDigital())
                    {
                        f32& positive = binding.component == InputAxisComponent::X
                            ? digitalPositiveX
                            : digitalPositiveY;
                        f32& negative = binding.component == InputAxisComponent::X
                            ? digitalNegativeX
                            : digitalNegativeY;
                        if (sample.value > 0.0f)
                        {
                            positive = (std::max)(positive, sample.value);
                        }
                        else
                        {
                            negative = (std::min)(negative, sample.value);
                        }
                    }
                    else if (binding.component == InputAxisComponent::X)
                    {
                        analogX += sample.value;
                    }
                    else
                    {
                        analogY += sample.value;
                    }

                    if (context->m_consumeInput && sample.HasActivity())
                    {
                        ConsumeBinding(binding, contextConsumption);
                    }
                }

                InputActionState next;
                next.type = action.state.type;
                if (next.type == InputActionType::Button)
                {
                    next.value.x = anyDown ? 1.0f : 0.0f;
                    next.active = anyDown;
                }
                else
                {
                    next.value.x = ClampDigitalAxis(
                        digitalPositiveX + digitalNegativeX) + analogX;
                    next.value.y = next.type == InputActionType::Axis2D
                        ? ClampDigitalAxis(digitalPositiveY + digitalNegativeY) + analogY
                        : 0.0f;
                    next.active = std::fabs(next.value.x) > kActionEpsilon ||
                        std::fabs(next.value.y) > kActionEpsilon;
                }

                next.pressed = !action.previousActive && (next.active || anyPressed);
                next.released = action.previousActive && !next.active;
                if (!action.previousActive && !next.active && anyPressed && anyReleased)
                {
                    next.released = true;
                }

                action.state = next;
                action.previousActive = next.active;
            }

            if (context->m_consumeInput)
            {
                MergeConsumedControls(consumed, contextConsumption);
            }
        }
    }

    const InputActionState* InputActionSystem::FindActionState(
        std::string_view contextName,
        std::string_view actionName) const
    {
        const InputContext* context = FindContext(contextName);
        return context ? context->FindActionState(actionName) : nullptr;
    }

    const InputActionState* InputActionSystem::FindActionState(std::string_view actionName) const
    {
        const InputContext* selectedContext = nullptr;
        for (const auto& context : m_contexts)
        {
            if (!context->IsEnabled() || !context->HasAction(actionName))
            {
                continue;
            }
            if (!selectedContext || context->GetPriority() > selectedContext->GetPriority() ||
                (context->GetPriority() == selectedContext->GetPriority() &&
                 context->m_insertionOrder < selectedContext->m_insertionOrder))
            {
                selectedContext = context.get();
            }
        }
        return selectedContext ? selectedContext->FindActionState(actionName) : nullptr;
    }

    bool InputActionSystem::IsActionDown(
        std::string_view contextName,
        std::string_view actionName) const
    {
        const InputActionState* state = FindActionState(contextName, actionName);
        return state && state->IsDown();
    }

    bool InputActionSystem::WasActionPressed(
        std::string_view contextName,
        std::string_view actionName) const
    {
        const InputActionState* state = FindActionState(contextName, actionName);
        return state && state->WasPressed();
    }

    bool InputActionSystem::WasActionReleased(
        std::string_view contextName,
        std::string_view actionName) const
    {
        const InputActionState* state = FindActionState(contextName, actionName);
        return state && state->WasReleased();
    }

    f32 InputActionSystem::GetActionValue(
        std::string_view contextName,
        std::string_view actionName) const
    {
        const InputActionState* state = FindActionState(contextName, actionName);
        return state ? state->GetValue() : 0.0f;
    }

    InputActionVector2 InputActionSystem::GetActionValue2D(
        std::string_view contextName,
        std::string_view actionName) const
    {
        const InputActionState* state = FindActionState(contextName, actionName);
        return state ? state->GetValue2D() : InputActionVector2{};
    }
} // namespace Pyramid
