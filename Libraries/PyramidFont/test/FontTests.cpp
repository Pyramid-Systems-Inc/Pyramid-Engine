#include <Pyramid/Font/Font.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "Font test failed: " << message << '\n';
        return EXIT_FAILURE;
    }
}

int main()
{
    using namespace Pyramid::Font;

    const LoadResult loaded = LoadTrueTypeFile(PYRAMID_FONT_TEST_ASSET);
    if (!loaded.Succeeded() || loaded.face.familyName != "Pyramid Sans" ||
        loaded.face.metrics.size() != 99 || loaded.face.GetGlyphId(U'A') == 0 ||
        loaded.face.GetGlyphId(U'~') == 0 || loaded.face.GetGlyphId(U'\u00E9') == 0 ||
        loaded.face.GetGlyphId(U'\u03A9') == 0 || loaded.face.GetGlyphId(U'\u2713') == 0 ||
        loaded.face.GetGlyphId(U'\u2603') != 0)
    {
        return Fail("owned TrueType asset was not parsed correctly");
    }
    if (loaded.face.GetKerning(loaded.face.GetGlyphId(U'A'), loaded.face.GetGlyphId(U'V')) >= 0)
    {
        return Fail("kern format 0 pair was not loaded");
    }

    RasterOptions rasterOptions;
    rasterOptions.pixelHeight = 24.0f;
    rasterOptions.oversample = 3;
    const RasterizedGlyph letter = RasterizeGlyph(
        loaded.face, loaded.face.GetGlyphId(U'A'), rasterOptions);
    if (!letter.IsValid() || letter.width == 0 || letter.height == 0 ||
        letter.alphaPixels.empty())
    {
        return Fail("outline rasterization failed");
    }
    bool hasCoverage = false;
    for (Pyramid::u8 alpha : letter.alphaPixels)
    {
        hasCoverage = hasCoverage || alpha != 0;
    }
    if (!hasCoverage)
    {
        return Fail("rasterized glyph has no coverage");
    }

    BakeOptions bakeOptions;
    bakeOptions.pixelHeight = 24.0f;
    bakeOptions.atlasWidth = 256;
    bakeOptions.atlasHeight = 256;
    bakeOptions.ranges.push_back({U'\u00E9', U'\u00E9'});
    bakeOptions.ranges.push_back({U'\u03A9', U'\u03A9'});
    bakeOptions.ranges.push_back({U'\u2713', U'\u2713'});
    const BakeResult baked = BakeFont(loaded.face, bakeOptions);
    if (!baked.Succeeded() || baked.font.glyphs.size() != 98 ||
        !baked.font.FindGlyph(U'A') || !baked.font.FindGlyph(U'?') ||
        !baked.font.FindGlyph(U'\u00E9') || !baked.font.FindGlyph(U'\u03A9') ||
        !baked.font.FindGlyph(U'\u2713') ||
        baked.font.GetKerning(U'A', U'V') >= 0.0f)
    {
        return Fail("font baking or kerning conversion failed");
    }

    std::vector<Pyramid::u8> bytes;
    std::string error;
    if (!SaveProcessedFont(baked.font, bytes, &error) || bytes.size() < 1024)
    {
        return Fail("processed font serialization failed");
    }
    BakedFont roundTrip;
    if (!LoadProcessedFont(bytes.data(), bytes.size(), roundTrip, &error) ||
        roundTrip.familyName != baked.font.familyName ||
        roundTrip.glyphs.size() != baked.font.glyphs.size() ||
        roundTrip.rgbaPixels != baked.font.rgbaPixels)
    {
        return Fail("processed font round trip failed");
    }

    bytes[bytes.size() / 2] ^= 0x01U;
    if (LoadProcessedFont(bytes.data(), bytes.size(), roundTrip, &error))
    {
        return Fail("corrupted processed font was accepted");
    }

    const Pyramid::u8 invalid[] = {0, 1, 2, 3, 4, 5};
    if (LoadTrueType(invalid, sizeof(invalid)).Succeeded())
    {
        return Fail("malformed TrueType input was accepted");
    }
    LoadOptions limits;
    limits.maximumFileBytes = 16;
    if (LoadTrueTypeFile(PYRAMID_FONT_TEST_ASSET, limits).Succeeded())
    {
        return Fail("font file size limit was ignored");
    }

    std::cout << "PyramidFont tests passed\n";
    return EXIT_SUCCESS;
}
