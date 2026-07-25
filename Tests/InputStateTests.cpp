#include <Pyramid/Platform/Input.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "InputState test failed: " << message << '\n';
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
    Require(!input.HasFocus(), "new input state should not assume focus");
    Require(!input.IsKeyDown(Key::W), "keys should start released");

    input.SetFocused(true);
    Require(input.HasFocus(), "focus gain should be recorded");

    input.BeginFrame();
    input.ProcessKey(Key::W, true);
    Require(input.IsKeyDown(Key::W), "key-down state was not recorded");
    Require(input.WasKeyPressed(Key::W), "initial key-down should press");
    Require(!input.WasKeyReleased(Key::W), "key-down should not release");

    input.ProcessKey(Key::W, true);
    Require(input.WasKeyPressed(Key::W), "repeat should preserve initial press");

    input.BeginFrame();
    Require(input.IsKeyDown(Key::W), "held key should survive BeginFrame");
    Require(!input.WasKeyPressed(Key::W), "press should last one frame");
    input.ProcessKey(Key::W, false);
    Require(!input.IsKeyDown(Key::W), "key-up should clear held state");
    Require(input.WasKeyReleased(Key::W), "key-up should release");

    input.BeginFrame();
    input.ProcessKey(Key::E, true);
    input.ProcessKey(Key::E, false);
    Require(!input.IsKeyDown(Key::E), "same-frame release should clear held state");
    Require(input.WasKeyPressed(Key::E), "same-frame tap should report press");
    Require(input.WasKeyReleased(Key::E), "same-frame tap should report release");

    input.BeginFrame();
    input.ProcessKey(Key::Unknown, true);
    input.ProcessKey(Key::Count, true);
    Require(!input.IsKeyDown(Key::Unknown), "unknown keys must be ignored");

    input.ProcessMouseMove(10.0f, 20.0f);
    Require(NearlyEqual(input.GetMouseDelta().x, 0.0f), "first mouse sample should not jump");
    input.ProcessMouseMove(14.0f, 18.0f);
    input.ProcessMouseMove(17.0f, 25.0f);
    Require(NearlyEqual(input.GetMouseDelta().x, 7.0f), "mouse X delta should aggregate");
    Require(NearlyEqual(input.GetMouseDelta().y, 5.0f), "mouse Y delta should aggregate");
    Require(NearlyEqual(input.GetMousePosition().x, 17.0f), "mouse X position mismatch");
    Require(NearlyEqual(input.GetMousePosition().y, 25.0f), "mouse Y position mismatch");

    input.ProcessMouseWheel(1.0f);
    input.ProcessMouseWheel(-0.25f, 2.0f);
    Require(NearlyEqual(input.GetMouseWheelDelta(), 0.75f), "vertical wheel should aggregate");
    Require(NearlyEqual(input.GetMouseHorizontalWheelDelta(), 2.0f), "horizontal wheel should aggregate");

    input.ProcessMouseButton(MouseButton::Right, true);
    Require(input.IsMouseButtonDown(MouseButton::Right), "mouse button should be held");
    Require(input.WasMouseButtonPressed(MouseButton::Right), "mouse button press missing");

    input.BeginFrame();
    Require(input.IsMouseButtonDown(MouseButton::Right), "held mouse button should survive BeginFrame");
    Require(NearlyEqual(input.GetMouseDelta().x, 0.0f), "BeginFrame should clear mouse delta");
    Require(NearlyEqual(input.GetMouseWheelDelta(), 0.0f), "BeginFrame should clear wheel delta");

    input.ReleaseMouseButtons();
    Require(!input.IsMouseButtonDown(MouseButton::Right), "capture loss should release buttons");
    Require(input.WasMouseButtonReleased(MouseButton::Right), "capture loss should report release");

    input.BeginFrame();
    input.ProcessKey(Key::A, true);
    input.ProcessMouseButton(MouseButton::Left, true);
    input.SetFocused(false);
    Require(!input.HasFocus(), "focus loss should be recorded");
    Require(!input.IsKeyDown(Key::A), "focus loss should clear held keys");
    Require(input.WasKeyReleased(Key::A), "focus loss should release keys");
    Require(!input.IsMouseButtonDown(MouseButton::Left), "focus loss should clear mouse buttons");
    Require(input.WasMouseButtonReleased(MouseButton::Left), "focus loss should release mouse buttons");

    input.BeginFrame();
    Require(!input.WasKeyReleased(Key::A), "release transition should last one frame");
    Require(!input.WasMouseButtonReleased(MouseButton::Left), "mouse release should last one frame");

    input.SetFocused(true);
    input.ProcessMouseMove(200.0f, 100.0f);
    Require(NearlyEqual(input.GetMouseDelta().x, 0.0f), "focus regain should reset pointer baseline");

    input.Reset(false);
    Require(!input.IsKeyDown(Key::A), "hard reset should clear keys");
    Require(!input.WasKeyReleased(Key::A), "hard reset should not emit release events");

    std::cout << "InputState tests passed\n";
    return EXIT_SUCCESS;
}
