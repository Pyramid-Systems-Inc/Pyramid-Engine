#include <Pyramid/Platform/Clipboard.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "Clipboard encoding test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    std::u16string utf16;
    std::string error;
    Require(Pyramid::ClipboardEncoding::Utf32ToUtf16(
        U"A\U0001F603\u03A9", utf16, 16, &error),
        "UTF-32 to UTF-16 conversion failed");
    Require(utf16.size() == 4, "surrogate pair output length mismatch");

    std::u32string utf32;
    Require(Pyramid::ClipboardEncoding::Utf16ToUtf32(
        utf16, utf32, 16, &error),
        "UTF-16 to UTF-32 conversion failed");
    Require(utf32 == U"A\U0001F603\u03A9", "clipboard conversion round trip mismatch");

    const std::u16string malformed{static_cast<char16_t>(0xd800), u'A'};
    Require(!Pyramid::ClipboardEncoding::Utf16ToUtf32(
        malformed, utf32, 16, &error),
        "malformed surrogate pair was accepted");
    Require(!error.empty(), "malformed UTF-16 did not report an error");

    const std::u32string invalid{static_cast<char32_t>(0x110000)};
    Require(!Pyramid::ClipboardEncoding::Utf32ToUtf16(
        invalid, utf16, 16, &error),
        "invalid Unicode scalar was accepted");

    Require(!Pyramid::ClipboardEncoding::Utf32ToUtf16(
        U"AB", utf16, 1, &error),
        "UTF-16 output limit was ignored");
    Require(!Pyramid::ClipboardEncoding::Utf32ToUtf16(
        U"\U0001F603", utf16, 1, &error),
        "UTF-16 surrogate-pair output limit was ignored");
    Require(!Pyramid::ClipboardEncoding::Utf16ToUtf32(
        u"AB", utf32, 1, &error),
        "UTF-32 output limit was ignored");

    std::cout << "Clipboard encoding tests passed\n";
    return EXIT_SUCCESS;
}
