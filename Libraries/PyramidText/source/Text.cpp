#include <Pyramid/Text/Text.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <string_view>
#include <tuple>

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
        return width > 0 && height > 0 && std::isfinite(lineHeight) && lineHeight > 0.0f &&
            rgbaPixels.size() == static_cast<std::size_t>(width) * height * 4U &&
            !glyphs.empty() &&
            std::is_sorted(glyphs.begin(), glyphs.end(), [](const Glyph& a, const Glyph& b)
            {
                return a.codepoint < b.codepoint;
            }) &&
            HasGlyph(fallbackCodepoint);
    }

    bool FontAtlas::HasGlyph(char32_t codepoint) const
    {
        const auto found = std::lower_bound(glyphs.begin(), glyphs.end(), codepoint,
            [](const Glyph& glyph, char32_t value)
            {
                return glyph.codepoint < value;
            });
        return found != glyphs.end() && found->codepoint == codepoint;
    }

    const Glyph& FontAtlas::GetGlyph(char32_t codepoint) const
    {
        auto find = [&](char32_t value) -> const Glyph*
        {
            const auto found = std::lower_bound(glyphs.begin(), glyphs.end(), value,
                [](const Glyph& glyph, char32_t candidate)
                {
                    return glyph.codepoint < candidate;
                });
            return found != glyphs.end() && found->codepoint == value ? &*found : nullptr;
        };
        if (const Glyph* glyph = find(codepoint))
        {
            return *glyph;
        }
        if (const Glyph* fallback = find(fallbackCodepoint))
        {
            return *fallback;
        }
        return glyphs.front();
    }

    f32 FontAtlas::GetKerning(char32_t left, char32_t right) const
    {
        const auto found = std::lower_bound(
            kerning.begin(), kerning.end(), std::pair<char32_t, char32_t>{left, right},
            [](const Font::BakedKerning& pair, const std::pair<char32_t, char32_t>& key)
            {
                return std::tie(pair.left, pair.right) < std::tie(key.first, key.second);
            });
        return found != kerning.end() && found->left == left && found->right == right
            ? found->value
            : 0.0f;
    }

    FontAtlas CreateDebugFontAtlas()
    {
        FontAtlas atlas;
        atlas.familyName = "Pyramid Embedded Debug";
        atlas.width = kColumns * kCellWidth;
        atlas.height = kRows * kCellHeight;
        atlas.pixelHeight = static_cast<f32>(kGlyphHeight);
        atlas.lineHeight = static_cast<f32>(kCellHeight);
        atlas.fallbackCodepoint = U'?';
        atlas.rgbaPixels.assign(
            static_cast<std::size_t>(atlas.width) * atlas.height * 4U,
            0);
        atlas.glyphs.resize(128);

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

    FontAtlas CreateFontAtlas(const Font::BakedFont& font)
    {
        FontAtlas atlas;
        if (!font.IsValid())
        {
            return atlas;
        }
        atlas.familyName = font.familyName;
        atlas.width = font.atlasWidth;
        atlas.height = font.atlasHeight;
        atlas.pixelHeight = font.pixelHeight;
        atlas.lineHeight = font.lineHeight;
        atlas.rgbaPixels = font.rgbaPixels;
        atlas.kerning = font.kerning;
        atlas.fallbackCodepoint = font.fallbackCodepoint;
        atlas.whitePixelUv = Math::Vec2(
            0.5f / static_cast<f32>(atlas.width),
            0.5f / static_cast<f32>(atlas.height));
        atlas.glyphs.reserve(font.glyphs.size());
        for (const Font::BakedGlyph& source : font.glyphs)
        {
            Glyph glyph;
            glyph.codepoint = source.codepoint;
            glyph.advance = source.advance;
            glyph.width = source.width;
            glyph.height = source.height;
            glyph.bearingX = source.bearingX;
            glyph.bearingY = source.bearingY;
            glyph.u0 = source.u0;
            glyph.v0 = source.v0;
            glyph.u1 = source.u1;
            glyph.v1 = source.v1;
            atlas.glyphs.push_back(glyph);
        }
        return atlas;
    }

    bool LoadFontAtlas(
        std::string_view processedFontPath,
        FontAtlas& output,
        std::string* error)
    {
        Font::BakedFont font;
        if (!Font::LoadProcessedFontFile(processedFontPath, font, error))
        {
            output = {};
            return false;
        }
        output = CreateFontAtlas(font);
        if (!output.IsValid())
        {
            output = {};
            if (error)
            {
                *error = "processed font could not be converted to a text atlas";
            }
            return false;
        }
        return true;
    }

    namespace
    {
        struct DecodedText
        {
            std::vector<char32_t> codepoints;
            u32 invalidSequences = 0;
        };

        bool IsContinuation(u8 byte)
        {
            return (byte & 0xC0U) == 0x80U;
        }

        DecodedText DecodeUtf8(std::string_view text, u32 tabWidth)
        {
            DecodedText decoded;
            decoded.codepoints.reserve(text.size());
            const u32 safeTabWidth = (std::max)(1U, tabWidth);

            std::size_t index = 0;
            while (index < text.size())
            {
                const u8 lead = static_cast<u8>(text[index]);
                if (lead == static_cast<u8>('\r'))
                {
                    ++index;
                    continue;
                }
                if (lead == static_cast<u8>('\t'))
                {
                    decoded.codepoints.insert(decoded.codepoints.end(), safeTabWidth, U' ');
                    ++index;
                    continue;
                }
                if (lead < 0x80U)
                {
                    decoded.codepoints.push_back(static_cast<char32_t>(lead));
                    ++index;
                    continue;
                }

                u32 length = 0;
                char32_t codepoint = 0;
                char32_t minimum = 0;
                if ((lead & 0xE0U) == 0xC0U)
                {
                    length = 2;
                    codepoint = static_cast<char32_t>(lead & 0x1FU);
                    minimum = 0x80;
                }
                else if ((lead & 0xF0U) == 0xE0U)
                {
                    length = 3;
                    codepoint = static_cast<char32_t>(lead & 0x0FU);
                    minimum = 0x800;
                }
                else if ((lead & 0xF8U) == 0xF0U)
                {
                    length = 4;
                    codepoint = static_cast<char32_t>(lead & 0x07U);
                    minimum = 0x10000;
                }

                bool valid = length != 0 && index + length <= text.size();
                if (valid)
                {
                    for (u32 offset = 1; offset < length; ++offset)
                    {
                        const u8 continuation = static_cast<u8>(text[index + offset]);
                        if (!IsContinuation(continuation))
                        {
                            valid = false;
                            break;
                        }
                        codepoint = static_cast<char32_t>(
                            (static_cast<u32>(codepoint) << 6U) |
                            static_cast<u32>(continuation & 0x3FU));
                    }
                }
                if (valid)
                {
                    valid = codepoint >= minimum && codepoint <= 0x10FFFF &&
                        !(codepoint >= 0xD800 && codepoint <= 0xDFFF);
                }

                if (!valid)
                {
                    decoded.codepoints.push_back(U'?');
                    ++decoded.invalidSequences;
                    ++index;
                    while (index < text.size() && IsContinuation(static_cast<u8>(text[index])))
                    {
                        ++index;
                    }
                    continue;
                }

                decoded.codepoints.push_back(codepoint);
                index += length;
            }
            return decoded;
        }

        f32 AdvanceFor(const FontAtlas& font, char32_t codepoint, f32 scale)
        {
            return font.GetGlyph(codepoint).advance * scale;
        }

        f32 PairAdjustment(
            const FontAtlas& font,
            char32_t left,
            char32_t right,
            f32 scale)
        {
            return font.GetKerning(left, right) * scale;
        }

        struct PendingLine
        {
            std::vector<char32_t> codepoints;
            f32 width = 0.0f;
        };

        f32 MeasureCodepoints(
            const FontAtlas& font,
            const std::vector<char32_t>& codepoints,
            f32 scale)
        {
            f32 width = 0.0f;
            char32_t previous = 0;
            for (char32_t codepoint : codepoints)
            {
                if (previous != 0)
                {
                    width += PairAdjustment(font, previous, codepoint, scale);
                }
                width += AdvanceFor(font, codepoint, scale);
                previous = codepoint;
            }
            return width;
        }

        f32 AppendWidth(
            const PendingLine& line,
            const FontAtlas& font,
            char32_t codepoint,
            f32 scale)
        {
            f32 width = AdvanceFor(font, codepoint, scale);
            if (!line.codepoints.empty())
            {
                width += PairAdjustment(font, line.codepoints.back(), codepoint, scale);
            }
            return width;
        }

        void Append(PendingLine& line, const FontAtlas& font, char32_t codepoint, f32 scale)
        {
            line.width += AppendWidth(line, font, codepoint, scale);
            line.codepoints.push_back(codepoint);
        }

        void TrimTrailingSpaces(PendingLine& line, const FontAtlas& font, f32 scale)
        {
            while (!line.codepoints.empty() && line.codepoints.back() == U' ')
            {
                line.codepoints.pop_back();
            }
            line.width = MeasureCodepoints(font, line.codepoints, scale);
        }

        void AppendCharacterWrapped(
            std::vector<PendingLine>& lines,
            PendingLine& line,
            char32_t codepoint,
            const FontAtlas& font,
            const LayoutOptions& options)
        {
            const f32 added = AppendWidth(line, font, codepoint, options.scale);
            if (options.maximumWidth > 0.0f && !line.codepoints.empty() &&
                line.width + added > options.maximumWidth)
            {
                TrimTrailingSpaces(line, font, options.scale);
                lines.push_back(std::move(line));
                line = {};
            }
            Append(line, font, codepoint, options.scale);
        }

        std::vector<PendingLine> BuildLines(
            const FontAtlas& font,
            const std::vector<char32_t>& codepoints,
            const LayoutOptions& options)
        {
            std::vector<PendingLine> lines;
            PendingLine line;

            auto finishLine = [&]()
            {
                TrimTrailingSpaces(line, font, options.scale);
                lines.push_back(std::move(line));
                line = {};
            };

            std::size_t index = 0;
            while (index < codepoints.size())
            {
                const char32_t codepoint = codepoints[index];
                if (codepoint == U'\n')
                {
                    finishLine();
                    ++index;
                    continue;
                }

                if (options.wrap != WrapMode::Word || options.maximumWidth <= 0.0f)
                {
                    if (options.wrap == WrapMode::Character)
                    {
                        AppendCharacterWrapped(lines, line, codepoint, font, options);
                    }
                    else
                    {
                        Append(line, font, codepoint, options.scale);
                    }
                    ++index;
                    continue;
                }

                if (codepoint == U' ')
                {
                    if (!line.codepoints.empty())
                    {
                        const f32 added = AppendWidth(line, font, U' ', options.scale);
                        if (line.width + added <= options.maximumWidth)
                        {
                            Append(line, font, U' ', options.scale);
                        }
                    }
                    ++index;
                    continue;
                }

                std::size_t wordEnd = index;
                std::vector<char32_t> word;
                while (wordEnd < codepoints.size() && codepoints[wordEnd] != U' ' &&
                    codepoints[wordEnd] != U'\n')
                {
                    word.push_back(codepoints[wordEnd]);
                    ++wordEnd;
                }
                const f32 wordWidth = MeasureCodepoints(font, word, options.scale);
                f32 boundaryKerning = 0.0f;
                if (!line.codepoints.empty() && !word.empty())
                {
                    boundaryKerning = PairAdjustment(
                        font, line.codepoints.back(), word.front(), options.scale);
                }
                if (!line.codepoints.empty() &&
                    line.width + boundaryKerning + wordWidth > options.maximumWidth)
                {
                    finishLine();
                }

                if (wordWidth <= options.maximumWidth)
                {
                    for (; index < wordEnd; ++index)
                    {
                        Append(line, font, codepoints[index], options.scale);
                    }
                }
                else
                {
                    for (; index < wordEnd; ++index)
                    {
                        AppendCharacterWrapped(lines, line, codepoints[index], font, options);
                    }
                }
            }

            finishLine();
            if (lines.empty())
            {
                lines.push_back({});
            }
            return lines;
        }
    } // namespace

    bool LayoutOptions::IsValid() const
    {
        return std::isfinite(scale) && scale > 0.0f &&
            std::isfinite(maximumWidth) && maximumWidth >= 0.0f &&
            std::isfinite(lineSpacing) && lineSpacing >= 0.0f &&
            tabWidth > 0 &&
            (alignment == HorizontalAlignment::Left ||
             alignment == HorizontalAlignment::Center ||
             alignment == HorizontalAlignment::Right) &&
            (wrap == WrapMode::None || wrap == WrapMode::Character ||
             wrap == WrapMode::Word);
    }

    LayoutResult Layout(
        const FontAtlas& font,
        std::string_view utf8Text,
        const Math::Vec2& origin,
        const LayoutOptions& options)
    {
        LayoutResult result;
        if (!font.IsValid() || !options.IsValid() || !std::isfinite(origin.x) ||
            !std::isfinite(origin.y))
        {
            return result;
        }

        const DecodedText decoded = DecodeUtf8(utf8Text, options.tabWidth);
        result.invalidUtf8Sequences = decoded.invalidSequences;
        const std::vector<PendingLine> pendingLines =
            BuildLines(font, decoded.codepoints, options);
        result.lines.reserve(pendingLines.size());

        const f32 lineHeight = font.lineHeight * options.scale;
        const f32 alignmentWidth = options.maximumWidth > 0.0f
            ? options.maximumWidth
            : [&]()
              {
                  f32 widest = 0.0f;
                  for (const PendingLine& line : pendingLines)
                  {
                      widest = (std::max)(widest, line.width);
                  }
                  return widest;
              }();

        f32 y = origin.y;
        for (const PendingLine& line : pendingLines)
        {
            LineMetrics lineMetrics;
            lineMetrics.firstGlyph = static_cast<u32>(result.glyphs.size());
            lineMetrics.width = line.width;

            f32 x = origin.x;
            if (options.alignment == HorizontalAlignment::Center)
            {
                x += (alignmentWidth - line.width) * 0.5f;
            }
            else if (options.alignment == HorizontalAlignment::Right)
            {
                x += alignmentWidth - line.width;
            }

            char32_t previousCodepoint = 0;
            for (char32_t codepoint : line.codepoints)
            {
                if (!font.HasGlyph(codepoint))
                {
                    ++result.fallbackGlyphs;
                }
                if (previousCodepoint != 0)
                {
                    x += font.GetKerning(previousCodepoint, codepoint) * options.scale;
                }
                const Glyph& glyph = font.GetGlyph(codepoint);
                if (glyph.width > 0.0f && glyph.height > 0.0f)
                {
                    GlyphQuad quad;
                    quad.minimum = Math::Vec2(
                        x + glyph.bearingX * options.scale,
                        y + glyph.bearingY * options.scale);
                    quad.maximum = quad.minimum + Math::Vec2(
                        glyph.width * options.scale,
                        glyph.height * options.scale);
                    quad.uvMinimum = Math::Vec2(glyph.u0, glyph.v0);
                    quad.uvMaximum = Math::Vec2(glyph.u1, glyph.v1);
                    quad.codepoint = codepoint;
                    result.glyphs.push_back(quad);
                    ++lineMetrics.glyphCount;
                }
                x += glyph.advance * options.scale;
                previousCodepoint = codepoint;
            }

            result.metrics.width = (std::max)(result.metrics.width, line.width);
            result.lines.push_back(lineMetrics);
            y += lineHeight + options.lineSpacing;
        }

        result.metrics.lineCount = static_cast<u32>(pendingLines.size());
        result.metrics.height = result.metrics.lineCount == 0
            ? 0.0f
            : lineHeight * static_cast<f32>(result.metrics.lineCount) +
                options.lineSpacing * static_cast<f32>(result.metrics.lineCount - 1U);
        return result;
    }

    TextMetrics Measure(const FontAtlas& font, std::string_view text, f32 scale)
    {
        LayoutOptions options;
        options.scale = scale;
        return Layout(font, text, Math::Vec2::Zero, options).metrics;
    }

    void BuildGlyphQuads(
        const FontAtlas& font,
        std::string_view text,
        const Math::Vec2& origin,
        f32 scale,
        std::vector<GlyphQuad>& output)
    {
        LayoutOptions options;
        options.scale = scale;
        LayoutResult result = Layout(font, text, origin, options);
        output.insert(output.end(), result.glyphs.begin(), result.glyphs.end());
    }
} // namespace Pyramid::Text
