#pragma once

#include <Pyramid/Core/Prerequisites.hpp>

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Pyramid::Font
{
    using GlyphId = u16;

    enum class DiagnosticSeverity : u8
    {
        Warning = 0,
        Error
    };

    struct Diagnostic
    {
        DiagnosticSeverity severity = DiagnosticSeverity::Error;
        std::string message;
        std::size_t offset = 0;
    };

    struct OutlinePoint
    {
        f32 x = 0.0f;
        f32 y = 0.0f;
        bool onCurve = true;
    };

    struct OutlineContour
    {
        std::vector<OutlinePoint> points;
    };

    struct GlyphOutline
    {
        i16 xMin = 0;
        i16 yMin = 0;
        i16 xMax = 0;
        i16 yMax = 0;
        std::vector<OutlineContour> contours;
    };

    struct GlyphMetrics
    {
        u16 advanceWidth = 0;
        i16 leftSideBearing = 0;
    };

    struct KerningPair
    {
        GlyphId left = 0;
        GlyphId right = 0;
        i16 value = 0;
    };

    struct FontFace
    {
        std::string familyName;
        u16 unitsPerEm = 0;
        i16 ascender = 0;
        i16 descender = 0;
        i16 lineGap = 0;
        std::vector<GlyphMetrics> metrics;
        std::vector<GlyphOutline> outlines;
        std::unordered_map<char32_t, GlyphId> characterMap;
        std::vector<KerningPair> kerning;

        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] GlyphId GetGlyphId(char32_t codepoint) const;
        [[nodiscard]] i16 GetKerning(GlyphId left, GlyphId right) const;
    };

    struct LoadOptions
    {
        std::size_t maximumFileBytes = 64U * 1024U * 1024U;
        u32 maximumTables = 256;
        u32 maximumGlyphs = 65535;
        u32 maximumMappings = 1000000;
        u32 maximumCompoundDepth = 32;
    };

    struct LoadResult
    {
        FontFace face;
        std::vector<Diagnostic> diagnostics;

        [[nodiscard]] bool Succeeded() const { return face.IsValid(); }
    };

    [[nodiscard]] LoadResult LoadTrueType(
        const u8* data,
        std::size_t size,
        const LoadOptions& options = {});
    [[nodiscard]] LoadResult LoadTrueTypeFile(
        std::string_view path,
        const LoadOptions& options = {});

    enum class RasterMode : u8
    {
        Coverage = 0,
        SignedDistanceField
    };

    struct RasterOptions
    {
        f32 pixelHeight = 24.0f;
        u32 oversample = 4;
        u32 padding = 1;
        u32 maximumBitmapDimension = 4096;
        RasterMode mode = RasterMode::Coverage;
        f32 distanceRange = 8.0f;

        [[nodiscard]] bool IsValid() const;
    };

    struct RasterizedGlyph
    {
        GlyphId glyph = 0;
        u32 width = 0;
        u32 height = 0;
        i32 bearingX = 0;
        i32 bearingY = 0;
        f32 advance = 0.0f;
        std::vector<u8> alphaPixels;

        [[nodiscard]] bool IsValid() const;
    };

    [[nodiscard]] RasterizedGlyph RasterizeGlyph(
        const FontFace& face,
        GlyphId glyph,
        const RasterOptions& options = {});

    struct CharacterRange
    {
        char32_t first = U' ';
        char32_t last = U'~';
    };

    enum class MissingGlyphPolicy : u8
    {
        UseFallback = 0,
        Skip
    };

    struct BakeOptions
    {
        f32 pixelHeight = 24.0f;
        u32 atlasWidth = 512;
        u32 atlasHeight = 512;
        u32 oversample = 4;
        u32 padding = 1;
        RasterMode mode = RasterMode::Coverage;
        f32 distanceRange = 8.0f;
        char32_t fallbackCodepoint = U'?';
        MissingGlyphPolicy missingGlyphPolicy = MissingGlyphPolicy::UseFallback;
        std::vector<CharacterRange> ranges = {{U' ', U'~'}};

        [[nodiscard]] bool IsValid() const;
    };

    struct BakedGlyph
    {
        char32_t codepoint = U'?';
        GlyphId glyph = 0;
        f32 advance = 0.0f;
        f32 width = 0.0f;
        f32 height = 0.0f;
        f32 bearingX = 0.0f;
        f32 bearingY = 0.0f;
        f32 u0 = 0.0f;
        f32 v0 = 0.0f;
        f32 u1 = 0.0f;
        f32 v1 = 0.0f;
    };

    struct BakedKerning
    {
        char32_t left = 0;
        char32_t right = 0;
        f32 value = 0.0f;
    };

    struct BakedFont
    {
        std::string familyName;
        u32 atlasWidth = 0;
        u32 atlasHeight = 0;
        f32 pixelHeight = 0.0f;
        f32 lineHeight = 0.0f;
        f32 ascent = 0.0f;
        f32 descent = 0.0f;
        RasterMode rasterMode = RasterMode::Coverage;
        f32 distanceRange = 0.0f;
        char32_t fallbackCodepoint = U'?';
        std::vector<u8> rgbaPixels;
        std::vector<BakedGlyph> glyphs;
        std::vector<BakedKerning> kerning;

        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] const BakedGlyph* FindGlyph(char32_t codepoint) const;
        [[nodiscard]] f32 GetKerning(char32_t left, char32_t right) const;
    };

    struct BakeResult
    {
        BakedFont font;
        std::vector<Diagnostic> diagnostics;

        [[nodiscard]] bool Succeeded() const { return font.IsValid(); }
    };

    [[nodiscard]] BakeResult BakeFont(
        const FontFace& face,
        const BakeOptions& options = {});

    struct CachedBakeResult
    {
        BakedFont font;
        std::vector<Diagnostic> diagnostics;
        std::string cacheKey;
        std::filesystem::path cachePath;
        bool cacheHit = false;

        [[nodiscard]] bool Succeeded() const { return font.IsValid(); }
    };

    /**
     * Returns a deterministic cache key for a TrueType payload and bake options.
     * The key is stable across processes and includes every option that affects
     * processed output.
     */
    [[nodiscard]] std::string BuildProcessedFontCacheKey(
        const u8* data,
        std::size_t size,
        const BakeOptions& options);

    /**
     * Loads a processed font from a content-addressed cache or builds it through
     * Pyramid's owned TrueType parser and rasterizer. Cache writes are staged and
     * atomically published when the host filesystem supports rename.
     */
    [[nodiscard]] CachedBakeResult LoadOrBakeProcessedFont(
        const u8* data,
        std::size_t size,
        const BakeOptions& options,
        const std::filesystem::path& cacheDirectory);

    [[nodiscard]] bool SaveProcessedFont(
        const BakedFont& font,
        std::vector<u8>& output,
        std::string* error = nullptr);
    [[nodiscard]] bool SaveProcessedFontFile(
        const BakedFont& font,
        std::string_view path,
        std::string* error = nullptr);
    [[nodiscard]] bool LoadProcessedFont(
        const u8* data,
        std::size_t size,
        BakedFont& output,
        std::string* error = nullptr);
    [[nodiscard]] bool LoadProcessedFontFile(
        std::string_view path,
        BakedFont& output,
        std::string* error = nullptr);
} // namespace Pyramid::Font
