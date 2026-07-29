#include <Pyramid/UI/UI.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "UI text editing test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

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

    Pyramid::UI::TextEditResult BuildField(
        Pyramid::UI::Context& ui,
        std::string& value)
    {
        Pyramid::UI::PanelOptions panel;
        panel.position = Pyramid::Math::Vec2(10.0f, 10.0f);
        panel.size = Pyramid::Math::Vec2(360.0f, 240.0f);
        Require(ui.BeginPanel("FORM", panel), "form panel failed");
        Pyramid::UI::TextFieldOptions options;
        options.placeholder = "Name";
        options.maximumCharacters = 32;
        const auto result = ui.TextField("PLAYER", value, options);
        ui.EndPanel();
        return result;
    }
}

int main()
{
    Pyramid::UI::Context ui;
    Pyramid::Text::FontAtlas latinFont;
    Pyramid::Text::FontAtlas arabicFont;
    Pyramid::Text::FontFamily fontFamily;
    std::string fontError;
    Require(Pyramid::Text::LoadFontAtlas(PYRAMID_UI_TEST_FONT, latinFont, &fontError),
        "Latin UI test font did not load");
    Require(Pyramid::Text::LoadFontAtlas(
            PYRAMID_UI_TEST_ARABIC_FONT, arabicFont, &fontError),
        "Arabic UI test font did not load");
    Require(Pyramid::Text::BuildFontFamily({latinFont, arabicFont}, fontFamily, &fontError),
        "UI fallback font family did not build");
    Require(ui.SetFontFamily(fontFamily), "UI fallback font family was rejected");
    Require(ui.GetFontFamily().ResolveFontIndex(0xFEE1) == 1,
        "UI context did not retain the Arabic fallback source");
    MemoryClipboard clipboard;
    ui.SetClipboard(&clipboard);
    Pyramid::InputState input;
    input.SetFocused(true);
    const Pyramid::UI::FrameInfo frame{640.0f, 480.0f, 1.0f, 1.0f / 60.0f};
    std::string value = "Alpha";

    Require(ui.BeginFrame(frame, input), "initial frame failed");
    (void)BuildField(ui, value);
    (void)ui.EndFrame();

    input.BeginFrame();
    input.ProcessMouseMove(45.0f, 78.0f);
    input.ProcessMouseButton(Pyramid::MouseButton::Left, true);
    Require(ui.BeginFrame(frame, input), "focus frame failed");
    auto result = BuildField(ui, value);
    (void)ui.EndFrame();
    Require(result.focused, "text field did not receive pointer focus");

    const auto capture = ui.PrepareInput(input);
    Require(capture.IsKeyConsumed(Pyramid::Key::W),
        "focused editor did not reserve gameplay keys");
    Require(capture.IsTextInputConsumed(),
        "focused editor did not reserve committed text");
    Require(!capture.IsKeyConsumed(Pyramid::Key::F1),
        "focused editor consumed an unrelated global function key");

    input.BeginFrame();
    input.ProcessMouseButton(Pyramid::MouseButton::Left, false);
    input.ProcessTextCodepoint(U'\u00E9');
    Require(ui.BeginFrame(frame, input), "typing frame failed");
    result = BuildField(ui, value);
    (void)ui.EndFrame();
    Require(result.changed && value.find("\xC3\xA9") != std::string::npos,
        "Unicode character input did not update the bound value");

    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::LeftControl, true);
    input.ProcessTextCodepoint(U'\u03A9');
    Require(ui.BeginFrame(frame, input), "AltGr-style text frame failed");
    result = BuildField(ui, value);
    (void)ui.EndFrame();
    Require(result.changed && value.find("\xCE\xA9") != std::string::npos,
        "committed Unicode text was suppressed while Control was held");

    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::LeftControl, false);
    input.ProcessTextCodepoint(U'\u0645');
    input.ProcessTextCodepoint(U'\u0644');
    input.ProcessTextCodepoint(U'\u0643');
    Require(ui.BeginFrame(frame, input), "Arabic typing frame failed");
    result = BuildField(ui, value);
    const auto& arabicDrawList = ui.EndFrame();
    Require(result.changed && value.find("\xD9\x85\xD9\x84\xD9\x83") != std::string::npos,
        "Arabic committed text did not update the bound value");
    Require(!arabicDrawList.Empty(), "Arabic editor frame produced no draw data");

    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::LeftControl, true);
    input.ProcessKey(Pyramid::Key::A, true);
    Require(ui.BeginFrame(frame, input), "select-all frame failed");
    (void)BuildField(ui, value);
    (void)ui.EndFrame();

    clipboard.value = U"Omega \u03A9";
    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::A, false);
    input.ProcessKey(Pyramid::Key::V, true);
    Require(ui.BeginFrame(frame, input), "paste frame failed");
    result = BuildField(ui, value);
    (void)ui.EndFrame();
    Require(result.changed && value == "Omega \xCE\xA9",
        "clipboard paste did not replace the selection");

    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::V, false);
    input.ProcessKey(Pyramid::Key::LeftControl, false);
    input.ProcessTextCodepoint(U'X');
    Require(ui.BeginFrame(frame, input), "edit-before-cancel frame failed");
    (void)BuildField(ui, value);
    (void)ui.EndFrame();
    Require(value != "Omega \xCE\xA9", "pre-cancel edit did not occur");

    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::Escape, true);
    Require(ui.BeginFrame(frame, input), "cancel frame failed");
    result = BuildField(ui, value);
    (void)ui.EndFrame();
    Require(result.cancelled && value == "Alpha",
        "Escape did not restore the focus snapshot");

    input.BeginFrame();
    input.ProcessKey(Pyramid::Key::Escape, false);
    input.ProcessMouseMove(45.0f, 78.0f);
    input.ProcessMouseButton(Pyramid::MouseButton::Left, true);
    Require(ui.BeginFrame(frame, input), "refocus frame failed");
    result = BuildField(ui, value);
    (void)ui.EndFrame();
    Require(result.focused, "text field did not refocus");

    input.BeginFrame();
    input.ProcessMouseButton(Pyramid::MouseButton::Left, false);
    Require(ui.BeginFrame(frame, input), "pointer release frame failed");
    (void)BuildField(ui, value);
    (void)ui.EndFrame();

    input.BeginFrame();
    input.ProcessMouseMove(600.0f, 430.0f);
    input.ProcessMouseButton(Pyramid::MouseButton::Left, true);
    const auto outsideCapture = ui.PrepareInput(input);
    Require(!outsideCapture.IsTextInputConsumed() &&
            !outsideCapture.IsKeyConsumed(Pyramid::Key::W),
        "clicking outside did not release text-input focus");
    Require(ui.BeginFrame(frame, input), "outside click frame failed");
    result = BuildField(ui, value);
    (void)ui.EndFrame();
    Require(!result.focused, "text field remained focused after an outside click");

    Pyramid::UI::Context multilineUI;
    multilineUI.SetClipboard(&clipboard);
    Pyramid::InputState multilineInput;
    multilineInput.SetFocused(true);
    std::string notes;
    Require(multilineUI.BeginFrame(frame, multilineInput), "multiline initial frame failed");
    Pyramid::UI::PanelOptions panel;
    panel.position = Pyramid::Math::Vec2(10.0f, 10.0f);
    panel.size = Pyramid::Math::Vec2(400.0f, 300.0f);
    Require(multilineUI.BeginPanel("NOTES FORM", panel), "multiline panel failed");
    Pyramid::UI::TextAreaOptions area;
    area.height = 150.0f;
    (void)multilineUI.MultilineTextArea("NOTES", notes, area);
    multilineUI.EndPanel();
    (void)multilineUI.EndFrame();

    multilineInput.BeginFrame();
    multilineInput.ProcessMouseMove(40.0f, 78.0f);
    multilineInput.ProcessMouseButton(Pyramid::MouseButton::Left, true);
    Require(multilineUI.BeginFrame(frame, multilineInput), "multiline focus frame failed");
    Require(multilineUI.BeginPanel("NOTES FORM", panel), "multiline focus panel failed");
    (void)multilineUI.MultilineTextArea("NOTES", notes, area);
    multilineUI.EndPanel();
    (void)multilineUI.EndFrame();

    multilineInput.BeginFrame();
    multilineInput.ProcessMouseButton(Pyramid::MouseButton::Left, false);
    multilineInput.ProcessTextCodepoint(U'A');
    multilineInput.ProcessKey(Pyramid::Key::Enter, true);
    Require(multilineUI.BeginFrame(frame, multilineInput), "multiline typing frame failed");
    Require(multilineUI.BeginPanel("NOTES FORM", panel), "multiline typing panel failed");
    const auto multilineResult = multilineUI.MultilineTextArea("NOTES", notes, area);
    multilineUI.EndPanel();
    (void)multilineUI.EndFrame();
    Require(multilineResult.changed && notes == "A\n",
        "multiline Enter did not insert a line break");

    Pyramid::UI::Context limitedUI;
    Pyramid::InputState limitedInput;
    limitedInput.SetFocused(true);
    std::string limitedValue = "abcdef\nrest";
    Require(limitedUI.BeginFrame(frame, limitedInput), "limited field frame failed");
    Require(limitedUI.BeginPanel("LIMIT FORM", panel), "limited field panel failed");
    Pyramid::UI::TextFieldOptions limitedOptions;
    limitedOptions.maximumCharacters = 4;
    const auto limitedResult = limitedUI.TextField(
        "LIMITED", limitedValue, limitedOptions);
    limitedUI.EndPanel();
    (void)limitedUI.EndFrame();
    Require(limitedResult.changed && limitedValue == "abcd",
        "field configuration did not synchronize normalized text");

    std::cout << "UI text editing tests passed\n";
    return EXIT_SUCCESS;
}
