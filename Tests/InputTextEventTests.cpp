#include <Pyramid/Platform/Input.hpp>

#include <cstdlib>
#include <iostream>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "Input text event test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    Pyramid::InputState input;
    input.SetFocused(true);
    input.BeginFrame();
    input.ProcessTextCodepoint(U'A');
    Require(input.GetTextInputEventCount() == 1, "committed codepoint was not recorded");
    Require(input.GetTextInputEvents()[0].text == U"A", "committed text mismatch");

    input.ProcessUtf16CodeUnit(static_cast<char16_t>(0xd83d));
    Require(input.GetTextInputEventCount() == 1, "high surrogate emitted prematurely");
    input.ProcessUtf16CodeUnit(static_cast<char16_t>(0xde03));
    Require(input.GetTextInputEventCount() == 2, "surrogate pair was not emitted");
    Require(input.GetTextInputEvents()[1].text[0] == U'\U0001F603',
        "surrogate pair decoded to the wrong codepoint");

    input.ProcessUtf16CodeUnit(static_cast<char16_t>(0xdc00));
    Require(input.GetTextInputEvents().back().text[0] == U'\uFFFD',
        "unmatched low surrogate did not emit replacement text");

    Pyramid::TextInputEvent composition;
    composition.type = Pyramid::TextInputEventType::CompositionUpdate;
    composition.text = U"candidate";
    input.ProcessTextEvent(composition);
    Require(input.GetTextInputEvents().back().type ==
            Pyramid::TextInputEventType::CompositionUpdate,
        "composition event type was not preserved");

    Pyramid::TextInputEvent compositionStart;
    compositionStart.type = Pyramid::TextInputEventType::CompositionStart;
    input.ProcessTextEvent(compositionStart);
    Require(input.GetTextInputEvents().back().type ==
            Pyramid::TextInputEventType::CompositionStart &&
            input.GetTextInputEvents().back().text.empty(),
        "empty composition lifecycle event was discarded");

    input.BeginFrame();
    Require(input.GetTextInputEvents().empty(), "text events did not clear at frame start");

    input.ProcessUtf16CodeUnit(static_cast<char16_t>(0xd83d));
    input.SetFocused(false);
    input.SetFocused(true);
    input.ProcessUtf16CodeUnit(u'B');
    Require(input.GetTextInputEvents().size() == 1 &&
            input.GetTextInputEvents()[0].text == U"B",
        "focus loss did not clear pending surrogate state");

    input.SetFocused(false);
    input.ProcessTextCodepoint(U'C');
    Require(input.GetTextInputEvents().empty(), "unfocused input accepted text");

    std::cout << "Input text event tests passed\n";
    return EXIT_SUCCESS;
}
