#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Math/Vec2.hpp>

#include <array>
#include <string_view>
#include <vector>

namespace Pyramid::Text
{
    struct Glyph
    {
        char32_t codepoint = U'?';
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

    struct FontAtlas
    {
        u32 width = 0;
        u32 height = 0;
        f32 lineHeight = 0.0f;
        std::vector<u8> rgbaPixels;
        std::array<Glyph, 128> glyphs{};
        Math::Vec2 whitePixelUv = Math::Vec2::Zero;

        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] const Glyph& GetGlyph(char32_t codepoint) const;
    };

    struct GlyphQuad
    {
        Math::Vec2 minimum = Math::Vec2::Zero;
        Math::Vec2 maximum = Math::Vec2::Zero;
        Math::Vec2 uvMinimum = Math::Vec2::Zero;
        Math::Vec2 uvMaximum = Math::Vec2::Zero;
        char32_t codepoint = U'?';
    };

    struct TextMetrics
    {
        f32 width = 0.0f;
        f32 height = 0.0f;
        u32 lineCount = 0;
    };

    enum class HorizontalAlignment : u8
    {
        Left = 0,
        Center,
        Right
    };

    enum class WrapMode : u8
    {
        None = 0,
        Character,
        Word
    };

    struct LayoutOptions
    {
        f32 scale = 1.0f;
        f32 maximumWidth = 0.0f;
        f32 lineSpacing = 0.0f;
        u32 tabWidth = 4;
        HorizontalAlignment alignment = HorizontalAlignment::Left;
        WrapMode wrap = WrapMode::None;

        [[nodiscard]] bool IsValid() const;
    };

    struct LineMetrics
    {
        u32 firstGlyph = 0;
        u32 glyphCount = 0;
        f32 width = 0.0f;
    };

    struct LayoutResult
    {
        TextMetrics metrics;
        std::vector<GlyphQuad> glyphs;
        std::vector<LineMetrics> lines;
        u32 invalidUtf8Sequences = 0;
    };

    /**
     * Built-in deterministic debug font. It is intentionally small and ASCII-only;
     * unsupported Unicode code points resolve to the replacement glyph. UTF-8
     * decoding, wrapping and alignment are nevertheless handled by the owned text
     * layout layer so game/editor font backends can reuse the same layout contract.
     */
    [[nodiscard]] FontAtlas CreateDebugFontAtlas();

    [[nodiscard]] LayoutResult Layout(
        const FontAtlas& font,
        std::string_view utf8Text,
        const Math::Vec2& origin,
        const LayoutOptions& options = {});

    [[nodiscard]] TextMetrics Measure(
        const FontAtlas& font,
        std::string_view text,
        f32 scale = 1.0f);

    void BuildGlyphQuads(
        const FontAtlas& font,
        std::string_view text,
        const Math::Vec2& origin,
        f32 scale,
        std::vector<GlyphQuad>& output);
} // namespace Pyramid::Text
