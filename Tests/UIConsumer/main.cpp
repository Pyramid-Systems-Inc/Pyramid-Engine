#include <Pyramid/Platform/Clipboard.hpp>
#include <Pyramid/Platform/Input.hpp>
#include <Pyramid/Text/Text.hpp>
#include <Pyramid/UI/UI.hpp>
#include <Pyramid/UI/GameUI.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    class MemoryClipboard final : public Pyramid::Clipboard
    {
    public:
        bool SetText(std::u32string_view text, std::string* error) override
        {
            if (error)
            {
                error->clear();
            }
            value.assign(text);
            return true;
        }

        Pyramid::ClipboardTextResult GetText() const override
        {
            Pyramid::ClipboardTextResult result;
            result.success = true;
            result.text = value;
            return result;
        }

        std::u32string value;
    };
}

int main()
{
    Pyramid::InputState input;
    input.SetFocused(true);
    input.BeginFrame();

    Pyramid::UI::Context ui;
    MemoryClipboard clipboard;
    ui.SetClipboard(&clipboard);
    Pyramid::UI::ScreenStack screens;
    Pyramid::UI::Rect available{0.0f, 0.0f, 320.0f, 260.0f};
    const auto docked = Pyramid::UI::ResolveDockedRect(
        available,
        Pyramid::UI::Dock::Top,
        32.0f);
    if (!docked.IsValid())
    {
        return EXIT_FAILURE;
    }
    const Pyramid::UI::FrameInfo frame{320.0f, 260.0f, 1.0f, 1.0f / 60.0f};
    if (!ui.BeginFrame(frame, input))
    {
        return EXIT_FAILURE;
    }

    Pyramid::UI::PanelOptions panel;
    panel.position = Pyramid::Math::Vec2(8.0f, 8.0f);
    panel.size = Pyramid::Math::Vec2(220.0f, 200.0f);
    if (!ui.BeginPanel("CONSUMER", panel))
    {
        return EXIT_FAILURE;
    }
    bool detailsOpen = true;
    (void)ui.CollapsingHeader("DETAILS", detailsOpen);
    if (detailsOpen)
    {
        ui.WrappedLabel("PYRAMID UI PACKAGE WITH OWNED UTF-8 LAYOUT");
        std::string profileName = "Pyramid user";
        Pyramid::UI::TextFieldOptions field;
        field.maximumCharacters = 64;
        (void)ui.TextField("PROFILE", profileName, field);
        Pyramid::UI::ScrollAreaOptions scroll;
        scroll.height = 42.0f;
        if (!ui.BeginScrollArea("STATUS", scroll))
        {
            return EXIT_FAILURE;
        }
        ui.ProgressBar("READY", 1.0f);
        ui.EndScrollArea();
    }
    ui.EndPanel();
    const auto& drawList = ui.EndFrame();
    const auto textLayout = Pyramid::Text::Layout(
        ui.GetDebugFont(),
        "Pyramid UTF-8: \xE2\x9C\x93",
        Pyramid::Math::Vec2::Zero);
    if (drawList.Empty() || textLayout.glyphs.empty() ||
        textLayout.invalidUtf8Sequences != 0)
    {
        return EXIT_FAILURE;
    }

    std::cout << "Pyramid::UI package is linked and operational\n";
    return EXIT_SUCCESS;
}
