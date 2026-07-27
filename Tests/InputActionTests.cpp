#include <Pyramid/Input/InputActions.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "InputAction test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    bool NearlyEqual(Pyramid::f32 left, Pyramid::f32 right)
    {
        return std::fabs(left - right) <= 0.0001f;
    }
}

int main()
{
    using namespace Pyramid;

    InputState input;
    input.SetFocused(true);
    InputActionSystem actions;

    InputContext* gameplay = actions.CreateContext("gameplay", 0, true);
    Require(gameplay != nullptr, "gameplay context creation failed");
    Require(actions.CreateContext("gameplay") == nullptr, "duplicate context should fail");
    Require(actions.GetContextCount() == 1, "context count mismatch");

    Require(gameplay->AddAction("Move", InputActionType::Axis2D), "Move action creation failed");
    Require(gameplay->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::A, -1.0f, InputAxisComponent::X)),
        "A binding failed");
    Require(gameplay->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::Left, -1.0f, InputAxisComponent::X)),
        "Left binding failed");
    Require(gameplay->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::D, 1.0f, InputAxisComponent::X)),
        "D binding failed");
    Require(gameplay->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::W, 1.0f, InputAxisComponent::Y)),
        "W binding failed");
    Require(gameplay->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::S, -1.0f, InputAxisComponent::Y)),
        "S binding failed");

    Require(gameplay->AddAction("Confirm", InputActionType::Button), "Confirm action creation failed");
    Require(gameplay->AddBinding("Confirm", InputBinding::KeyBinding(Key::Enter)),
        "Confirm binding failed");
    Require(!gameplay->AddBinding("Confirm", InputBinding::MouseWheelBinding()),
        "button action should reject analog bindings");

    Require(gameplay->AddAction("Look", InputActionType::Axis2D), "Look action creation failed");
    auto lookX = InputBinding::MouseDeltaXBinding(0.5f, InputAxisComponent::X);
    lookX.RequireMouseButton(MouseButton::Right);
    auto lookY = InputBinding::MouseDeltaYBinding(-0.25f, InputAxisComponent::Y);
    lookY.RequireMouseButton(MouseButton::Right);
    Require(gameplay->AddBinding("Look", lookX), "Look X binding failed");
    Require(gameplay->AddBinding("Look", lookY), "Look Y binding failed");

    Require(gameplay->AddAction("Zoom", InputActionType::Axis1D), "Zoom action creation failed");
    Require(gameplay->AddBinding("Zoom", InputBinding::MouseWheelBinding(-2.0f)),
        "Zoom binding failed");
    Require(!gameplay->AddBinding(
        "Zoom",
        InputBinding::KeyBinding(Key::Q, 1.0f, InputAxisComponent::Y)),
        "one-dimensional action should reject Y bindings");

    Require(!gameplay->AddAction("", InputActionType::Button), "empty action names should fail");
    Require(!gameplay->AddAction("Move", InputActionType::Axis1D), "duplicate actions should fail");
    Require(!gameplay->AddBinding("Missing", InputBinding::KeyBinding(Key::A)),
        "binding unknown action should fail");
    Require(!InputBinding::KeyBinding(Key::Unknown).IsValid(), "unknown key binding should be invalid");
    Require(!InputBinding::MouseWheelBinding(0.0f).IsValid(), "zero-scale binding should be invalid");

    input.BeginFrame();
    input.ProcessKey(Key::A, true);
    actions.Update(input);
    auto move = actions.GetActionValue2D("gameplay", "Move");
    Require(NearlyEqual(move.x, -1.0f) && NearlyEqual(move.y, 0.0f),
        "negative X movement mismatch");
    Require(actions.WasActionPressed("gameplay", "Move"), "axis should start on first activity");

    input.BeginFrame();
    input.ProcessKey(Key::Left, true);
    actions.Update(input);
    move = actions.GetActionValue2D("gameplay", "Move");
    Require(NearlyEqual(move.x, -1.0f), "alternate keys should not double a digital axis");
    Require(!actions.WasActionPressed("gameplay", "Move"), "held axis should not retrigger");

    input.BeginFrame();
    input.ProcessKey(Key::D, true);
    actions.Update(input);
    move = actions.GetActionValue2D("gameplay", "Move");
    Require(NearlyEqual(move.x, 0.0f), "opposing digital bindings should cancel");
    Require(actions.WasActionReleased("gameplay", "Move"),
        "cancelled axis should complete its active state");

    input.BeginFrame();
    input.ProcessKey(Key::A, false);
    input.ProcessKey(Key::Left, false);
    input.ProcessKey(Key::D, false);
    input.ProcessKey(Key::W, true);
    actions.Update(input);
    move = actions.GetActionValue2D("gameplay", "Move");
    Require(NearlyEqual(move.y, 1.0f), "positive Y movement mismatch");

    input.BeginFrame();
    input.ProcessKey(Key::Enter, true);
    input.ProcessKey(Key::Enter, false);
    actions.Update(input);
    Require(!actions.IsActionDown("gameplay", "Confirm"), "same-frame tap should not remain down");
    Require(actions.WasActionPressed("gameplay", "Confirm"), "same-frame tap press missing");
    Require(actions.WasActionReleased("gameplay", "Confirm"), "same-frame tap release missing");

    input.BeginFrame();
    input.ProcessMouseMove(100.0f, 100.0f);
    input.ProcessMouseMove(106.0f, 92.0f);
    actions.Update(input);
    auto look = actions.GetActionValue2D("gameplay", "Look");
    Require(NearlyEqual(look.x, 0.0f) && NearlyEqual(look.y, 0.0f),
        "mouse chord should require its button");

    input.BeginFrame();
    input.ProcessMouseButton(MouseButton::Right, true);
    input.ProcessMouseMove(110.0f, 100.0f);
    actions.Update(input);
    look = actions.GetActionValue2D("gameplay", "Look");
    Require(NearlyEqual(look.x, 2.0f), "gated mouse X mismatch");
    Require(NearlyEqual(look.y, -2.0f), "gated mouse Y mismatch");

    input.BeginFrame();
    input.ProcessMouseWheel(1.5f);
    actions.Update(input);
    Require(NearlyEqual(actions.GetActionValue("gameplay", "Zoom"), -3.0f),
        "wheel action scaling mismatch");

    InputContext* overlay = actions.CreateContext("overlay", 100, true);
    Require(overlay != nullptr, "overlay context creation failed");
    Require(overlay->AddAction("Accept", InputActionType::Button), "overlay action failed");
    Require(overlay->AddBinding("Accept", InputBinding::KeyBinding(Key::Space)),
        "overlay binding failed");
    Require(gameplay->AddAction("Jump", InputActionType::Button), "gameplay Jump failed");
    Require(gameplay->AddBinding("Jump", InputBinding::KeyBinding(Key::Space)),
        "gameplay Space binding failed");

    input.BeginFrame();
    input.ProcessKey(Key::Space, true);
    actions.Update(input);
    Require(actions.IsActionDown("overlay", "Accept"), "higher context missed consumed key");
    Require(!actions.IsActionDown("gameplay", "Jump"), "consumed key reached lower context");
    Require(actions.FindActionState("Accept") == overlay->FindActionState("Accept"),
        "global lookup did not select highest-priority context");

    overlay->SetEnabled(false);
    input.BeginFrame();
    actions.Update(input);
    Require(actions.WasActionReleased("overlay", "Accept"), "disabled context should release actions");
    Require(actions.IsActionDown("gameplay", "Jump"), "disabled context still consumed input");
    Require(actions.WasActionPressed("gameplay", "Jump"),
        "newly exposed held key should start the lower action");

    InputContext* observer = actions.CreateContext("observer", 200, false);
    Require(observer != nullptr, "observer context creation failed");
    Require(observer->AddAction("ObserveSpace", InputActionType::Button), "observer action failed");
    Require(observer->AddBinding("ObserveSpace", InputBinding::KeyBinding(Key::Space)),
        "observer binding failed");
    input.BeginFrame();
    actions.Update(input);
    Require(actions.IsActionDown("observer", "ObserveSpace"), "non-consuming context missed key");
    Require(actions.IsActionDown("gameplay", "Jump"), "non-consuming context blocked lower key");

    Require(gameplay->AddAction("Rebindable", InputActionType::Axis1D), "rebind action failed");
    Require(gameplay->AddBinding("Rebindable", InputBinding::KeyBinding(Key::Q, -1.0f)),
        "initial rebind binding failed");
    Require(gameplay->Rebind("Rebindable", 0, InputBinding::KeyBinding(Key::E, 1.0f)),
        "runtime rebind failed");
    const InputBinding* rebound = gameplay->GetBinding("Rebindable", 0);
    Require(rebound && rebound->key == Key::E && NearlyEqual(rebound->scale, 1.0f),
        "runtime binding was not replaced");

    input.BeginFrame();
    input.ProcessKey(Key::E, true);
    actions.Update(input);
    Require(NearlyEqual(actions.GetActionValue("gameplay", "Rebindable"), 1.0f),
        "rebound key did not drive action");
    Require(gameplay->RemoveBinding("Rebindable", 0), "binding removal failed");
    Require(gameplay->GetBindingCount("Rebindable") == 0, "binding count after removal mismatch");

    InputState maskedInput;
    maskedInput.SetFocused(true);
    InputActionSystem maskedActions;
    InputContext* maskedContext = maskedActions.CreateContext("masked", 0, true);
    Require(maskedContext != nullptr, "masked context creation failed");
    Require(maskedContext->AddAction("Key", InputActionType::Button),
        "masked key action creation failed");
    Require(maskedContext->AddBinding("Key", InputBinding::KeyBinding(Key::Enter)),
        "masked key binding failed");
    Require(maskedContext->AddAction("Click", InputActionType::Button),
        "masked click action creation failed");
    Require(maskedContext->AddBinding(
        "Click", InputBinding::MouseButtonBinding(MouseButton::Left)),
        "masked click binding failed");
    Require(maskedContext->AddAction("Wheel", InputActionType::Axis1D),
        "masked wheel action creation failed");
    Require(maskedContext->AddBinding("Wheel", InputBinding::MouseWheelBinding()),
        "masked wheel binding failed");

    maskedInput.BeginFrame();
    maskedInput.ProcessKey(Key::Enter, true);
    maskedInput.ProcessMouseButton(MouseButton::Left, true);
    maskedInput.ProcessMouseWheel(2.0f);
    InputConsumptionMask initialConsumption;
    initialConsumption.ConsumeKey(Key::Enter);
    initialConsumption.ConsumeMouseButton(MouseButton::Left);
    initialConsumption.ConsumeMouseWheel();
    maskedActions.Update(maskedInput, initialConsumption);
    Require(!maskedActions.IsActionDown("masked", "Key"),
        "pre-consumed key reached action contexts");
    Require(!maskedActions.IsActionDown("masked", "Click"),
        "pre-consumed mouse button reached action contexts");
    Require(NearlyEqual(maskedActions.GetActionValue("masked", "Wheel"), 0.0f),
        "pre-consumed wheel reached action contexts");

    input.BeginFrame();
    input.SetFocused(false);
    actions.Update(input);
    Require(actions.WasActionReleased("gameplay", "Move"),
        "focus loss should release held action");
    Require(!actions.IsActionDown("gameplay", "Jump"), "focus loss left action active");

    Require(actions.RemoveContext("observer"), "context removal failed");
    Require(!actions.RemoveContext("observer"), "removing missing context should fail");
    Require(actions.GetContextCount() == 2, "context count after removal mismatch");
    actions.ClearContexts();
    Require(actions.GetContextCount() == 0, "context clear failed");

    std::cout << "InputAction tests passed\n";
    return EXIT_SUCCESS;
}
