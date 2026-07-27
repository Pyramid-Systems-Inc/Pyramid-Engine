#include <Pyramid/Platform/Input.hpp>
#include <Pyramid/UI/UI.hpp>

#include <cstdlib>
#include <iostream>

int main()
{
    Pyramid::InputState input;
    input.SetFocused(true);
    input.BeginFrame();

    Pyramid::UI::Context ui;
    const Pyramid::UI::FrameInfo frame{320.0f, 180.0f, 1.0f, 1.0f / 60.0f};
    if (!ui.BeginFrame(frame, input))
    {
        return EXIT_FAILURE;
    }

    Pyramid::UI::PanelOptions panel;
    panel.position = Pyramid::Math::Vec2(8.0f, 8.0f);
    panel.size = Pyramid::Math::Vec2(220.0f, 120.0f);
    if (!ui.BeginPanel("CONSUMER", panel))
    {
        return EXIT_FAILURE;
    }
    ui.Label("PYRAMID UI PACKAGE");
    ui.ProgressBar("READY", 1.0f);
    ui.EndPanel();
    const auto& drawList = ui.EndFrame();
    if (drawList.Empty())
    {
        return EXIT_FAILURE;
    }

    std::cout << "Pyramid::UI package is linked and operational\n";
    return EXIT_SUCCESS;
}
