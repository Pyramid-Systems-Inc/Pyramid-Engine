#include <Pyramid/UI/UI.hpp>

#include <cstdlib>
#include <iostream>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "UI test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    void BuildPanel(Pyramid::UI::Context& ui, bool& checked, float& slider)
    {
        Pyramid::UI::PanelOptions panel;
        panel.position = Pyramid::Math::Vec2(10.0f, 10.0f);
        panel.size = Pyramid::Math::Vec2(240.0f, 220.0f);
        Require(ui.BeginPanel("DEBUG", panel), "panel creation failed");
        ui.LabelValue("FPS", "144");
        Require(!ui.Button("ACTION"), "idle button activated");
        (void)ui.Checkbox("PAUSED", checked);
        (void)ui.SliderFloat("SPEED", slider, 0.0f, 10.0f);
        ui.ProgressBar("FRAME", 0.5f);
        ui.EndPanel();
    }
}

int main()
{
    Pyramid::UI::Context ui;
    Pyramid::InputState input;
    input.SetFocused(true);
    input.ProcessMouseMove(30.0f, 82.0f);

    const Pyramid::UI::FrameInfo frame{640.0f, 480.0f, 1.0f, 1.0f / 60.0f};
    bool checked = false;
    float slider = 5.0f;
    Require(ui.BeginFrame(frame, input), "first frame rejected");
    BuildPanel(ui, checked, slider);
    const auto& firstDraw = ui.EndFrame();
    Require(!firstDraw.Empty(), "first frame emitted no geometry");
    Require(firstDraw.GetVertices().size() % 4 == 0, "quad vertex count mismatch");
    Require(firstDraw.GetIndices().size() % 6 == 0, "quad index count mismatch");
    Require(!firstDraw.GetBatches().empty(), "draw batching failed");

    const Pyramid::InputConsumptionMask capture = ui.PrepareInput(input);
    Require(capture.IsMouseButtonConsumed(Pyramid::MouseButton::Left),
        "panel did not reserve pointer button");
    Require(capture.IsMouseWheelConsumed(), "panel did not reserve wheel input");

    input.BeginFrame();
    input.ProcessMouseMove(30.0f, 82.0f);
    input.ProcessMouseButton(Pyramid::MouseButton::Left, true);
    Require(ui.BeginFrame(frame, input), "interaction frame rejected");
    Pyramid::UI::PanelOptions panel;
    panel.position = Pyramid::Math::Vec2(10.0f, 10.0f);
    panel.size = Pyramid::Math::Vec2(240.0f, 220.0f);
    Require(ui.BeginPanel("DEBUG", panel), "interaction panel failed");
    ui.LabelValue("FPS", "144");
    const bool clicked = ui.Button("ACTION");
    (void)ui.Checkbox("PAUSED", checked);
    (void)ui.SliderFloat("SPEED", slider, 0.0f, 10.0f);
    ui.ProgressBar("FRAME", 0.5f);
    ui.EndPanel();
    (void)ui.EndFrame();
    Require(clicked, "button click was not detected");
    Require(ui.GetFocusedWidget() != 0, "clicked widget did not receive focus");

    input.BeginFrame();
    input.SetFocused(false);
    input.SetFocused(true);
    const Pyramid::InputConsumptionMask keyboardCapture = ui.PrepareInput(input);
    Require(!input.HasMousePosition(), "focus restore retained stale pointer position");
    Require(keyboardCapture.IsKeyConsumed(Pyramid::Key::Enter),
        "focused UI did not reserve activation key");
    Require(keyboardCapture.IsKeyConsumed(Pyramid::Key::Tab),
        "focused UI did not reserve navigation key");

    const Pyramid::UI::WidgetId previousFocus = ui.GetFocusedWidget();
    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::Tab, true);
    Require(ui.BeginFrame(frame, input), "keyboard frame rejected");
    BuildPanel(ui, checked, slider);
    (void)ui.EndFrame();
    Require(ui.GetFocusedWidget() != 0 && ui.GetFocusedWidget() != previousFocus,
        "Tab did not advance retained keyboard focus");

    ui.SetEnabled(false);
    Require(ui.GetFocusedWidget() == 0, "disabled context retained focus");
    Require(!ui.PrepareInput(input).HasAnyConsumption(),
        "disabled context consumed input");
    Require(!ui.BeginFrame(frame, input), "disabled context began a frame");

    Pyramid::UI::DrawList list;
    list.AddQuad(
        {0.0f, 0.0f, 10.0f, 10.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        Pyramid::Color::White,
        1,
        {0.0f, 0.0f, 100.0f, 100.0f});
    list.AddQuad(
        {10.0f, 0.0f, 10.0f, 10.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        Pyramid::Color::White,
        1,
        {0.0f, 0.0f, 100.0f, 100.0f});
    Require(list.GetBatches().size() == 1, "compatible quads were not batched");
    Require(list.GetBatches()[0].indexCount == 12, "batched index count mismatch");

    std::cout << "UI tests passed\n";
    return EXIT_SUCCESS;
}
