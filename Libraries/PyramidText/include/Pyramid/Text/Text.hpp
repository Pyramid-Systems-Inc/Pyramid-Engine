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

    /**
     * Built-in deterministic debug font. It is intentionally small and ASCII-only;
     * production Unicode shaping and font import belong to later text milestones.
     */
    [[nodiscard]] FontAtlas CreateDebugFontAtlas();

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
