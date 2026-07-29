#include <Pyramid/Font/Font.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "Reference typography test failed: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool HasCoverageNearFontX(
        const Pyramid::Font::RasterizedGlyph& glyph,
        float scale,
        float fontX)
    {
        const int center = static_cast<int>(std::lround(fontX * scale)) - glyph.bearingX;
        const int first = (std::max)(0, center - 1);
        const int last = (std::min)(static_cast<int>(glyph.width) - 1, center + 1);
        for (int x = first; x <= last; ++x)
        {
            for (Pyramid::u32 y = 0; y < glyph.height; ++y)
            {
                if (glyph.alphaPixels[static_cast<std::size_t>(y) * glyph.width +
                        static_cast<std::size_t>(x)] > 32U)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool HasAntialiasing(const Pyramid::Font::RasterizedGlyph& glyph)
    {
        return std::any_of(
            glyph.alphaPixels.begin(), glyph.alphaPixels.end(),
            [](Pyramid::u8 alpha) { return alpha > 0U && alpha < 255U; });
    }

    float CoverageRatio(const Pyramid::Font::RasterizedGlyph& glyph)
    {
        const std::size_t covered = static_cast<std::size_t>(std::count_if(
            glyph.alphaPixels.begin(), glyph.alphaPixels.end(),
            [](Pyramid::u8 alpha) { return alpha > 32U; }));
        return glyph.alphaPixels.empty()
            ? 0.0f
            : static_cast<float>(covered) /
                static_cast<float>(glyph.alphaPixels.size());
    }
}

int main()
{
    using namespace Pyramid::Font;

    const LoadResult source = LoadTrueTypeFile(PYRAMID_FONT_TEST_ARABIC_SOURCE);
    if (!source.Succeeded() || source.face.familyName != "Pyramid Arabic" ||
        source.face.metrics.size() < 260U ||
        source.face.GetGlyphId(U'A') == 0U ||
        source.face.GetGlyphId(U' ') == 0U ||
        source.face.GetGlyphId(U'\u0645') == 0U ||
        source.face.GetGlyphId(U'\uFEE1') == 0U ||
        source.face.GetGlyphId(U'\uFEE4') == 0U ||
        source.face.GetGlyphId(U'\u0661') == 0U ||
        source.face.GetGlyphId(U'\u061F') == 0U)
    {
        return Fail("owned Arabic source font coverage is incomplete");
    }

    RasterOptions options;
    options.pixelHeight = 48.0f;
    options.oversample = 4U;
    options.padding = 1U;
    const float scale = options.pixelHeight /
        static_cast<float>(source.face.unitsPerEm);

    for (char32_t codepoint = U'\uFE80'; codepoint <= U'\uFEF4'; ++codepoint)
    {
        const GlyphId glyphId = source.face.GetGlyphId(codepoint);
        if (glyphId == 0U)
        {
            return Fail("core Arabic presentation-form coverage has a gap");
        }
        const RasterizedGlyph form = RasterizeGlyph(source.face, glyphId, options);
        if (!form.IsValid() || form.width == 0U || form.height == 0U ||
            CoverageRatio(form) < 0.015f)
        {
            return Fail("core Arabic presentation form is empty or too sparse");
        }
    }

    const RasterizedGlyph initial = RasterizeGlyph(
        source.face, source.face.GetGlyphId(U'\uFE91'), options); // beh initial
    const RasterizedGlyph final = RasterizeGlyph(
        source.face, source.face.GetGlyphId(U'\uFE90'), options); // beh final
    const RasterizedGlyph medial = RasterizeGlyph(
        source.face, source.face.GetGlyphId(U'\uFE92'), options); // beh medial
    const RasterizedGlyph isolated = RasterizeGlyph(
        source.face, source.face.GetGlyphId(U'\uFE8F'), options); // beh isolated

    if (!initial.IsValid() || !final.IsValid() || !medial.IsValid() ||
        !isolated.IsValid() || !HasAntialiasing(medial))
    {
        return Fail("reference Arabic forms did not rasterize with antialiasing");
    }
    if (!HasCoverageNearFontX(initial, scale, 0.0f) ||
        HasCoverageNearFontX(initial, scale, 620.0f) ||
        HasCoverageNearFontX(final, scale, 0.0f) ||
        !HasCoverageNearFontX(final, scale, 620.0f) ||
        !HasCoverageNearFontX(medial, scale, 0.0f) ||
        !HasCoverageNearFontX(medial, scale, 620.0f) ||
        HasCoverageNearFontX(isolated, scale, 0.0f) ||
        HasCoverageNearFontX(isolated, scale, 620.0f))
    {
        return Fail("contextual forms do not expose the expected joining edges");
    }

    const float density = CoverageRatio(medial);
    if (density < 0.12f || density > 0.72f)
    {
        return Fail("Arabic reference glyph is too sparse or too heavy");
    }

    BakedFont processed;
    std::string error;
    if (!LoadProcessedFontFile(
            PYRAMID_FONT_TEST_ARABIC_PROCESSED, processed, &error) ||
        processed.familyName != "Pyramid Arabic" ||
        std::fabs(processed.pixelHeight - 64.0f) > 0.001f ||
        processed.atlasWidth != 1024U || processed.atlasHeight != 1024U ||
        processed.rasterMode != RasterMode::SignedDistanceField ||
        std::fabs(processed.distanceRange - 10.0f) > 0.001f ||
        processed.glyphs.size() < 260U ||
        !processed.FindGlyph(U'\u0645') ||
        !processed.FindGlyph(U'\uFEE3') ||
        !processed.FindGlyph(U'\uFEEC'))
    {
        return Fail("scalable SDF processed Arabic atlas is invalid");
    }

    std::cout << "Reference typography tests passed\n";
    return EXIT_SUCCESS;
}
