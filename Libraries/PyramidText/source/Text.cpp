#include <Pyramid/Text/Text.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace Pyramid::Text
{
    namespace
    {
        constexpr u32 kColumns = 16;
        constexpr u32 kRows = 8;
        constexpr u32 kCellWidth = 8;
        constexpr u32 kCellHeight = 10;
        constexpr u32 kGlyphWidth = 5;
        constexpr u32 kGlyphHeight = 7;

        struct Pattern
        {
            char character;
            std::array<const char*, kGlyphHeight> rows;
        };

        constexpr Pattern kPatterns[] = {
            {'A', {"01110", "10001", "10001", "11111", "10001", "10001", "10001"}},
            {'B', {"11110", "10001", "10001", "11110", "10001", "10001", "11110"}},
            {'C', {"01111", "10000", "10000", "10000", "10000", "10000", "01111"}},
            {'D', {"11110", "10001", "10001", "10001", "10001", "10001", "11110"}},
            {'E', {"11111", "10000", "10000", "11110", "10000", "10000", "11111"}},
            {'F', {"11111", "10000", "10000", "11110", "10000", "10000", "10000"}},
            {'G', {"01111", "10000", "10000", "10111", "10001", "10001", "01111"}},
            {'H', {"10001", "10001", "10001", "11111", "10001", "10001", "10001"}},
            {'I', {"11111", "00100", "00100", "00100", "00100", "00100", "11111"}},
            {'J', {"00111", "00010", "00010", "00010", "10010", "10010", "01100"}},
            {'K', {"10001", "10010", "10100", "11000", "10100", "10010", "10001"}},
            {'L', {"10000", "10000", "10000", "10000", "10000", "10000", "11111"}},
            {'M', {"10001", "11011", "10101", "10101", "10001", "10001", "10001"}},
            {'N', {"10001", "11001", "10101", "10011", "10001", "10001", "10001"}},
            {'O', {"01110", "10001", "10001", "10001", "10001", "10001", "01110"}},
            {'P', {"11110", "10001", "10001", "11110", "10000", "10000", "10000"}},
            {'Q', {"01110", "10001", "10001", "10001", "10101", "10010", "01101"}},
            {'R', {"11110", "10001", "10001", "11110", "10100", "10010", "10001"}},
            {'S', {"01111", "10000", "10000", "01110", "00001", "00001", "11110"}},
            {'T', {"11111", "00100", "00100", "00100", "00100", "00100", "00100"}},
            {'U', {"10001", "10001", "10001", "10001", "10001", "10001", "01110"}},
            {'V', {"10001", "10001", "10001", "10001", "10001", "01010", "00100"}},
            {'W', {"10001", "10001", "10001", "10101", "10101", "10101", "01010"}},
            {'X', {"10001", "10001", "01010", "00100", "01010", "10001", "10001"}},
            {'Y', {"10001", "10001", "01010", "00100", "00100", "00100", "00100"}},
            {'Z', {"11111", "00001", "00010", "00100", "01000", "10000", "11111"}},
            {'0', {"01110", "10001", "10011", "10101", "11001", "10001", "01110"}},
            {'1', {"00100", "01100", "00100", "00100", "00100", "00100", "01110"}},
            {'2', {"01110", "10001", "00001", "00010", "00100", "01000", "11111"}},
            {'3', {"11110", "00001", "00001", "01110", "00001", "00001", "11110"}},
            {'4', {"00010", "00110", "01010", "10010", "11111", "00010", "00010"}},
            {'5', {"11111", "10000", "10000", "11110", "00001", "00001", "11110"}},
            {'6', {"01110", "10000", "10000", "11110", "10001", "10001", "01110"}},
            {'7', {"11111", "00001", "00010", "00100", "01000", "01000", "01000"}},
            {'8', {"01110", "10001", "10001", "01110", "10001", "10001", "01110"}},
            {'9', {"01110", "10001", "10001", "01111", "00001", "00001", "01110"}},
            {'?', {"01110", "10001", "00010", "00100", "00100", "00000", "00100"}},
            {'!', {"00100", "00100", "00100", "00100", "00100", "00000", "00100"}},
            {'.', {"00000", "00000", "00000", "00000", "00000", "00000", "00100"}},
            {',', {"00000", "00000", "00000", "00000", "00000", "00100", "01000"}},
            {':', {"00000", "00100", "00100", "00000", "00100", "00100", "00000"}},
            {';', {"00000", "00100", "00100", "00000", "00100", "01000", "00000"}},
            {'-', {"00000", "00000", "00000", "11111", "00000", "00000", "00000"}},
            {'_', {"00000", "00000", "00000", "00000", "00000", "00000", "11111"}},
            {'+', {"00000", "00100", "00100", "11111", "00100", "00100", "00000"}},
            {'=', {"00000", "11111", "00000", "11111", "00000", "00000", "00000"}},
            {'/', {"00001", "00010", "00010", "00100", "01000", "01000", "10000"}},
            {'\\', {"10000", "01000", "01000", "00100", "00010", "00010", "00001"}},
            {'(', {"00010", "00100", "01000", "01000", "01000", "00100", "00010"}},
            {')', {"01000", "00100", "00010", "00010", "00010", "00100", "01000"}},
            {'[', {"01110", "01000", "01000", "01000", "01000", "01000", "01110"}},
            {']', {"01110", "00010", "00010", "00010", "00010", "00010", "01110"}},
            {'<', {"00010", "00100", "01000", "10000", "01000", "00100", "00010"}},
            {'>', {"01000", "00100", "00010", "00001", "00010", "00100", "01000"}},
            {'#', {"01010", "11111", "01010", "01010", "11111", "01010", "01010"}},
            {'%', {"11001", "11010", "00100", "01000", "10110", "00110", "00000"}},
            {'&', {"01100", "10010", "10100", "01000", "10101", "10010", "01101"}},
            {'@', {"01110", "10001", "10111", "10101", "10111", "10000", "01110"}},
            {'|', {"00100", "00100", "00100", "00100", "00100", "00100", "00100"}},
            {'*', {"00000", "10101", "01110", "11111", "01110", "10101", "00000"}},
            {'\'', {"00100", "00100", "00010", "00000", "00000", "00000", "00000"}},
            {'\"', {"01010", "01010", "00100", "00000", "00000", "00000", "00000"}},
        };

        const Pattern* FindPattern(char character)
        {
            const char normalized = character >= 'a' && character <= 'z'
                ? static_cast<char>(character - 'a' + 'A')
                : character;
            for (const Pattern& pattern : kPatterns)
            {
                if (pattern.character == normalized)
                {
                    return &pattern;
                }
            }
            for (const Pattern& pattern : kPatterns)
            {
                if (pattern.character == '?')
                {
                    return &pattern;
                }
            }
            return nullptr;
        }

        void SetPixel(FontAtlas& atlas, u32 x, u32 y, u8 alpha)
        {
            const std::size_t offset =
                (static_cast<std::size_t>(y) * atlas.width + x) * 4U;
            atlas.rgbaPixels[offset + 0] = 255;
            atlas.rgbaPixels[offset + 1] = 255;
            atlas.rgbaPixels[offset + 2] = 255;
            atlas.rgbaPixels[offset + 3] = alpha;
        }
    } // namespace

    bool FontAtlas::IsValid() const
    {
        return width > 0 && height > 0 && lineHeight > 0.0f &&
            rgbaPixels.size() == static_cast<std::size_t>(width) * height * 4U;
    }

    const Glyph& FontAtlas::GetGlyph(char32_t codepoint) const
    {
        const std::size_t index = codepoint < glyphs.size()
            ? static_cast<std::size_t>(codepoint)
            : static_cast<std::size_t>('?');
        return glyphs[index];
    }

    FontAtlas CreateDebugFontAtlas()
    {
        FontAtlas atlas;
        atlas.width = kColumns * kCellWidth;
        atlas.height = kRows * kCellHeight;
        atlas.lineHeight = static_cast<f32>(kCellHeight);
        atlas.rgbaPixels.assign(
            static_cast<std::size_t>(atlas.width) * atlas.height * 4U,
            0);

        SetPixel(atlas, 0, 0, 255);
        atlas.whitePixelUv = Math::Vec2(
            0.5f / static_cast<f32>(atlas.width),
            0.5f / static_cast<f32>(atlas.height));

        for (u32 code = 0; code < atlas.glyphs.size(); ++code)
        {
            Glyph glyph;
            glyph.codepoint = static_cast<char32_t>(code);
            glyph.advance = code == static_cast<u32>(' ') ? 4.0f : 6.0f;
            glyph.width = code == static_cast<u32>(' ') ? 0.0f : 5.0f;
            glyph.height = code == static_cast<u32>(' ') ? 0.0f : 7.0f;
            glyph.bearingX = 0.0f;
            glyph.bearingY = 0.0f;

            const u32 cellX = (code % kColumns) * kCellWidth;
            const u32 cellY = (code / kColumns) * kCellHeight;
            const Pattern* pattern = FindPattern(static_cast<char>(code));
            if (code != static_cast<u32>(' ') && pattern)
            {
                for (u32 y = 0; y < kGlyphHeight; ++y)
                {
                    for (u32 x = 0; x < kGlyphWidth; ++x)
                    {
                        if (pattern->rows[y][x] == '1')
                        {
                            SetPixel(atlas, cellX + 1U + x, cellY + 1U + y, 255);
                        }
                    }
                }
            }

            glyph.u0 = static_cast<f32>(cellX + 1U) / static_cast<f32>(atlas.width);
            glyph.v0 = static_cast<f32>(cellY + 1U) / static_cast<f32>(atlas.height);
            glyph.u1 = static_cast<f32>(cellX + 1U + kGlyphWidth) /
                static_cast<f32>(atlas.width);
            glyph.v1 = static_cast<f32>(cellY + 1U + kGlyphHeight) /
                static_cast<f32>(atlas.height);
            atlas.glyphs[code] = glyph;
        }
        return atlas;
    }

    TextMetrics Measure(const FontAtlas& font, std::string_view text, f32 scale)
    {
        TextMetrics metrics;
        if (!font.IsValid() || scale <= 0.0f)
        {
            return metrics;
        }

        f32 lineWidth = 0.0f;
        metrics.lineCount = 1;
        for (unsigned char byte : text)
        {
            if (byte == '\n')
            {
                metrics.width = (std::max)(metrics.width, lineWidth);
                lineWidth = 0.0f;
                ++metrics.lineCount;
                continue;
            }
            lineWidth += font.GetGlyph(static_cast<char32_t>(byte)).advance * scale;
        }
        metrics.width = (std::max)(metrics.width, lineWidth);
        metrics.height = static_cast<f32>(metrics.lineCount) * font.lineHeight * scale;
        return metrics;
    }

    void BuildGlyphQuads(
        const FontAtlas& font,
        std::string_view text,
        const Math::Vec2& origin,
        f32 scale,
        std::vector<GlyphQuad>& output)
    {
        if (!font.IsValid() || scale <= 0.0f)
        {
            return;
        }

        f32 x = origin.x;
        f32 y = origin.y;
        for (unsigned char byte : text)
        {
            if (byte == '\n')
            {
                x = origin.x;
                y += font.lineHeight * scale;
                continue;
            }

            const Glyph& glyph = font.GetGlyph(static_cast<char32_t>(byte));
            if (glyph.width > 0.0f && glyph.height > 0.0f)
            {
                GlyphQuad quad;
                quad.minimum = Math::Vec2(
                    x + glyph.bearingX * scale,
                    y + glyph.bearingY * scale);
                quad.maximum = quad.minimum +
                    Math::Vec2(glyph.width * scale, glyph.height * scale);
                quad.uvMinimum = Math::Vec2(glyph.u0, glyph.v0);
                quad.uvMaximum = Math::Vec2(glyph.u1, glyph.v1);
                quad.codepoint = glyph.codepoint;
                output.push_back(quad);
            }
            x += glyph.advance * scale;
        }
    }
} // namespace Pyramid::Text
