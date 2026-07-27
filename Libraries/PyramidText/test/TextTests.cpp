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

    std::cout << "Text tests passed\n";
    return EXIT_SUCCESS;
}
