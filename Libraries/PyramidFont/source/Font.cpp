#include <Pyramid/Font/Font.hpp>
#include <tuple>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <set>
#include <unordered_set>
#include <utility>

namespace Pyramid::Font
{
    namespace
    {
        constexpr u32 MakeTag(char a, char b, char c, char d)
        {
            return (static_cast<u32>(static_cast<u8>(a)) << 24U) |
                (static_cast<u32>(static_cast<u8>(b)) << 16U) |
                (static_cast<u32>(static_cast<u8>(c)) << 8U) |
                static_cast<u32>(static_cast<u8>(d));
        }

        constexpr u32 kTagCmap = MakeTag('c', 'm', 'a', 'p');
        constexpr u32 kTagGlyf = MakeTag('g', 'l', 'y', 'f');
        constexpr u32 kTagHead = MakeTag('h', 'e', 'a', 'd');
        constexpr u32 kTagHhea = MakeTag('h', 'h', 'e', 'a');
        constexpr u32 kTagHmtx = MakeTag('h', 'm', 't', 'x');
        constexpr u32 kTagKern = MakeTag('k', 'e', 'r', 'n');
        constexpr u32 kTagLoca = MakeTag('l', 'o', 'c', 'a');
        constexpr u32 kTagMaxp = MakeTag('m', 'a', 'x', 'p');
        constexpr u32 kTagName = MakeTag('n', 'a', 'm', 'e');

        struct Table
        {
            std::size_t offset = 0;
            std::size_t length = 0;
        };

        class Reader final
        {
        public:
            Reader(const u8* data, std::size_t size, std::size_t base = 0)
                : m_data(data)
                , m_size(size)
                , m_base(base)
            {
            }

            [[nodiscard]] bool CanRead(std::size_t offset, std::size_t count) const
            {
                return offset <= m_size && count <= m_size - offset;
            }

            [[nodiscard]] bool U8(std::size_t offset, u8& value) const
            {
                if (!CanRead(offset, 1))
                {
                    return false;
                }
                value = m_data[offset];
                return true;
            }

            [[nodiscard]] bool U16(std::size_t offset, u16& value) const
            {
                if (!CanRead(offset, 2))
                {
                    return false;
                }
                value = static_cast<u16>(
                    (static_cast<u16>(m_data[offset]) << 8U) |
                    static_cast<u16>(m_data[offset + 1]));
                return true;
            }

            [[nodiscard]] bool I16(std::size_t offset, i16& value) const
            {
                u16 raw = 0;
                if (!U16(offset, raw))
                {
                    return false;
                }
                value = static_cast<i16>(raw);
                return true;
            }

            [[nodiscard]] bool U32(std::size_t offset, u32& value) const
            {
                if (!CanRead(offset, 4))
                {
                    return false;
                }
                value = (static_cast<u32>(m_data[offset]) << 24U) |
                    (static_cast<u32>(m_data[offset + 1]) << 16U) |
                    (static_cast<u32>(m_data[offset + 2]) << 8U) |
                    static_cast<u32>(m_data[offset + 3]);
                return true;
            }

            [[nodiscard]] const u8* Data(std::size_t offset) const
            {
                return CanRead(offset, 0) ? m_data + offset : nullptr;
            }

            [[nodiscard]] std::size_t Size() const { return m_size; }
            [[nodiscard]] std::size_t Absolute(std::size_t offset) const
            {
                return m_base + offset;
            }

        private:
            const u8* m_data = nullptr;
            std::size_t m_size = 0;
            std::size_t m_base = 0;
        };

        void AddDiagnostic(
            std::vector<Diagnostic>& diagnostics,
            DiagnosticSeverity severity,
            std::string message,
            std::size_t offset = 0)
        {
            diagnostics.push_back({severity, std::move(message), offset});
        }

        bool ReadFile(std::string_view path, std::vector<u8>& bytes, std::string& error)
        {
            std::ifstream stream(std::string(path), std::ios::binary | std::ios::ate);
            if (!stream)
            {
                error = "unable to open file";
                return false;
            }
            const std::streamoff end = stream.tellg();
            if (end < 0)
            {
                error = "unable to determine file size";
                return false;
            }
            const auto size = static_cast<std::size_t>(end);
            bytes.resize(size);
            stream.seekg(0, std::ios::beg);
            if (size > 0 && !stream.read(reinterpret_cast<char*>(bytes.data()), end))
            {
                error = "unable to read complete file";
                return false;
            }
            return true;
        }

        bool WriteFile(std::string_view path, const std::vector<u8>& bytes, std::string& error)
        {
            std::ofstream stream(std::string(path), std::ios::binary | std::ios::trunc);
            if (!stream)
            {
                error = "unable to create file";
                return false;
            }
            if (!bytes.empty())
            {
                stream.write(
                    reinterpret_cast<const char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
            }
            if (!stream)
            {
                error = "unable to write complete file";
                return false;
            }
            return true;
        }

        std::string DecodeUtf16Be(const u8* data, std::size_t size)
        {
            std::string output;
            for (std::size_t i = 0; i + 1 < size; i += 2)
            {
                const u16 value = static_cast<u16>(
                    (static_cast<u16>(data[i]) << 8U) |
                    static_cast<u16>(data[i + 1]));
                char32_t codepoint = value;
                if (value >= 0xD800U && value <= 0xDBFFU && i + 3 < size)
                {
                    const u16 low = static_cast<u16>(
                        (static_cast<u16>(data[i + 2]) << 8U) |
                        static_cast<u16>(data[i + 3]));
                    if (low >= 0xDC00U && low <= 0xDFFFU)
                    {
                        codepoint = 0x10000U +
                            ((static_cast<char32_t>(value - 0xD800U) << 10U) |
                             static_cast<char32_t>(low - 0xDC00U));
                        i += 2;
                    }
                }

                if (codepoint <= 0x7FU)
                {
                    output.push_back(static_cast<char>(codepoint));
                }
                else if (codepoint <= 0x7FFU)
                {
                    output.push_back(static_cast<char>(0xC0U | (codepoint >> 6U)));
                    output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
                }
                else if (codepoint <= 0xFFFFU)
                {
                    output.push_back(static_cast<char>(0xE0U | (codepoint >> 12U)));
                    output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
                    output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
                }
                else
                {
                    output.push_back(static_cast<char>(0xF0U | (codepoint >> 18U)));
                    output.push_back(static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3FU)));
                    output.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3FU)));
                    output.push_back(static_cast<char>(0x80U | (codepoint & 0x3FU)));
                }
            }
            return output;
        }

        bool ParseName(
            const Reader& reader,
            const Table& table,
            std::string& family,
            std::vector<Diagnostic>& diagnostics)
        {
            if (table.length < 6 || !reader.CanRead(table.offset, table.length))
            {
                AddDiagnostic(diagnostics, DiagnosticSeverity::Warning, "invalid name table", table.offset);
                return false;
            }
            Reader name(reader.Data(table.offset), table.length, table.offset);
            u16 count = 0;
            u16 stringOffset = 0;
            if (!name.U16(2, count) || !name.U16(4, stringOffset) ||
                !name.CanRead(6, static_cast<std::size_t>(count) * 12U))
            {
                AddDiagnostic(diagnostics, DiagnosticSeverity::Warning, "truncated name table", table.offset);
                return false;
            }

            int bestScore = -1;
            for (u16 i = 0; i < count; ++i)
            {
                const std::size_t record = 6U + static_cast<std::size_t>(i) * 12U;
                u16 platform = 0;
                u16 language = 0;
                u16 nameId = 0;
                u16 length = 0;
                u16 offset = 0;
                if (!name.U16(record, platform) || !name.U16(record + 4, language) ||
                    !name.U16(record + 6, nameId) || !name.U16(record + 8, length) ||
                    !name.U16(record + 10, offset))
                {
                    continue;
                }
                if (nameId != 1 && nameId != 16)
                {
                    continue;
                }
                const std::size_t valueOffset = static_cast<std::size_t>(stringOffset) + offset;
                if (!name.CanRead(valueOffset, length))
                {
                    continue;
                }
                int score = nameId == 16 ? 20 : 10;
                if (platform == 3)
                {
                    score += 5;
                    if (language == 0x0409)
                    {
                        score += 2;
                    }
                }
                if (score <= bestScore)
                {
                    continue;
                }
                std::string decoded;
                if (platform == 0 || platform == 3)
                {
                    decoded = DecodeUtf16Be(name.Data(valueOffset), length);
                }
                else
                {
                    decoded.assign(
                        reinterpret_cast<const char*>(name.Data(valueOffset)),
                        length);
                }
                if (!decoded.empty())
                {
                    bestScore = score;
                    family = std::move(decoded);
                }
            }
            return !family.empty();
        }

        bool ParseCmapFormat12(
            const Reader& cmap,
            std::size_t offset,
            u32 glyphCount,
            const LoadOptions& options,
            std::unordered_map<char32_t, GlyphId>& mappings)
        {
            u32 length = 0;
            u32 groups = 0;
            if (!cmap.U32(offset + 4, length) || !cmap.U32(offset + 12, groups) ||
                length < 16 || !cmap.CanRead(offset, length) ||
                groups > options.maximumMappings)
            {
                return false;
            }
            if (!cmap.CanRead(offset + 16, static_cast<std::size_t>(groups) * 12U))
            {
                return false;
            }
            for (u32 i = 0; i < groups; ++i)
            {
                const std::size_t groupOffset = offset + 16U + static_cast<std::size_t>(i) * 12U;
                u32 start = 0;
                u32 end = 0;
                u32 startGlyph = 0;
                if (!cmap.U32(groupOffset, start) || !cmap.U32(groupOffset + 4, end) ||
                    !cmap.U32(groupOffset + 8, startGlyph) || start > end || end > 0x10FFFFU)
                {
                    return false;
                }
                const u64 span = static_cast<u64>(end) - start + 1U;
                if (span > options.maximumMappings ||
                    mappings.size() + static_cast<std::size_t>(span) > options.maximumMappings)
                {
                    return false;
                }
                for (u32 cp = start; cp <= end; ++cp)
                {
                    const u64 glyph = static_cast<u64>(startGlyph) + (cp - start);
                    if (glyph < glyphCount)
                    {
                        mappings[static_cast<char32_t>(cp)] = static_cast<GlyphId>(glyph);
                    }
                    if (cp == end)
                    {
                        break;
                    }
                }
            }
            return true;
        }

        bool ParseCmapFormat4(
            const Reader& cmap,
            std::size_t offset,
            u32 glyphCount,
            const LoadOptions& options,
            std::unordered_map<char32_t, GlyphId>& mappings)
        {
            u16 length = 0;
            u16 segCountX2 = 0;
            if (!cmap.U16(offset + 2, length) || !cmap.U16(offset + 6, segCountX2) ||
                length < 16 || (segCountX2 & 1U) != 0 || !cmap.CanRead(offset, length))
            {
                return false;
            }
            const u16 segCount = static_cast<u16>(segCountX2 / 2U);
            const std::size_t endCodes = offset + 14U;
            const std::size_t startCodes = endCodes + static_cast<std::size_t>(segCount) * 2U + 2U;
            const std::size_t deltas = startCodes + static_cast<std::size_t>(segCount) * 2U;
            const std::size_t ranges = deltas + static_cast<std::size_t>(segCount) * 2U;
            if (!cmap.CanRead(ranges, static_cast<std::size_t>(segCount) * 2U))
            {
                return false;
            }

            for (u16 i = 0; i < segCount; ++i)
            {
                u16 start = 0;
                u16 end = 0;
                i16 delta = 0;
                u16 range = 0;
                if (!cmap.U16(startCodes + static_cast<std::size_t>(i) * 2U, start) ||
                    !cmap.U16(endCodes + static_cast<std::size_t>(i) * 2U, end) ||
                    !cmap.I16(deltas + static_cast<std::size_t>(i) * 2U, delta) ||
                    !cmap.U16(ranges + static_cast<std::size_t>(i) * 2U, range) || start > end)
                {
                    return false;
                }
                for (u32 cp = start; cp <= end; ++cp)
                {
                    if (cp == 0xFFFFU)
                    {
                        break;
                    }
                    u16 glyph = 0;
                    if (range == 0)
                    {
                        glyph = static_cast<u16>((cp + static_cast<i32>(delta)) & 0xFFFF);
                    }
                    else
                    {
                        const std::size_t rangeWord = ranges + static_cast<std::size_t>(i) * 2U;
                        const std::size_t glyphOffset = rangeWord + range + (cp - start) * 2U;
                        if (!cmap.U16(glyphOffset, glyph))
                        {
                            return false;
                        }
                        if (glyph != 0)
                        {
                            glyph = static_cast<u16>((glyph + static_cast<i32>(delta)) & 0xFFFF);
                        }
                    }
                    if (glyph < glyphCount)
                    {
                        mappings[static_cast<char32_t>(cp)] = glyph;
                        if (mappings.size() > options.maximumMappings)
                        {
                            return false;
                        }
                    }
                    if (cp == end)
                    {
                        break;
                    }
                }
            }
            return true;
        }

        bool ParseCmap(
            const Reader& reader,
            const Table& table,
            u32 glyphCount,
            const LoadOptions& options,
            std::unordered_map<char32_t, GlyphId>& mappings,
            std::vector<Diagnostic>& diagnostics)
        {
            if (table.length < 4 || !reader.CanRead(table.offset, table.length))
            {
                AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "missing or invalid cmap table", table.offset);
                return false;
            }
            Reader cmap(reader.Data(table.offset), table.length, table.offset);
            u16 count = 0;
            if (!cmap.U16(2, count) || !cmap.CanRead(4, static_cast<std::size_t>(count) * 8U))
            {
                AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "truncated cmap encoding records", table.offset);
                return false;
            }

            struct Candidate
            {
                int score = 0;
                std::size_t offset = 0;
                u16 format = 0;
            };
            std::vector<Candidate> candidates;
            for (u16 i = 0; i < count; ++i)
            {
                const std::size_t record = 4U + static_cast<std::size_t>(i) * 8U;
                u16 platform = 0;
                u16 encoding = 0;
                u32 subOffset = 0;
                if (!cmap.U16(record, platform) || !cmap.U16(record + 2, encoding) ||
                    !cmap.U32(record + 4, subOffset) || !cmap.CanRead(subOffset, 2))
                {
                    continue;
                }
                u16 format = 0;
                if (!cmap.U16(subOffset, format) || (format != 4 && format != 12))
                {
                    continue;
                }
                int score = format == 12 ? 100 : 50;
                if (platform == 3 && encoding == 10)
                {
                    score += 20;
                }
                else if (platform == 0)
                {
                    score += 15;
                }
                else if (platform == 3 && encoding == 1)
                {
                    score += 10;
                }
                candidates.push_back({score, subOffset, format});
            }
            std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b)
            {
                return a.score > b.score;
            });
            for (const Candidate& candidate : candidates)
            {
                std::unordered_map<char32_t, GlyphId> parsed;
                const bool okay = candidate.format == 12
                    ? ParseCmapFormat12(cmap, candidate.offset, glyphCount, options, parsed)
                    : ParseCmapFormat4(cmap, candidate.offset, glyphCount, options, parsed);
                if (okay && !parsed.empty())
                {
                    mappings = std::move(parsed);
                    return true;
                }
            }
            AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "no supported cmap format 4 or 12 subtable", table.offset);
            return false;
        }

        struct GlyphParser
        {
            const Reader& reader;
            Table glyf;
            const std::vector<u32>& locations;
            u32 maximumDepth = 32;
            std::vector<GlyphOutline> cache;
            std::vector<u8> state;
            std::vector<Diagnostic>& diagnostics;

            bool Parse(GlyphId glyphId, u32 depth)
            {
                if (glyphId >= cache.size())
                {
                    return false;
                }
                if (state[glyphId] == 2)
                {
                    return true;
                }
                if (state[glyphId] == 1 || depth > maximumDepth)
                {
                    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "cyclic or excessively deep compound glyph", glyf.offset);
                    return false;
                }
                state[glyphId] = 1;
                const u32 begin = locations[glyphId];
                const u32 end = locations[static_cast<std::size_t>(glyphId) + 1U];
                if (begin > end || end > glyf.length)
                {
                    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "glyph location outside glyf table", glyf.offset + begin);
                    return false;
                }
                GlyphOutline outline;
                if (begin == end)
                {
                    cache[glyphId] = std::move(outline);
                    state[glyphId] = 2;
                    return true;
                }
                Reader glyph(reader.Data(glyf.offset + begin), end - begin, glyf.offset + begin);
                i16 contourCount = 0;
                if (!glyph.I16(0, contourCount) || !glyph.I16(2, outline.xMin) ||
                    !glyph.I16(4, outline.yMin) || !glyph.I16(6, outline.xMax) ||
                    !glyph.I16(8, outline.yMax))
                {
                    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "truncated glyph header", glyf.offset + begin);
                    return false;
                }
                bool okay = contourCount >= 0
                    ? ParseSimple(glyph, static_cast<u16>(contourCount), outline)
                    : ParseCompound(glyph, outline, depth);
                if (!okay)
                {
                    return false;
                }
                cache[glyphId] = std::move(outline);
                state[glyphId] = 2;
                return true;
            }

            bool ParseSimple(const Reader& glyph, u16 contourCount, GlyphOutline& outline)
            {
                std::size_t cursor = 10;
                std::vector<u16> ends(contourCount);
                for (u16 i = 0; i < contourCount; ++i)
                {
                    if (!glyph.U16(cursor, ends[i]) || (i > 0 && ends[i] <= ends[i - 1]))
                    {
                        AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "invalid simple glyph contour endpoints", glyph.Absolute(cursor));
                        return false;
                    }
                    cursor += 2;
                }
                u16 instructionLength = 0;
                if (!glyph.U16(cursor, instructionLength))
                {
                    return false;
                }
                cursor += 2;
                if (!glyph.CanRead(cursor, instructionLength))
                {
                    return false;
                }
                cursor += instructionLength;
                const u32 pointCount = contourCount == 0 ? 0U : static_cast<u32>(ends.back()) + 1U;
                if (pointCount > 1000000U)
                {
                    return false;
                }
                std::vector<u8> flags;
                flags.reserve(pointCount);
                while (flags.size() < pointCount)
                {
                    u8 flag = 0;
                    if (!glyph.U8(cursor++, flag))
                    {
                        return false;
                    }
                    flags.push_back(flag);
                    if ((flag & 0x08U) != 0)
                    {
                        u8 repeat = 0;
                        if (!glyph.U8(cursor++, repeat) || flags.size() + repeat > pointCount)
                        {
                            return false;
                        }
                        flags.insert(flags.end(), repeat, flag);
                    }
                }

                std::vector<i32> xs(pointCount, 0);
                std::vector<i32> ys(pointCount, 0);
                i32 x = 0;
                for (u32 i = 0; i < pointCount; ++i)
                {
                    const u8 flag = flags[i];
                    if ((flag & 0x02U) != 0)
                    {
                        u8 delta = 0;
                        if (!glyph.U8(cursor++, delta))
                        {
                            return false;
                        }
                        x += (flag & 0x10U) != 0 ? delta : -static_cast<i32>(delta);
                    }
                    else if ((flag & 0x10U) == 0)
                    {
                        i16 delta = 0;
                        if (!glyph.I16(cursor, delta))
                        {
                            return false;
                        }
                        cursor += 2;
                        x += delta;
                    }
                    xs[i] = x;
                }
                i32 y = 0;
                for (u32 i = 0; i < pointCount; ++i)
                {
                    const u8 flag = flags[i];
                    if ((flag & 0x04U) != 0)
                    {
                        u8 delta = 0;
                        if (!glyph.U8(cursor++, delta))
                        {
                            return false;
                        }
                        y += (flag & 0x20U) != 0 ? delta : -static_cast<i32>(delta);
                    }
                    else if ((flag & 0x20U) == 0)
                    {
                        i16 delta = 0;
                        if (!glyph.I16(cursor, delta))
                        {
                            return false;
                        }
                        cursor += 2;
                        y += delta;
                    }
                    ys[i] = y;
                }

                outline.contours.clear();
                outline.contours.reserve(contourCount);
                u32 first = 0;
                for (u16 contour = 0; contour < contourCount; ++contour)
                {
                    const u32 last = ends[contour];
                    OutlineContour output;
                    output.points.reserve(last - first + 1U);
                    for (u32 i = first; i <= last; ++i)
                    {
                        output.points.push_back({
                            static_cast<f32>(xs[i]),
                            static_cast<f32>(ys[i]),
                            (flags[i] & 0x01U) != 0});
                    }
                    outline.contours.push_back(std::move(output));
                    first = last + 1U;
                }
                return true;
            }

            static OutlinePoint Transform(
                const OutlinePoint& point,
                f32 a,
                f32 b,
                f32 c,
                f32 d,
                f32 tx,
                f32 ty)
            {
                return {
                    a * point.x + c * point.y + tx,
                    b * point.x + d * point.y + ty,
                    point.onCurve};
            }

            static std::vector<OutlinePoint> FlattenPoints(const GlyphOutline& outline)
            {
                std::vector<OutlinePoint> points;
                for (const OutlineContour& contour : outline.contours)
                {
                    points.insert(points.end(), contour.points.begin(), contour.points.end());
                }
                return points;
            }

            bool ParseCompound(const Reader& glyph, GlyphOutline& outline, u32 depth)
            {
                constexpr u16 ArgWords = 0x0001;
                constexpr u16 ArgsXY = 0x0002;
                constexpr u16 HaveScale = 0x0008;
                constexpr u16 More = 0x0020;
                constexpr u16 HaveXYScale = 0x0040;
                constexpr u16 HaveMatrix = 0x0080;
                constexpr u16 HaveInstructions = 0x0100;

                std::size_t cursor = 10;
                u16 flags = 0;
                do
                {
                    GlyphId componentId = 0;
                    if (!glyph.U16(cursor, flags) || !glyph.U16(cursor + 2, componentId))
                    {
                        return false;
                    }
                    cursor += 4;
                    i32 arg1 = 0;
                    i32 arg2 = 0;
                    if ((flags & ArgWords) != 0)
                    {
                        i16 a = 0;
                        i16 b = 0;
                        if (!glyph.I16(cursor, a) || !glyph.I16(cursor + 2, b))
                        {
                            return false;
                        }
                        arg1 = a;
                        arg2 = b;
                        cursor += 4;
                    }
                    else
                    {
                        u8 rawA = 0;
                        u8 rawB = 0;
                        if (!glyph.U8(cursor, rawA) || !glyph.U8(cursor + 1, rawB))
                        {
                            return false;
                        }
                        if ((flags & ArgsXY) != 0)
                        {
                            arg1 = static_cast<i8>(rawA);
                            arg2 = static_cast<i8>(rawB);
                        }
                        else
                        {
                            arg1 = rawA;
                            arg2 = rawB;
                        }
                        cursor += 2;
                    }

                    f32 a = 1.0f;
                    f32 b = 0.0f;
                    f32 c = 0.0f;
                    f32 d = 1.0f;
                    auto readF2 = [&](f32& value)
                    {
                        i16 raw = 0;
                        if (!glyph.I16(cursor, raw))
                        {
                            return false;
                        }
                        cursor += 2;
                        value = static_cast<f32>(raw) / 16384.0f;
                        return true;
                    };
                    if ((flags & HaveScale) != 0)
                    {
                        if (!readF2(a))
                        {
                            return false;
                        }
                        d = a;
                    }
                    else if ((flags & HaveXYScale) != 0)
                    {
                        if (!readF2(a) || !readF2(d))
                        {
                            return false;
                        }
                    }
                    else if ((flags & HaveMatrix) != 0)
                    {
                        if (!readF2(a) || !readF2(b) || !readF2(c) || !readF2(d))
                        {
                            return false;
                        }
                    }

                    if (!Parse(componentId, depth + 1U))
                    {
                        return false;
                    }
                    const GlyphOutline& component = cache[componentId];
                    f32 tx = 0.0f;
                    f32 ty = 0.0f;
                    if ((flags & ArgsXY) != 0)
                    {
                        tx = static_cast<f32>(arg1);
                        ty = static_cast<f32>(arg2);
                    }
                    else
                    {
                        const std::vector<OutlinePoint> parentPoints = FlattenPoints(outline);
                        const std::vector<OutlinePoint> componentPoints = FlattenPoints(component);
                        if (arg1 < 0 || arg2 < 0 ||
                            static_cast<std::size_t>(arg1) >= parentPoints.size() ||
                            static_cast<std::size_t>(arg2) >= componentPoints.size())
                        {
                            AddDiagnostic(diagnostics, DiagnosticSeverity::Error, "compound glyph point attachment is out of range", glyph.Absolute(cursor));
                            return false;
                        }
                        const OutlinePoint transformed = Transform(
                            componentPoints[static_cast<std::size_t>(arg2)],
                            a, b, c, d, 0.0f, 0.0f);
                        tx = parentPoints[static_cast<std::size_t>(arg1)].x - transformed.x;
                        ty = parentPoints[static_cast<std::size_t>(arg1)].y - transformed.y;
                    }

                    for (const OutlineContour& contour : component.contours)
                    {
                        OutlineContour transformed;
                        transformed.points.reserve(contour.points.size());
                        for (const OutlinePoint& point : contour.points)
                        {
                            transformed.points.push_back(Transform(point, a, b, c, d, tx, ty));
                        }
                        outline.contours.push_back(std::move(transformed));
                    }
                } while ((flags & More) != 0);

                if ((flags & HaveInstructions) != 0)
                {
                    u16 length = 0;
                    if (!glyph.U16(cursor, length) || !glyph.CanRead(cursor + 2, length))
                    {
                        return false;
                    }
                }
                return true;
            }
        };

        bool ParseKerning(
            const Reader& reader,
            const Table& table,
            u32 glyphCount,
            std::vector<KerningPair>& pairs)
        {
            if (table.length < 4 || !reader.CanRead(table.offset, table.length))
            {
                return false;
            }
            Reader kern(reader.Data(table.offset), table.length, table.offset);
            u16 version = 0;
            u16 count = 0;
            if (!kern.U16(0, version) || !kern.U16(2, count) || version != 0)
            {
                return false;
            }
            std::size_t cursor = 4;
            for (u16 tableIndex = 0; tableIndex < count; ++tableIndex)
            {
                u16 length = 0;
                u16 coverage = 0;
                if (!kern.U16(cursor + 2, length) || !kern.U16(cursor + 4, coverage) ||
                    length < 6 || !kern.CanRead(cursor, length))
                {
                    return false;
                }
                const u16 format = static_cast<u16>(coverage >> 8U);
                if (format == 0 && length >= 14)
                {
                    u16 pairCount = 0;
                    if (!kern.U16(cursor + 6, pairCount) ||
                        !kern.CanRead(cursor + 14, static_cast<std::size_t>(pairCount) * 6U))
                    {
                        return false;
                    }
                    for (u16 pairIndex = 0; pairIndex < pairCount; ++pairIndex)
                    {
                        const std::size_t pairOffset = cursor + 14U + static_cast<std::size_t>(pairIndex) * 6U;
                        u16 left = 0;
                        u16 right = 0;
                        i16 value = 0;
                        if (!kern.U16(pairOffset, left) || !kern.U16(pairOffset + 2, right) ||
                            !kern.I16(pairOffset + 4, value))
                        {
                            return false;
                        }
                        if (left < glyphCount && right < glyphCount && value != 0)
                        {
                            pairs.push_back({left, right, value});
                        }
                    }
                }
                cursor += length;
            }
            std::sort(pairs.begin(), pairs.end(), [](const KerningPair& a, const KerningPair& b)
            {
                return std::tie(a.left, a.right) < std::tie(b.left, b.right);
            });
            return true;
        }

        struct Point2
        {
            f32 x = 0.0f;
            f32 y = 0.0f;
        };

        Point2 Midpoint(const Point2& a, const Point2& b)
        {
            return {(a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f};
        }

        void AppendQuadratic(
            std::vector<Point2>& output,
            const Point2& p0,
            const Point2& p1,
            const Point2& p2)
        {
            const f32 curvature =
                std::abs(p0.x - 2.0f * p1.x + p2.x) +
                std::abs(p0.y - 2.0f * p1.y + p2.y);
            const u32 steps = (std::max)(4U, (std::min)(32U, static_cast<u32>(curvature / 32.0f) + 4U));
            for (u32 step = 1; step <= steps; ++step)
            {
                const f32 t = static_cast<f32>(step) / static_cast<f32>(steps);
                const f32 inverse = 1.0f - t;
                output.push_back({
                    inverse * inverse * p0.x + 2.0f * inverse * t * p1.x + t * t * p2.x,
                    inverse * inverse * p0.y + 2.0f * inverse * t * p1.y + t * t * p2.y});
            }
        }

        std::vector<std::vector<Point2>> FlattenOutline(const GlyphOutline& outline)
        {
            std::vector<std::vector<Point2>> polygons;
            polygons.reserve(outline.contours.size());
            for (const OutlineContour& contour : outline.contours)
            {
                if (contour.points.empty())
                {
                    continue;
                }
                const std::size_t count = contour.points.size();
                const OutlinePoint& first = contour.points.front();
                const OutlinePoint& last = contour.points.back();
                Point2 start;
                std::size_t index = 0;
                if (first.onCurve)
                {
                    start = {first.x, first.y};
                    index = 1;
                }
                else if (last.onCurve)
                {
                    start = {last.x, last.y};
                }
                else
                {
                    start = Midpoint({last.x, last.y}, {first.x, first.y});
                }

                std::vector<Point2> polygon;
                polygon.push_back(start);
                Point2 current = start;
                std::size_t consumed = 0;
                while (consumed < count)
                {
                    const OutlinePoint& point = contour.points[index % count];
                    if (point.onCurve)
                    {
                        current = {point.x, point.y};
                        polygon.push_back(current);
                        ++index;
                        ++consumed;
                        continue;
                    }
                    const OutlinePoint& next = contour.points[(index + 1U) % count];
                    Point2 end;
                    if (next.onCurve)
                    {
                        end = {next.x, next.y};
                        index += 2U;
                        consumed += 2U;
                    }
                    else
                    {
                        end = Midpoint({point.x, point.y}, {next.x, next.y});
                        ++index;
                        ++consumed;
                    }
                    AppendQuadratic(polygon, current, {point.x, point.y}, end);
                    current = end;
                }
                if (polygon.size() > 1 &&
                    (polygon.back().x != polygon.front().x || polygon.back().y != polygon.front().y))
                {
                    polygon.push_back(polygon.front());
                }
                if (polygon.size() >= 3)
                {
                    polygons.push_back(std::move(polygon));
                }
            }
            return polygons;
        }

        bool PointInside(const std::vector<std::vector<Point2>>& polygons, f32 x, f32 y)
        {
            i32 winding = 0;
            for (const auto& polygon : polygons)
            {
                for (std::size_t i = 1; i < polygon.size(); ++i)
                {
                    const Point2& a = polygon[i - 1];
                    const Point2& b = polygon[i];
                    if (a.y <= y)
                    {
                        if (b.y > y && (b.x - a.x) * (y - a.y) - (x - a.x) * (b.y - a.y) > 0.0f)
                        {
                            ++winding;
                        }
                    }
                    else if (b.y <= y &&
                        (b.x - a.x) * (y - a.y) - (x - a.x) * (b.y - a.y) < 0.0f)
                    {
                        --winding;
                    }
                }
            }
            return winding != 0;
        }

        void PutU16(std::vector<u8>& output, u16 value)
        {
            output.push_back(static_cast<u8>(value & 0xFFU));
            output.push_back(static_cast<u8>((value >> 8U) & 0xFFU));
        }

        void PutU32(std::vector<u8>& output, u32 value)
        {
            for (u32 shift = 0; shift < 32; shift += 8)
            {
                output.push_back(static_cast<u8>((value >> shift) & 0xFFU));
            }
        }

        void PutF32(std::vector<u8>& output, f32 value)
        {
            u32 bits = 0;
            static_assert(sizeof(bits) == sizeof(value), "unexpected float size");
            std::memcpy(&bits, &value, sizeof(bits));
            PutU32(output, bits);
        }

        u32 HashBytes(const u8* data, std::size_t size)
        {
            u32 hash = 2166136261U;
            for (std::size_t i = 0; i < size; ++i)
            {
                hash ^= data[i];
                hash *= 16777619U;
            }
            return hash;
        }

        class LittleReader final
        {
        public:
            LittleReader(const u8* data, std::size_t size)
                : m_data(data), m_size(size) {}

            bool U16(std::size_t& cursor, u16& value) const
            {
                if (cursor > m_size || m_size - cursor < 2)
                {
                    return false;
                }
                value = static_cast<u16>(m_data[cursor]) |
                    static_cast<u16>(static_cast<u16>(m_data[cursor + 1]) << 8U);
                cursor += 2;
                return true;
            }

            bool U32(std::size_t& cursor, u32& value) const
            {
                if (cursor > m_size || m_size - cursor < 4)
                {
                    return false;
                }
                value = static_cast<u32>(m_data[cursor]) |
                    (static_cast<u32>(m_data[cursor + 1]) << 8U) |
                    (static_cast<u32>(m_data[cursor + 2]) << 16U) |
                    (static_cast<u32>(m_data[cursor + 3]) << 24U);
                cursor += 4;
                return true;
            }

            bool F32(std::size_t& cursor, f32& value) const
            {
                u32 bits = 0;
                if (!U32(cursor, bits))
                {
                    return false;
                }
                std::memcpy(&value, &bits, sizeof(value));
                return std::isfinite(value);
            }

            bool Bytes(std::size_t& cursor, std::size_t count, const u8*& data) const
            {
                if (cursor > m_size || count > m_size - cursor)
                {
                    return false;
                }
                data = m_data + cursor;
                cursor += count;
                return true;
            }

        private:
            const u8* m_data = nullptr;
            std::size_t m_size = 0;
        };
    } // namespace

    bool FontFace::IsValid() const
    {
        return unitsPerEm > 0 && !metrics.empty() && metrics.size() == outlines.size() &&
            !characterMap.empty();
    }

    GlyphId FontFace::GetGlyphId(char32_t codepoint) const
    {
        const auto found = characterMap.find(codepoint);
        return found == characterMap.end() ? 0 : found->second;
    }

    i16 FontFace::GetKerning(GlyphId left, GlyphId right) const
    {
        const auto found = std::lower_bound(
            kerning.begin(), kerning.end(), std::pair<GlyphId, GlyphId>{left, right},
            [](const KerningPair& pair, const std::pair<GlyphId, GlyphId>& key)
            {
                return std::tie(pair.left, pair.right) < std::tie(key.first, key.second);
            });
        return found != kerning.end() && found->left == left && found->right == right
            ? found->value
            : 0;
    }

    LoadResult LoadTrueType(const u8* data, std::size_t size, const LoadOptions& options)
    {
        LoadResult result;
        if (!data || size < 12 || size > options.maximumFileBytes || options.maximumTables == 0 ||
            options.maximumGlyphs == 0 || options.maximumMappings == 0)
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "invalid TrueType input or limits");
            return result;
        }
        Reader reader(data, size);
        u32 scaler = 0;
        u16 tableCount = 0;
        if (!reader.U32(0, scaler) || !reader.U16(4, tableCount) ||
            (scaler != 0x00010000U && scaler != MakeTag('t', 'r', 'u', 'e')) ||
            tableCount == 0 || tableCount > options.maximumTables ||
            !reader.CanRead(12, static_cast<std::size_t>(tableCount) * 16U))
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "unsupported or malformed SFNT directory");
            return result;
        }

        std::unordered_map<u32, Table> tables;
        for (u16 i = 0; i < tableCount; ++i)
        {
            const std::size_t entry = 12U + static_cast<std::size_t>(i) * 16U;
            u32 tag = 0;
            u32 offset = 0;
            u32 length = 0;
            if (!reader.U32(entry, tag) || !reader.U32(entry + 8, offset) ||
                !reader.U32(entry + 12, length) || !reader.CanRead(offset, length))
            {
                AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "font table lies outside the file", entry);
                return result;
            }
            if (!tables.emplace(tag, Table{offset, length}).second)
            {
                AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "duplicate SFNT table", entry);
                return result;
            }
        }

        auto require = [&](u32 tag) -> const Table*
        {
            const auto found = tables.find(tag);
            return found == tables.end() ? nullptr : &found->second;
        };
        const Table* head = require(kTagHead);
        const Table* hhea = require(kTagHhea);
        const Table* maxp = require(kTagMaxp);
        const Table* hmtx = require(kTagHmtx);
        const Table* loca = require(kTagLoca);
        const Table* glyf = require(kTagGlyf);
        const Table* cmap = require(kTagCmap);
        if (!head || !hhea || !maxp || !hmtx || !loca || !glyf || !cmap ||
            head->length < 54 || hhea->length < 36 || maxp->length < 6)
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "required TrueType table is missing or truncated");
            return result;
        }

        u16 unitsPerEm = 0;
        i16 indexFormat = 0;
        i16 ascender = 0;
        i16 descender = 0;
        i16 lineGap = 0;
        u16 metricCount = 0;
        u16 glyphCount = 0;
        if (!reader.U16(head->offset + 18, unitsPerEm) ||
            !reader.I16(head->offset + 50, indexFormat) ||
            !reader.I16(hhea->offset + 4, ascender) ||
            !reader.I16(hhea->offset + 6, descender) ||
            !reader.I16(hhea->offset + 8, lineGap) ||
            !reader.U16(hhea->offset + 34, metricCount) ||
            !reader.U16(maxp->offset + 4, glyphCount) || unitsPerEm < 16 ||
            glyphCount == 0 || glyphCount > options.maximumGlyphs ||
            metricCount == 0 || metricCount > glyphCount ||
            (indexFormat != 0 && indexFormat != 1))
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "invalid TrueType metrics header");
            return result;
        }

        FontFace face;
        face.unitsPerEm = unitsPerEm;
        face.ascender = ascender;
        face.descender = descender;
        face.lineGap = lineGap;
        face.metrics.resize(glyphCount);
        std::size_t metricCursor = hmtx->offset;
        u16 lastAdvance = 0;
        for (u16 glyph = 0; glyph < glyphCount; ++glyph)
        {
            i16 bearing = 0;
            if (glyph < metricCount)
            {
                if (!reader.U16(metricCursor, lastAdvance) || !reader.I16(metricCursor + 2, bearing))
                {
                    AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "truncated hmtx table", metricCursor);
                    return result;
                }
                metricCursor += 4;
            }
            else
            {
                if (!reader.I16(metricCursor, bearing))
                {
                    AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "truncated hmtx side bearings", metricCursor);
                    return result;
                }
                metricCursor += 2;
            }
            face.metrics[glyph] = {lastAdvance, bearing};
        }
        if (metricCursor > hmtx->offset + hmtx->length)
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "hmtx data exceeds table bounds", metricCursor);
            return result;
        }

        std::vector<u32> locations(static_cast<std::size_t>(glyphCount) + 1U);
        const std::size_t locaEntrySize = indexFormat == 0 ? 2U : 4U;
        if (locations.size() > std::numeric_limits<std::size_t>::max() / locaEntrySize ||
            loca->length < locations.size() * locaEntrySize)
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "truncated loca table", loca->offset);
            return result;
        }
        for (std::size_t i = 0; i < locations.size(); ++i)
        {
            if (indexFormat == 0)
            {
                u16 value = 0;
                if (!reader.U16(loca->offset + i * 2U, value))
                {
                    return result;
                }
                locations[i] = static_cast<u32>(value) * 2U;
            }
            else if (!reader.U32(loca->offset + i * 4U, locations[i]))
            {
                return result;
            }
            if (i > 0 && locations[i] < locations[i - 1])
            {
                AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "loca offsets are not monotonic", loca->offset + i * locaEntrySize);
                return result;
            }
        }

        if (!ParseCmap(reader, *cmap, glyphCount, options, face.characterMap, result.diagnostics))
        {
            return result;
        }
        face.outlines.resize(glyphCount);
        GlyphParser parser{
            reader,
            *glyf,
            locations,
            options.maximumCompoundDepth,
            std::vector<GlyphOutline>(glyphCount),
            std::vector<u8>(glyphCount, 0),
            result.diagnostics};
        for (u16 glyph = 0; glyph < glyphCount; ++glyph)
        {
            if (!parser.Parse(glyph, 0))
            {
                return result;
            }
        }
        face.outlines = std::move(parser.cache);

        const auto name = tables.find(kTagName);
        if (name != tables.end())
        {
            (void)ParseName(reader, name->second, face.familyName, result.diagnostics);
        }
        if (face.familyName.empty())
        {
            face.familyName = "Unnamed TrueType Font";
        }
        const auto kern = tables.find(kTagKern);
        if (kern != tables.end() && !ParseKerning(reader, kern->second, glyphCount, face.kerning))
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Warning, "ignored malformed kern table", kern->second.offset);
            face.kerning.clear();
        }

        result.face = std::move(face);
        return result;
    }

    LoadResult LoadTrueTypeFile(std::string_view path, const LoadOptions& options)
    {
        std::vector<u8> bytes;
        std::string error;
        if (!ReadFile(path, bytes, error))
        {
            LoadResult result;
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, std::move(error));
            return result;
        }
        return LoadTrueType(bytes.data(), bytes.size(), options);
    }

    bool RasterOptions::IsValid() const
    {
        return std::isfinite(pixelHeight) && pixelHeight > 0.0f && pixelHeight <= 1024.0f &&
            oversample > 0 && oversample <= 16 && padding <= 64 && maximumBitmapDimension > 0;
    }

    bool RasterizedGlyph::IsValid() const
    {
        return std::isfinite(advance) && width <= 16384 && height <= 16384 &&
            alphaPixels.size() == static_cast<std::size_t>(width) * height;
    }

    RasterizedGlyph RasterizeGlyph(
        const FontFace& face,
        GlyphId glyph,
        const RasterOptions& options)
    {
        RasterizedGlyph result;
        result.glyph = glyph;
        if (!face.IsValid() || glyph >= face.outlines.size() || !options.IsValid())
        {
            return result;
        }
        const f32 scale = options.pixelHeight / static_cast<f32>(face.unitsPerEm);
        result.advance = static_cast<f32>(face.metrics[glyph].advanceWidth) * scale;
        const GlyphOutline& outline = face.outlines[glyph];
        if (outline.contours.empty())
        {
            return result;
        }

        const i32 left = static_cast<i32>(std::floor(static_cast<f32>(outline.xMin) * scale)) -
            static_cast<i32>(options.padding);
        const i32 right = static_cast<i32>(std::ceil(static_cast<f32>(outline.xMax) * scale)) +
            static_cast<i32>(options.padding);
        const i32 bottom = static_cast<i32>(std::floor(static_cast<f32>(outline.yMin) * scale)) -
            static_cast<i32>(options.padding);
        const i32 top = static_cast<i32>(std::ceil(static_cast<f32>(outline.yMax) * scale)) +
            static_cast<i32>(options.padding);
        if (right <= left || top <= bottom)
        {
            return result;
        }
        const u32 width = static_cast<u32>(right - left);
        const u32 height = static_cast<u32>(top - bottom);
        if (width > options.maximumBitmapDimension || height > options.maximumBitmapDimension ||
            static_cast<std::size_t>(width) > std::numeric_limits<std::size_t>::max() / height)
        {
            return {};
        }
        result.width = width;
        result.height = height;
        result.bearingX = left;
        result.bearingY = top;
        result.alphaPixels.assign(static_cast<std::size_t>(width) * height, 0);

        const auto polygons = FlattenOutline(outline);
        const u32 samples = options.oversample * options.oversample;
        for (u32 y = 0; y < height; ++y)
        {
            for (u32 x = 0; x < width; ++x)
            {
                u32 covered = 0;
                for (u32 sy = 0; sy < options.oversample; ++sy)
                {
                    for (u32 sx = 0; sx < options.oversample; ++sx)
                    {
                        const f32 pixelX = static_cast<f32>(left) +
                            (static_cast<f32>(x) + (static_cast<f32>(sx) + 0.5f) /
                                static_cast<f32>(options.oversample));
                        const f32 pixelYFromTop = static_cast<f32>(top) -
                            (static_cast<f32>(y) + (static_cast<f32>(sy) + 0.5f) /
                                static_cast<f32>(options.oversample));
                        if (PointInside(polygons, pixelX / scale, pixelYFromTop / scale))
                        {
                            ++covered;
                        }
                    }
                }
                result.alphaPixels[static_cast<std::size_t>(y) * width + x] =
                    static_cast<u8>((covered * 255U + samples / 2U) / samples);
            }
        }
        return result;
    }

    bool BakeOptions::IsValid() const
    {
        if (!std::isfinite(pixelHeight) || pixelHeight <= 0.0f || pixelHeight > 1024.0f ||
            atlasWidth == 0 || atlasHeight == 0 || atlasWidth > 16384 || atlasHeight > 16384 ||
            oversample == 0 || oversample > 16 || padding > 64 || ranges.empty())
        {
            return false;
        }
        for (const CharacterRange& range : ranges)
        {
            if (range.first > range.last || range.last > 0x10FFFF)
            {
                return false;
            }
        }
        return true;
    }

    bool BakedFont::IsValid() const
    {
        return atlasWidth > 0 && atlasHeight > 0 && std::isfinite(pixelHeight) &&
            pixelHeight > 0.0f && std::isfinite(lineHeight) && lineHeight > 0.0f &&
            rgbaPixels.size() == static_cast<std::size_t>(atlasWidth) * atlasHeight * 4U &&
            !glyphs.empty() && FindGlyph(fallbackCodepoint) != nullptr;
    }

    const BakedGlyph* BakedFont::FindGlyph(char32_t codepoint) const
    {
        const auto found = std::lower_bound(glyphs.begin(), glyphs.end(), codepoint,
            [](const BakedGlyph& glyph, char32_t value)
            {
                return glyph.codepoint < value;
            });
        return found != glyphs.end() && found->codepoint == codepoint ? &*found : nullptr;
    }

    f32 BakedFont::GetKerning(char32_t left, char32_t right) const
    {
        const auto found = std::lower_bound(
            kerning.begin(), kerning.end(), std::pair<char32_t, char32_t>{left, right},
            [](const BakedKerning& pair, const std::pair<char32_t, char32_t>& key)
            {
                return std::tie(pair.left, pair.right) < std::tie(key.first, key.second);
            });
        return found != kerning.end() && found->left == left && found->right == right
            ? found->value
            : 0.0f;
    }

    BakeResult BakeFont(const FontFace& face, const BakeOptions& options)
    {
        BakeResult result;
        if (!face.IsValid() || !options.IsValid())
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "invalid font face or bake options");
            return result;
        }

        std::set<char32_t> codepoints;
        for (const CharacterRange& range : options.ranges)
        {
            for (char32_t cp = range.first; cp <= range.last; ++cp)
            {
                codepoints.insert(cp);
                if (cp == range.last)
                {
                    break;
                }
            }
        }
        codepoints.insert(options.fallbackCodepoint);

        BakedFont font;
        font.familyName = face.familyName;
        font.atlasWidth = options.atlasWidth;
        font.atlasHeight = options.atlasHeight;
        font.pixelHeight = options.pixelHeight;
        const f32 scale = options.pixelHeight / static_cast<f32>(face.unitsPerEm);
        font.ascent = static_cast<f32>(face.ascender) * scale;
        font.descent = static_cast<f32>(face.descender) * scale;
        font.lineHeight = static_cast<f32>(face.ascender - face.descender + face.lineGap) * scale;
        if (!(font.lineHeight > 0.0f))
        {
            font.lineHeight = options.pixelHeight;
        }
        font.fallbackCodepoint = options.fallbackCodepoint;
        font.rgbaPixels.assign(
            static_cast<std::size_t>(font.atlasWidth) * font.atlasHeight * 4U,
            0);
        font.rgbaPixels[0] = 255;
        font.rgbaPixels[1] = 255;
        font.rgbaPixels[2] = 255;
        font.rgbaPixels[3] = 255;

        u32 x = 1;
        u32 y = 0;
        u32 rowHeight = 1;
        std::unordered_map<GlyphId, RasterizedGlyph> rasterCache;
        for (char32_t codepoint : codepoints)
        {
            GlyphId glyphId = face.GetGlyphId(codepoint);
            if (glyphId == 0 && codepoint != 0 && codepoint != options.fallbackCodepoint)
            {
                glyphId = face.GetGlyphId(options.fallbackCodepoint);
                AddDiagnostic(result.diagnostics, DiagnosticSeverity::Warning,
                    "codepoint mapped to fallback glyph");
            }
            auto found = rasterCache.find(glyphId);
            if (found == rasterCache.end())
            {
                RasterOptions rasterOptions;
                rasterOptions.pixelHeight = options.pixelHeight;
                rasterOptions.oversample = options.oversample;
                rasterOptions.padding = options.padding;
                rasterOptions.maximumBitmapDimension = (std::max)(options.atlasWidth, options.atlasHeight);
                found = rasterCache.emplace(glyphId, RasterizeGlyph(face, glyphId, rasterOptions)).first;
            }
            const RasterizedGlyph& raster = found->second;
            if (!raster.IsValid())
            {
                AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "glyph rasterization failed");
                return result;
            }
            if (x + raster.width > font.atlasWidth)
            {
                x = 0;
                y += rowHeight;
                rowHeight = 0;
            }
            if (y + raster.height > font.atlasHeight)
            {
                AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "font atlas is too small");
                return result;
            }
            for (u32 row = 0; row < raster.height; ++row)
            {
                for (u32 column = 0; column < raster.width; ++column)
                {
                    const u8 alpha = raster.alphaPixels[static_cast<std::size_t>(row) * raster.width + column];
                    const std::size_t target =
                        (static_cast<std::size_t>(y + row) * font.atlasWidth + x + column) * 4U;
                    font.rgbaPixels[target + 0] = 255;
                    font.rgbaPixels[target + 1] = 255;
                    font.rgbaPixels[target + 2] = 255;
                    font.rgbaPixels[target + 3] = alpha;
                }
            }
            BakedGlyph glyph;
            glyph.codepoint = codepoint;
            glyph.glyph = glyphId;
            glyph.advance = raster.advance;
            glyph.width = static_cast<f32>(raster.width);
            glyph.height = static_cast<f32>(raster.height);
            glyph.bearingX = static_cast<f32>(raster.bearingX);
            glyph.bearingY = font.ascent - static_cast<f32>(raster.bearingY);
            glyph.u0 = static_cast<f32>(x) / static_cast<f32>(font.atlasWidth);
            glyph.v0 = static_cast<f32>(y) / static_cast<f32>(font.atlasHeight);
            glyph.u1 = static_cast<f32>(x + raster.width) / static_cast<f32>(font.atlasWidth);
            glyph.v1 = static_cast<f32>(y + raster.height) / static_cast<f32>(font.atlasHeight);
            font.glyphs.push_back(glyph);
            x += raster.width + 1U;
            rowHeight = (std::max)(rowHeight, raster.height + 1U);
        }
        std::sort(font.glyphs.begin(), font.glyphs.end(), [](const BakedGlyph& a, const BakedGlyph& b)
        {
            return a.codepoint < b.codepoint;
        });

        std::unordered_map<GlyphId, std::vector<char32_t>> reverse;
        for (const BakedGlyph& glyph : font.glyphs)
        {
            reverse[glyph.glyph].push_back(glyph.codepoint);
        }
        for (const KerningPair& pair : face.kerning)
        {
            const auto left = reverse.find(pair.left);
            const auto right = reverse.find(pair.right);
            if (left == reverse.end() || right == reverse.end())
            {
                continue;
            }
            for (char32_t leftCodepoint : left->second)
            {
                for (char32_t rightCodepoint : right->second)
                {
                    font.kerning.push_back({
                        leftCodepoint,
                        rightCodepoint,
                        static_cast<f32>(pair.value) * scale});
                }
            }
        }
        std::sort(font.kerning.begin(), font.kerning.end(), [](const BakedKerning& a, const BakedKerning& b)
        {
            return std::tie(a.left, a.right) < std::tie(b.left, b.right);
        });
        if (!font.IsValid())
        {
            AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, "baked font failed validation");
            return result;
        }
        result.font = std::move(font);
        return result;
    }

    bool SaveProcessedFont(const BakedFont& font, std::vector<u8>& output, std::string* error)
    {
        output.clear();
        if (!font.IsValid() || font.familyName.size() > std::numeric_limits<u16>::max() ||
            font.glyphs.size() > std::numeric_limits<u32>::max() ||
            font.kerning.size() > std::numeric_limits<u32>::max() ||
            font.rgbaPixels.size() > std::numeric_limits<u32>::max())
        {
            if (error)
            {
                *error = "invalid or oversized baked font";
            }
            return false;
        }
        output.insert(output.end(), {'P', 'F', 'N', 'T'});
        PutU16(output, 1);
        PutU16(output, 0);
        PutU32(output, font.atlasWidth);
        PutU32(output, font.atlasHeight);
        PutF32(output, font.pixelHeight);
        PutF32(output, font.lineHeight);
        PutF32(output, font.ascent);
        PutF32(output, font.descent);
        PutU32(output, static_cast<u32>(font.fallbackCodepoint));
        PutU32(output, static_cast<u32>(font.glyphs.size()));
        PutU32(output, static_cast<u32>(font.kerning.size()));
        PutU32(output, static_cast<u32>(font.rgbaPixels.size()));
        PutU16(output, static_cast<u16>(font.familyName.size()));
        output.insert(output.end(), font.familyName.begin(), font.familyName.end());
        for (const BakedGlyph& glyph : font.glyphs)
        {
            PutU32(output, static_cast<u32>(glyph.codepoint));
            PutU16(output, glyph.glyph);
            PutU16(output, 0);
            PutF32(output, glyph.advance);
            PutF32(output, glyph.width);
            PutF32(output, glyph.height);
            PutF32(output, glyph.bearingX);
            PutF32(output, glyph.bearingY);
            PutF32(output, glyph.u0);
            PutF32(output, glyph.v0);
            PutF32(output, glyph.u1);
            PutF32(output, glyph.v1);
        }
        for (const BakedKerning& pair : font.kerning)
        {
            PutU32(output, static_cast<u32>(pair.left));
            PutU32(output, static_cast<u32>(pair.right));
            PutF32(output, pair.value);
        }
        output.insert(output.end(), font.rgbaPixels.begin(), font.rgbaPixels.end());
        PutU32(output, HashBytes(output.data(), output.size()));
        return true;
    }

    bool SaveProcessedFontFile(
        const BakedFont& font,
        std::string_view path,
        std::string* error)
    {
        std::vector<u8> bytes;
        if (!SaveProcessedFont(font, bytes, error))
        {
            return false;
        }
        std::string localError;
        if (!WriteFile(path, bytes, localError))
        {
            if (error)
            {
                *error = std::move(localError);
            }
            return false;
        }
        return true;
    }

    bool LoadProcessedFont(
        const u8* data,
        std::size_t size,
        BakedFont& output,
        std::string* error)
    {
        output = {};
        if (!data || size < 50 || std::memcmp(data, "PFNT", 4) != 0)
        {
            if (error)
            {
                *error = "invalid processed-font header";
            }
            return false;
        }
        u32 expectedHash = static_cast<u32>(data[size - 4]) |
            (static_cast<u32>(data[size - 3]) << 8U) |
            (static_cast<u32>(data[size - 2]) << 16U) |
            (static_cast<u32>(data[size - 1]) << 24U);
        if (HashBytes(data, size - 4U) != expectedHash)
        {
            if (error)
            {
                *error = "processed-font checksum mismatch";
            }
            return false;
        }
        LittleReader reader(data, size - 4U);
        std::size_t cursor = 4;
        u16 version = 0;
        u16 flags = 0;
        u32 fallback = 0;
        u32 glyphCount = 0;
        u32 kerningCount = 0;
        u32 pixelCount = 0;
        u16 familyLength = 0;
        if (!reader.U16(cursor, version) || !reader.U16(cursor, flags) || version != 1 || flags != 0 ||
            !reader.U32(cursor, output.atlasWidth) || !reader.U32(cursor, output.atlasHeight) ||
            !reader.F32(cursor, output.pixelHeight) || !reader.F32(cursor, output.lineHeight) ||
            !reader.F32(cursor, output.ascent) || !reader.F32(cursor, output.descent) ||
            !reader.U32(cursor, fallback) || !reader.U32(cursor, glyphCount) ||
            !reader.U32(cursor, kerningCount) || !reader.U32(cursor, pixelCount) ||
            !reader.U16(cursor, familyLength) || glyphCount > 1000000U ||
            kerningCount > 4000000U || pixelCount > 1024U * 1024U * 1024U)
        {
            if (error)
            {
                *error = "invalid processed-font metadata";
            }
            return false;
        }
        output.fallbackCodepoint = static_cast<char32_t>(fallback);
        const u8* family = nullptr;
        if (!reader.Bytes(cursor, familyLength, family))
        {
            return false;
        }
        output.familyName.assign(reinterpret_cast<const char*>(family), familyLength);
        output.glyphs.resize(glyphCount);
        for (BakedGlyph& glyph : output.glyphs)
        {
            u32 codepoint = 0;
            u16 reserved = 0;
            if (!reader.U32(cursor, codepoint) || !reader.U16(cursor, glyph.glyph) ||
                !reader.U16(cursor, reserved) || reserved != 0 ||
                !reader.F32(cursor, glyph.advance) || !reader.F32(cursor, glyph.width) ||
                !reader.F32(cursor, glyph.height) || !reader.F32(cursor, glyph.bearingX) ||
                !reader.F32(cursor, glyph.bearingY) || !reader.F32(cursor, glyph.u0) ||
                !reader.F32(cursor, glyph.v0) || !reader.F32(cursor, glyph.u1) ||
                !reader.F32(cursor, glyph.v1))
            {
                if (error)
                {
                    *error = "truncated processed-font glyph records";
                }
                return false;
            }
            glyph.codepoint = static_cast<char32_t>(codepoint);
        }
        output.kerning.resize(kerningCount);
        for (BakedKerning& pair : output.kerning)
        {
            u32 left = 0;
            u32 right = 0;
            if (!reader.U32(cursor, left) || !reader.U32(cursor, right) ||
                !reader.F32(cursor, pair.value))
            {
                return false;
            }
            pair.left = static_cast<char32_t>(left);
            pair.right = static_cast<char32_t>(right);
        }
        const u8* pixels = nullptr;
        if (!reader.Bytes(cursor, pixelCount, pixels) || cursor != size - 4U)
        {
            if (error)
            {
                *error = "processed-font payload size mismatch";
            }
            return false;
        }
        output.rgbaPixels.assign(pixels, pixels + pixelCount);
        if (!std::is_sorted(output.glyphs.begin(), output.glyphs.end(), [](const BakedGlyph& a, const BakedGlyph& b)
            {
                return a.codepoint < b.codepoint;
            }) ||
            !std::is_sorted(output.kerning.begin(), output.kerning.end(), [](const BakedKerning& a, const BakedKerning& b)
            {
                return std::tie(a.left, a.right) < std::tie(b.left, b.right);
            }) || !output.IsValid())
        {
            output = {};
            if (error)
            {
                *error = "processed-font content failed validation";
            }
            return false;
        }
        return true;
    }

    bool LoadProcessedFontFile(
        std::string_view path,
        BakedFont& output,
        std::string* error)
    {
        std::vector<u8> bytes;
        std::string localError;
        if (!ReadFile(path, bytes, localError))
        {
            if (error)
            {
                *error = std::move(localError);
            }
            return false;
        }
        return LoadProcessedFont(bytes.data(), bytes.size(), output, error);
    }
} // namespace Pyramid::Font
