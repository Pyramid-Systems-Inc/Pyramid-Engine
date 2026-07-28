#include <Pyramid/Text/Text.hpp>

#include <cstdlib>
#include <iostream>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "TextBuffer test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    const auto decoded = Pyramid::Text::DecodeUtf8("A\xC3\xA9\xF0\x9F\x98\x83");
    Require(decoded.invalidSequences == 0, "valid UTF-8 was rejected");
    Require(decoded.text == U"A\u00E9\U0001F603", "UTF-8 decode mismatch");
    Require(Pyramid::Text::EncodeUtf8(decoded.text) ==
            "A\xC3\xA9\xF0\x9F\x98\x83",
        "UTF-8 encode mismatch");

    Pyramid::Text::TextBuffer buffer(U"alpha beta\nsecond");
    buffer.SetCursor(5);
    Require(buffer.MoveCursor(Pyramid::Text::CursorMove::NextWord),
        "next-word movement failed");
    Require(buffer.GetCursor() == 6, "next-word cursor mismatch");
    Require(buffer.MoveCursor(Pyramid::Text::CursorMove::NextWord),
        "second next-word movement failed");
    Require(buffer.GetCursor() == 11, "word movement did not reach next line");

    buffer.SetCursor(8);
    Require(buffer.SelectWordAt(8), "word selection failed");
    Require(buffer.GetSelectedText() == U"beta", "word selection mismatch");
    Require(buffer.Insert(U"kingdom"), "selection replacement failed");
    Require(buffer.GetText() == U"alpha kingdom\nsecond", "replacement text mismatch");

    buffer.SetCursor(buffer.Size());
    Require(buffer.MoveCursor(Pyramid::Text::CursorMove::LineUp),
        "line-up movement failed");
    Require(buffer.GetCursor() < buffer.GetText().find(U'\n'),
        "line-up cursor did not move to the previous line");

    buffer.SetSelection({0, 5});
    Require(buffer.Backspace(), "selection backspace failed");
    Require(buffer.GetText().rfind(U" kingdom", 0) == 0,
        "selection deletion mismatch");

    buffer.SetMaximumCharacters(buffer.Size() + 2);
    buffer.SetCursor(buffer.Size());
    Require(buffer.Insert(U"XYZ"), "bounded insert failed");
    Require(buffer.GetText().substr(buffer.Size() - 2) == U"XY",
        "maximum character limit was not enforced");

    buffer.SetSingleLine(true);
    Require(buffer.GetText().find(U'\n') == std::u32string::npos,
        "single-line normalization retained a newline");

    buffer.SetReadOnly(true);
    const std::u32string before = buffer.GetText();
    Require(!buffer.Insert(U"blocked") && !buffer.DeleteForward(),
        "read-only buffer accepted a mutation");
    Require(buffer.GetText() == before, "read-only text changed");

    buffer.SetReadOnly(false);
    buffer.SelectAll();
    Require(buffer.DeleteSelection() && buffer.Empty(), "select-all deletion failed");

    std::cout << "TextBuffer tests passed\n";
    return EXIT_SUCCESS;
}
