#include <Pyramid/Font/Font.hpp>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

    std::ifstream sourceStream(PYRAMID_FONT_TEST_ASSET, std::ios::binary);
    const std::vector<Pyramid::u8> sourceBytes(
        (std::istreambuf_iterator<char>(sourceStream)),
        std::istreambuf_iterator<char>());
    if (sourceBytes.empty())
    {
        return Fail("owned TrueType test asset could not be read");
    }

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

    RasterOptions distanceOptions;
    distanceOptions.pixelHeight = 64.0f;
    distanceOptions.mode = RasterMode::SignedDistanceField;
    distanceOptions.distanceRange = 8.0f;
    const RasterizedGlyph distanceGlyph = RasterizeGlyph(
        loaded.face, loaded.face.GetGlyphId(U'A'), distanceOptions);
    if (!distanceGlyph.IsValid() || distanceGlyph.width <= letter.width ||
        distanceGlyph.height <= letter.height)
    {
        return Fail("signed-distance rasterization did not preserve an exterior field");
    }
    bool hasInside = false;
    bool hasOutside = false;
    bool hasBoundary = false;
    for (Pyramid::u8 sample : distanceGlyph.alphaPixels)
    {
        hasInside = hasInside || sample > 160U;
        hasOutside = hasOutside || sample < 96U;
        hasBoundary = hasBoundary || (sample >= 112U && sample <= 144U);
    }
    if (!hasInside || !hasOutside || !hasBoundary)
    {
        return Fail("signed-distance rasterization has an invalid value field");
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
        roundTrip.rgbaPixels != baked.font.rgbaPixels ||
        roundTrip.rasterMode != RasterMode::Coverage ||
        roundTrip.distanceRange != 0.0f)
    {
        return Fail("processed font round trip failed");
    }

    BakeOptions distanceBake = bakeOptions;
    distanceBake.pixelHeight = 64.0f;
    distanceBake.atlasWidth = 1024;
    distanceBake.atlasHeight = 1024;
    distanceBake.mode = RasterMode::SignedDistanceField;
    distanceBake.distanceRange = 10.0f;
    const BakeResult distanceBaked = BakeFont(loaded.face, distanceBake);
    std::vector<Pyramid::u8> distanceBytes;
    BakedFont distanceRoundTrip;
    if (!distanceBaked.Succeeded() ||
        !SaveProcessedFont(distanceBaked.font, distanceBytes, &error) ||
        !LoadProcessedFont(
            distanceBytes.data(), distanceBytes.size(), distanceRoundTrip, &error) ||
        distanceRoundTrip.rasterMode != RasterMode::SignedDistanceField ||
        std::fabs(distanceRoundTrip.distanceRange - 10.0f) > 0.001f ||
        distanceRoundTrip.rgbaPixels != distanceBaked.font.rgbaPixels)
    {
        return Fail("signed-distance processed font round trip failed");
    }

    BakeOptions cachedOptions;
    cachedOptions.pixelHeight = 32.0f;
    cachedOptions.atlasWidth = 512;
    cachedOptions.atlasHeight = 512;
    cachedOptions.mode = RasterMode::SignedDistanceField;
    cachedOptions.distanceRange = 6.0f;
    cachedOptions.missingGlyphPolicy = MissingGlyphPolicy::Skip;
    cachedOptions.ranges = {{U' ', U'~'}, {U'☃', U'☃'}};
    const std::string cacheKey = BuildProcessedFontCacheKey(
        sourceBytes.data(), sourceBytes.size(), cachedOptions);
    BakeOptions changedCachedOptions = cachedOptions;
    changedCachedOptions.distanceRange = 7.0f;
    if (cacheKey.size() != 16U || cacheKey != BuildProcessedFontCacheKey(
            sourceBytes.data(), sourceBytes.size(), cachedOptions) ||
        cacheKey == BuildProcessedFontCacheKey(
            sourceBytes.data(), sourceBytes.size(), changedCachedOptions))
    {
        return Fail("processed-font cache key is not deterministic or option-sensitive");
    }

    const std::filesystem::path cacheDirectory =
        std::filesystem::temp_directory_path() / "pyramid-font-cache-test";
    std::error_code filesystemError;
    std::filesystem::remove_all(cacheDirectory, filesystemError);
    const CachedBakeResult firstCached = LoadOrBakeProcessedFont(
        sourceBytes.data(), sourceBytes.size(), cachedOptions, cacheDirectory);
    const CachedBakeResult secondCached = LoadOrBakeProcessedFont(
        sourceBytes.data(), sourceBytes.size(), cachedOptions, cacheDirectory);
    if (!firstCached.Succeeded() || firstCached.cacheHit ||
        firstCached.font.FindGlyph(U'☃') != nullptr ||
        !secondCached.Succeeded() || !secondCached.cacheHit ||
        firstCached.cachePath != secondCached.cachePath ||
        firstCached.font.rgbaPixels != secondCached.font.rgbaPixels)
    {
        std::filesystem::remove_all(cacheDirectory, filesystemError);
        return Fail("content-addressed processed-font cache failed");
    }
    std::filesystem::remove_all(cacheDirectory, filesystemError);

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
