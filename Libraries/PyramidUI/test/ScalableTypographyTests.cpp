#include <Pyramid/UI/UI.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "Scalable typography test failed: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool Near(float left, float right)
    {
        return std::fabs(left - right) < 0.0001f;
    }
}

int main()
{
    Pyramid::Text::FontAtlas font;
    std::string error;
    if (!Pyramid::Text::LoadFontAtlas(PYRAMID_UI_SCALABLE_FONT, font, &error) ||
        font.rasterMode != Pyramid::Font::RasterMode::SignedDistanceField ||
        !Near(font.pixelHeight, 64.0f) || !Near(font.distanceRange, 10.0f))
    {
        return Fail("scalable processed font metadata mismatch");
    }

    Pyramid::UI::Context ui;
    if (!ui.SetFontAtlas(font))
    {
        return Fail("SDF font atlas was rejected by the UI context");
    }
    Pyramid::UI::Theme theme = ui.GetTheme();
    theme.textScale = 18.0f / font.pixelHeight;
    if (!theme.typography.heading.IsValid() || !theme.typography.body.IsValid() ||
        !theme.typography.label.IsValid() || !theme.typography.button.IsValid() ||
        !theme.typography.input.IsValid() || !theme.typography.caption.IsValid())
    {
        return Fail("default typography roles are invalid");
    }
    ui.SetTheme(theme);

    Pyramid::InputState input;
    input.SetFocused(true);
    const Pyramid::UI::FrameInfo frame{640.0f, 480.0f, 1.0f, 1.0f / 60.0f};
    if (!ui.BeginFrame(frame, input))
    {
        return Fail("UI frame was rejected");
    }
    Pyramid::UI::PanelOptions panel;
    panel.position = {12.0f, 12.0f};
    panel.size = {360.0f, 260.0f};
    if (!ui.BeginPanel("TYPOGRAPHY", panel))
    {
        return Fail("typography panel was rejected");
    }
    ui.Heading("Production heading");
    ui.Label("Readable body text");
    ui.Caption("Supporting caption");
    (void)ui.Button("ACTION");
    ui.EndPanel();
    const Pyramid::UI::DrawList& draw = ui.EndFrame();

    bool sawSdf = false;
    bool sawHeadingWeight = false;
    bool sawButtonWeight = false;
    for (const Pyramid::UI::Vertex& vertex : draw.GetVertices())
    {
        if (Near(vertex.textParameters.x, 1.0f))
        {
            sawSdf = true;
            sawHeadingWeight = sawHeadingWeight ||
                Near(vertex.textParameters.y, theme.typography.heading.weight);
            sawButtonWeight = sawButtonWeight ||
                Near(vertex.textParameters.y, theme.typography.button.weight);
        }
    }
    if (!sawSdf || !sawHeadingWeight || !sawButtonWeight)
    {
        return Fail("typography roles were not encoded into SDF text vertices");
    }

    std::cout << "Scalable typography tests passed\n";
    return EXIT_SUCCESS;
}
