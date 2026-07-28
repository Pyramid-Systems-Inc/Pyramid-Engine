#include <Pyramid/Text/Text.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "Text test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }
}

int main()
{
    const Pyramid::Text::FontAtlas font = Pyramid::Text::CreateDebugFontAtlas();
    Require(font.IsValid(), "embedded atlas is invalid");
    Require(font.width == 128 && font.height == 80, "atlas dimensions mismatch");
    Require(font.rgbaPixels[3] == 255, "white atlas texel is missing");

    const auto metrics = Pyramid::Text::Measure(font, "FPS 144\nFRAME 6.9MS", 1.0f);
    Require(metrics.lineCount == 2, "multiline count mismatch");
    Require(metrics.width > 30.0f, "measured width is implausible");
    Require(std::fabs(metrics.height - 20.0f) < 0.001f, "measured height mismatch");

    std::vector<Pyramid::Text::GlyphQuad> quads;
    Pyramid::Text::BuildGlyphQuads(
        font,
        "Aa?",
        Pyramid::Math::Vec2(10.0f, 20.0f),
        2.0f,
        quads);
    Require(quads.size() == 3, "glyph quad count mismatch");
    Require(quads[0].maximum.x > quads[0].minimum.x, "glyph extent is invalid");
    Require(
        std::fabs(quads[1].uvMinimum.x - font.GetGlyph('a').u0) < 0.0001f &&
            std::fabs(quads[1].uvMinimum.y - font.GetGlyph('a').v0) < 0.0001f,
        "lowercase atlas lookup mismatch");
    Require(font.GetGlyph(0x400).codepoint == U'?', "fallback glyph mismatch");

    Pyramid::Text::LayoutOptions wrappedOptions;
    wrappedOptions.maximumWidth = 24.0f;
    wrappedOptions.wrap = Pyramid::Text::WrapMode::Word;
    wrappedOptions.lineSpacing = 2.0f;
    const auto wrapped = Pyramid::Text::Layout(
        font,
        "ONE TWO THREE",
        Pyramid::Math::Vec2(4.0f, 8.0f),
        wrappedOptions);
    Require(wrapped.metrics.lineCount >= 3, "word wrapping did not create multiple lines");
    Require(wrapped.metrics.width <= 24.001f, "wrapped line exceeded the requested width");
    Require(wrapped.lines.size() == wrapped.metrics.lineCount,
        "line metrics count mismatch");

    Pyramid::Text::LayoutOptions centeredOptions;
    centeredOptions.maximumWidth = 60.0f;
    centeredOptions.alignment = Pyramid::Text::HorizontalAlignment::Center;
    const auto centered = Pyramid::Text::Layout(
        font,
        "AB",
        Pyramid::Math::Vec2(10.0f, 5.0f),
        centeredOptions);
    Require(centered.glyphs.size() == 2, "centered layout glyph count mismatch");
    Require(centered.glyphs.front().minimum.x > 10.0f,
        "center alignment did not offset the glyph run");

    const std::string validUtf8 = std::string("A") + "\xC3\xA9" + "B";
    const auto validUnicode = Pyramid::Text::Layout(
        font,
        validUtf8,
        Pyramid::Math::Vec2::Zero);
    Require(validUnicode.invalidUtf8Sequences == 0,
        "valid UTF-8 was reported as malformed");
    Require(validUnicode.glyphs.size() == 3,
        "unsupported Unicode must still emit a replacement glyph");

    const std::string malformedUtf8("A\xC0\xAF" "B", 4);
    const auto malformed = Pyramid::Text::Layout(
        font,
        malformedUtf8,
        Pyramid::Math::Vec2::Zero);
    Require(malformed.invalidUtf8Sequences == 1,
        "malformed UTF-8 sequence count mismatch");
    Require(malformed.glyphs.size() == 3,
        "malformed UTF-8 did not emit one replacement glyph");

    Pyramid::Text::LayoutOptions tabOptions;
    tabOptions.tabWidth = 3;
    const auto tabbed = Pyramid::Text::Layout(
        font,
        "A\tB",
        Pyramid::Math::Vec2::Zero,
        tabOptions);
    Require(tabbed.metrics.width > Pyramid::Text::Measure(font, "AB", 1.0f).width,
        "tab expansion did not affect text width");


    Pyramid::Text::FontAtlas processed;
    std::string processedError;
    Require(
        Pyramid::Text::LoadFontAtlas(PYRAMID_TEXT_TEST_FONT, processed, &processedError),
        "processed font atlas failed to load");
    Require(processed.IsValid(), "processed atlas is invalid");
    Require(processed.familyName == "Pyramid Sans", "processed family name mismatch");
    Require(processed.glyphs.size() == 98, "processed glyph count mismatch");
    Require(processed.HasGlyph(U'\u00E9') && processed.HasGlyph(U'\u03A9') &&
            processed.HasGlyph(U'\u2713'),
        "processed Unicode coverage mismatch");
    const float unkerned = processed.GetGlyph(U'A').advance + processed.GetGlyph(U'V').advance;
    const auto kernedMetrics = Pyramid::Text::Measure(processed, "AV", 1.0f);
    Require(kernedMetrics.width < unkerned, "processed kerning was not applied");
    const auto coveredUnicode = Pyramid::Text::Layout(
        processed,
        std::string("\xC3\xA9 ") + "\xCE\xA9 " + "\xE2\x9C\x93",
        Pyramid::Math::Vec2::Zero);
    Require(coveredUnicode.glyphs.size() == 3 && coveredUnicode.fallbackGlyphs == 0,
        "processed Unicode text used fallback glyphs");
    const auto unsupportedUnicode = Pyramid::Text::Layout(
        processed,
        "\xE2\x98\x83",
        Pyramid::Math::Vec2::Zero);
    Require(unsupportedUnicode.fallbackGlyphs == 1 &&
            processed.GetGlyph(U'\u2603').codepoint == U'?',
        "processed fallback glyph mismatch");

    std::cout << "Text tests passed\n";
    return EXIT_SUCCESS;
}
