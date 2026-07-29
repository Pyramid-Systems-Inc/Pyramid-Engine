#include <Pyramid/Text/Text.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <tuple>
#include <utility>

namespace Pyramid::Text
{
    namespace
    {
        bool InRange(char32_t value, char32_t first, char32_t last)
        {
            return value >= first && value <= last;
        }

        void SetError(std::string* error, std::string message)
        {
            if (error)
            {
                *error = std::move(message);
            }
        }

        struct ArabicForm
        {
            char32_t base;
            char32_t isolated;
            char32_t final;
            char32_t initial;
            char32_t medial;
            bool joinsPrevious;
            bool joinsNext;
        };

        constexpr ArabicForm kArabicForms[] = {
            {0x0621, 0xFE80, 0,      0,      0,      false, false},
            {0x0622, 0xFE81, 0xFE82, 0,      0,      true,  false},
            {0x0623, 0xFE83, 0xFE84, 0,      0,      true,  false},
            {0x0624, 0xFE85, 0xFE86, 0,      0,      true,  false},
            {0x0625, 0xFE87, 0xFE88, 0,      0,      true,  false},
            {0x0626, 0xFE89, 0xFE8A, 0xFE8B, 0xFE8C, true,  true },
            {0x0627, 0xFE8D, 0xFE8E, 0,      0,      true,  false},
            {0x0628, 0xFE8F, 0xFE90, 0xFE91, 0xFE92, true,  true },
            {0x0629, 0xFE93, 0xFE94, 0,      0,      true,  false},
            {0x062A, 0xFE95, 0xFE96, 0xFE97, 0xFE98, true,  true },
            {0x062B, 0xFE99, 0xFE9A, 0xFE9B, 0xFE9C, true,  true },
            {0x062C, 0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0, true,  true },
            {0x062D, 0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4, true,  true },
            {0x062E, 0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8, true,  true },
            {0x062F, 0xFEA9, 0xFEAA, 0,      0,      true,  false},
            {0x0630, 0xFEAB, 0xFEAC, 0,      0,      true,  false},
            {0x0631, 0xFEAD, 0xFEAE, 0,      0,      true,  false},
            {0x0632, 0xFEAF, 0xFEB0, 0,      0,      true,  false},
            {0x0633, 0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4, true,  true },
            {0x0634, 0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8, true,  true },
            {0x0635, 0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC, true,  true },
            {0x0636, 0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0, true,  true },
            {0x0637, 0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4, true,  true },
            {0x0638, 0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8, true,  true },
            {0x0639, 0xFEC9, 0xFECA, 0xFECB, 0xFECC, true,  true },
            {0x063A, 0xFECD, 0xFECE, 0xFECF, 0xFED0, true,  true },
            {0x0641, 0xFED1, 0xFED2, 0xFED3, 0xFED4, true,  true },
            {0x0642, 0xFED5, 0xFED6, 0xFED7, 0xFED8, true,  true },
            {0x0643, 0xFED9, 0xFEDA, 0xFEDB, 0xFEDC, true,  true },
            {0x0644, 0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0, true,  true },
            {0x0645, 0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4, true,  true },
            {0x0646, 0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8, true,  true },
            {0x0647, 0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC, true,  true },
            {0x0648, 0xFEED, 0xFEEE, 0,      0,      true,  false},
            {0x0649, 0xFEEF, 0xFEF0, 0,      0,      true,  false},
            {0x064A, 0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4, true,  true },
        };

        const ArabicForm* FindArabicForm(char32_t codepoint)
        {
            const auto found = std::lower_bound(
                std::begin(kArabicForms),
                std::end(kArabicForms),
                codepoint,
                [](const ArabicForm& form, char32_t value)
                {
                    return form.base < value;
                });
            return found != std::end(kArabicForms) && found->base == codepoint
                ? &*found
                : nullptr;
        }

        bool IsArabicMark(char32_t codepoint)
        {
            return InRange(codepoint, 0x0610, 0x061A) ||
                InRange(codepoint, 0x064B, 0x065F) || codepoint == 0x0670 ||
                InRange(codepoint, 0x06D6, 0x06ED) ||
                InRange(codepoint, 0x08D3, 0x08FF);
        }

        char32_t FirstBase(std::u32string_view text, TextRange range)
        {
            for (std::size_t index = range.begin; index < range.end; ++index)
            {
                if (!IsArabicMark(text[index]))
                {
                    return text[index];
                }
            }
            return range.begin < range.end ? text[range.begin] : 0;
        }

        char32_t ShapeArabic(
            const ArabicForm& current,
            const ArabicForm* previous,
            const ArabicForm* next)
        {
            const bool joinsPrevious = previous && previous->joinsNext && current.joinsPrevious;
            const bool joinsNext = next && current.joinsNext && next->joinsPrevious;
            if (joinsPrevious && joinsNext && current.medial != 0)
            {
                return current.medial;
            }
            if (joinsPrevious && current.final != 0)
            {
                return current.final;
            }
            if (joinsNext && current.initial != 0)
            {
                return current.initial;
            }
            return current.isolated != 0 ? current.isolated : current.base;
        }

        enum class DirectionClass
        {
            Left,
            Right,
            Number,
            Neutral
        };

        DirectionClass Classify(char32_t codepoint)
        {
            if ((codepoint >= U'A' && codepoint <= U'Z') ||
                (codepoint >= U'a' && codepoint <= U'z') ||
                InRange(codepoint, 0x00C0, 0x02AF) ||
                InRange(codepoint, 0x0370, 0x052F) ||
                InRange(codepoint, 0x1E00, 0x1EFF))
            {
                return DirectionClass::Left;
            }
            if ((codepoint >= U'0' && codepoint <= U'9') ||
                InRange(codepoint, 0x0660, 0x0669) ||
                InRange(codepoint, 0x06F0, 0x06F9))
            {
                return DirectionClass::Number;
            }
            if (InRange(codepoint, 0x0590, 0x08FF) ||
                InRange(codepoint, 0xFB1D, 0xFDFF) ||
                InRange(codepoint, 0xFE70, 0xFEFF) ||
                InRange(codepoint, 0x10800, 0x10FFF))
            {
                return DirectionClass::Right;
            }
            return DirectionClass::Neutral;
        }

        ResolvedDirection ToResolved(DirectionClass direction)
        {
            return direction == DirectionClass::Right
                ? ResolvedDirection::RightToLeft
                : ResolvedDirection::LeftToRight;
        }

        char32_t Mirror(char32_t codepoint)
        {
            switch (codepoint)
            {
                case U'(': return U')';
                case U')': return U'(';
                case U'[': return U']';
                case U']': return U'[';
                case U'{': return U'}';
                case U'}': return U'{';
                case U'<': return U'>';
                case U'>': return U'<';
                case 0x00AB: return 0x00BB;
                case 0x00BB: return 0x00AB;
                case 0x2039: return 0x203A;
                case 0x203A: return 0x2039;
                default: return codepoint;
            }
        }

        bool IsWhitespace(char32_t codepoint)
        {
            return codepoint == U' ' || codepoint == U'\t' || codepoint == 0x00A0 ||
                codepoint == 0x1680 || InRange(codepoint, 0x2000, 0x200A) ||
                codepoint == 0x202F || codepoint == 0x205F || codepoint == 0x3000;
        }

        bool IsCjk(char32_t codepoint)
        {
            return InRange(codepoint, 0x2E80, 0x2FFF) ||
                InRange(codepoint, 0x3040, 0x30FF) ||
                InRange(codepoint, 0x3100, 0x312F) ||
                InRange(codepoint, 0x3130, 0x318F) ||
                InRange(codepoint, 0x31A0, 0x31BF) ||
                InRange(codepoint, 0x3400, 0x4DBF) ||
                InRange(codepoint, 0x4E00, 0x9FFF) ||
                InRange(codepoint, 0xAC00, 0xD7AF) ||
                InRange(codepoint, 0xF900, 0xFAFF) ||
                InRange(codepoint, 0x20000, 0x2FA1F);
        }

        bool IsBreakOpportunity(char32_t codepoint)
        {
            return IsWhitespace(codepoint) || codepoint == U'-' || codepoint == U'/' ||
                codepoint == 0x00AD || codepoint == 0x058A || codepoint == 0x060C ||
                codepoint == 0x061B || codepoint == 0x061F || codepoint == 0x200B ||
                codepoint == 0x2010 || codepoint == 0x2013 || codepoint == 0x2014 ||
                IsCjk(codepoint);
        }

        struct LogicalCluster
        {
            TextRange range;
            char32_t base = 0;
            std::u32string render;
            DirectionClass originalDirection = DirectionClass::Neutral;
            ResolvedDirection direction = ResolvedDirection::LeftToRight;
            f32 advance = 0.0f;
            bool hardBreak = false;
            bool whitespace = false;
            bool breakAfter = false;
        };

        struct LogicalLine
        {
            std::vector<std::size_t> clusterIndices;
            std::size_t logicalBegin = 0;
            std::size_t logicalEnd = 0;
        };

        f32 MeasureRender(const FontAtlas& font, std::u32string_view render, f32 scale)
        {
            f32 width = 0.0f;
            char32_t previous = 0;
            for (char32_t codepoint : render)
            {
                if (previous != 0)
                {
                    width += font.GetKerning(previous, codepoint) * scale;
                }
                width += font.GetGlyph(codepoint).advance * scale;
                previous = codepoint;
            }
            return width;
        }

        ResolvedDirection ResolveParagraphDirection(
            const std::vector<LogicalCluster>& clusters,
            TextDirection requested)
        {
            if (requested == TextDirection::LeftToRight)
            {
                return ResolvedDirection::LeftToRight;
            }
            if (requested == TextDirection::RightToLeft)
            {
                return ResolvedDirection::RightToLeft;
            }
            for (const LogicalCluster& cluster : clusters)
            {
                if (cluster.originalDirection == DirectionClass::Left)
                {
                    return ResolvedDirection::LeftToRight;
                }
                if (cluster.originalDirection == DirectionClass::Right)
                {
                    return ResolvedDirection::RightToLeft;
                }
            }
            return ResolvedDirection::LeftToRight;
        }

        std::vector<LogicalCluster> BuildClusters(
            const FontAtlas& font,
            std::u32string_view text,
            const InternationalLayoutOptions& options)
        {
            const std::vector<TextRange> ranges = SegmentGraphemes(text);
            std::vector<LogicalCluster> clusters;
            clusters.reserve(ranges.size());
            std::vector<const ArabicForm*> arabicForms;
            arabicForms.reserve(ranges.size());
            for (const TextRange& range : ranges)
            {
                arabicForms.push_back(FindArabicForm(FirstBase(text, range)));
            }

            for (std::size_t clusterIndex = 0; clusterIndex < ranges.size(); ++clusterIndex)
            {
                const TextRange range = ranges[clusterIndex];
                LogicalCluster cluster;
                cluster.range = range;
                cluster.base = FirstBase(text, range);
                cluster.hardBreak = cluster.base == U'\n' || cluster.base == U'\r';
                cluster.whitespace = IsWhitespace(cluster.base);
                cluster.breakAfter = IsBreakOpportunity(cluster.base);
                cluster.originalDirection = Classify(cluster.base);

                if (!cluster.hardBreak)
                {
                    if (options.maskCharacter != 0)
                    {
                        cluster.render.push_back(options.maskCharacter);
                    }
                    else
                    {
                        const ArabicForm* current = arabicForms[clusterIndex];
                        if (current)
                        {
                            const ArabicForm* previous = clusterIndex > 0
                                ? arabicForms[clusterIndex - 1U]
                                : nullptr;
                            const ArabicForm* next = clusterIndex + 1U < arabicForms.size()
                                ? arabicForms[clusterIndex + 1U]
                                : nullptr;
                            cluster.render.push_back(ShapeArabic(*current, previous, next));
                            for (std::size_t index = range.begin; index < range.end; ++index)
                            {
                                if (text[index] != current->base)
                                {
                                    cluster.render.push_back(text[index]);
                                }
                            }
                        }
                        else if (cluster.base == U'\t')
                        {
                            cluster.render.assign(options.tabWidth, U' ');
                        }
                        else
                        {
                            cluster.render.assign(
                                text.begin() + static_cast<std::ptrdiff_t>(range.begin),
                                text.begin() + static_cast<std::ptrdiff_t>(range.end));
                        }
                    }
                }
                cluster.advance = MeasureRender(font, cluster.render, options.scale);
                clusters.push_back(std::move(cluster));
            }
            return clusters;
        }

        std::vector<LogicalLine> BuildLogicalLines(
            const std::vector<LogicalCluster>& clusters,
            std::size_t textSize,
            const InternationalLayoutOptions& options)
        {
            std::vector<LogicalLine> lines;
            LogicalLine line;
            line.logicalBegin = 0;
            f32 width = 0.0f;
            std::size_t lastBreakPosition = static_cast<std::size_t>(-1);

            auto recalculate = [&]()
            {
                width = 0.0f;
                lastBreakPosition = static_cast<std::size_t>(-1);
                for (std::size_t position = 0; position < line.clusterIndices.size(); ++position)
                {
                    const LogicalCluster& cluster = clusters[line.clusterIndices[position]];
                    width += cluster.advance;
                    if (cluster.breakAfter)
                    {
                        lastBreakPosition = position + 1U;
                    }
                }
            };

            auto trimTrailingWhitespace = [&]()
            {
                while (!line.clusterIndices.empty() &&
                    clusters[line.clusterIndices.back()].whitespace)
                {
                    line.clusterIndices.pop_back();
                }
            };

            auto finishLine = [&](std::size_t logicalEnd)
            {
                trimTrailingWhitespace();
                line.logicalEnd = logicalEnd;
                lines.push_back(std::move(line));
                line = {};
                line.logicalBegin = logicalEnd;
                width = 0.0f;
                lastBreakPosition = static_cast<std::size_t>(-1);
            };

            for (std::size_t index = 0; index < clusters.size(); ++index)
            {
                const LogicalCluster& cluster = clusters[index];
                if (cluster.hardBreak)
                {
                    finishLine(cluster.range.begin);
                    line.logicalBegin = cluster.range.end;
                    continue;
                }

                const bool constrained = options.maximumWidth > 0.0f &&
                    options.wrap != WrapMode::None;
                if (constrained && !line.clusterIndices.empty() &&
                    width + cluster.advance > options.maximumWidth)
                {
                    if (options.wrap == WrapMode::Word &&
                        lastBreakPosition != static_cast<std::size_t>(-1) &&
                        lastBreakPosition < line.clusterIndices.size())
                    {
                        std::vector<std::size_t> carry(
                            line.clusterIndices.begin() +
                                static_cast<std::ptrdiff_t>(lastBreakPosition),
                            line.clusterIndices.end());
                        line.clusterIndices.erase(
                            line.clusterIndices.begin() +
                                static_cast<std::ptrdiff_t>(lastBreakPosition),
                            line.clusterIndices.end());
                        const std::size_t end = carry.empty()
                            ? cluster.range.begin
                            : clusters[carry.front()].range.begin;
                        finishLine(end);
                        while (!carry.empty() && clusters[carry.front()].whitespace)
                        {
                            line.logicalBegin = clusters[carry.front()].range.end;
                            carry.erase(carry.begin());
                        }
                        line.clusterIndices = std::move(carry);
                        recalculate();
                    }
                    else
                    {
                        finishLine(cluster.range.begin);
                    }
                }

                if (line.clusterIndices.empty() && cluster.whitespace)
                {
                    line.logicalBegin = cluster.range.end;
                    continue;
                }
                line.clusterIndices.push_back(index);
                width += cluster.advance;
                if (cluster.breakAfter)
                {
                    lastBreakPosition = line.clusterIndices.size();
                }
            }

            finishLine(textSize);
            if (lines.empty())
            {
                lines.push_back({{}, 0, textSize});
            }
            return lines;
        }

        void ShapeLineClusters(
            const FontAtlas& font,
            std::u32string_view text,
            std::vector<LogicalCluster>& clusters,
            const LogicalLine& line,
            const InternationalLayoutOptions& options)
        {
            for (std::size_t position = 0; position < line.clusterIndices.size(); ++position)
            {
                LogicalCluster& cluster = clusters[line.clusterIndices[position]];
                cluster.render.clear();
                if (options.maskCharacter != 0)
                {
                    cluster.render.push_back(options.maskCharacter);
                }
                else if (const ArabicForm* current = FindArabicForm(cluster.base))
                {
                    const ArabicForm* previous = position > 0
                        ? FindArabicForm(clusters[line.clusterIndices[position - 1U]].base)
                        : nullptr;
                    const ArabicForm* next = position + 1U < line.clusterIndices.size()
                        ? FindArabicForm(clusters[line.clusterIndices[position + 1U]].base)
                        : nullptr;
                    cluster.render.push_back(ShapeArabic(*current, previous, next));
                    for (std::size_t index = cluster.range.begin; index < cluster.range.end; ++index)
                    {
                        if (text[index] != current->base)
                        {
                            cluster.render.push_back(text[index]);
                        }
                    }
                }
                else if (cluster.base == U'\t')
                {
                    cluster.render.assign(options.tabWidth, U' ');
                }
                else
                {
                    cluster.render.assign(
                        text.begin() + static_cast<std::ptrdiff_t>(cluster.range.begin),
                        text.begin() + static_cast<std::ptrdiff_t>(cluster.range.end));
                }
                cluster.advance = MeasureRender(font, cluster.render, options.scale);
            }
        }

        void ResolveDirections(
            std::vector<LogicalCluster>& clusters,
            const LogicalLine& line,
            ResolvedDirection base)
        {
            const DirectionClass baseClass = base == ResolvedDirection::RightToLeft
                ? DirectionClass::Right
                : DirectionClass::Left;
            for (std::size_t position = 0; position < line.clusterIndices.size(); ++position)
            {
                LogicalCluster& cluster = clusters[line.clusterIndices[position]];
                DirectionClass direction = cluster.originalDirection;
                if (direction == DirectionClass::Number)
                {
                    direction = DirectionClass::Left;
                }
                if (direction == DirectionClass::Neutral)
                {
                    DirectionClass before = baseClass;
                    for (std::size_t scan = position; scan > 0; --scan)
                    {
                        DirectionClass candidate =
                            clusters[line.clusterIndices[scan - 1U]].originalDirection;
                        if (candidate == DirectionClass::Number)
                        {
                            candidate = DirectionClass::Left;
                        }
                        if (candidate != DirectionClass::Neutral)
                        {
                            before = candidate;
                            break;
                        }
                    }
                    DirectionClass after = baseClass;
                    for (std::size_t scan = position + 1U;
                        scan < line.clusterIndices.size(); ++scan)
                    {
                        DirectionClass candidate =
                            clusters[line.clusterIndices[scan]].originalDirection;
                        if (candidate == DirectionClass::Number)
                        {
                            candidate = DirectionClass::Left;
                        }
                        if (candidate != DirectionClass::Neutral)
                        {
                            after = candidate;
                            break;
                        }
                    }
                    direction = before == after ? before : baseClass;
                }
                cluster.direction = ToResolved(direction);
            }
        }

        struct RunBuild
        {
            std::vector<std::size_t> clusters;
            ResolvedDirection direction = ResolvedDirection::LeftToRight;
        };

        std::vector<RunBuild> BuildVisualRuns(
            const std::vector<LogicalCluster>& clusters,
            const LogicalLine& line,
            ResolvedDirection base)
        {
            std::vector<RunBuild> runs;
            for (std::size_t index : line.clusterIndices)
            {
                const ResolvedDirection direction = clusters[index].direction;
                if (runs.empty() || runs.back().direction != direction)
                {
                    runs.push_back({{}, direction});
                }
                runs.back().clusters.push_back(index);
            }
            for (RunBuild& run : runs)
            {
                if (run.direction == ResolvedDirection::RightToLeft)
                {
                    std::reverse(run.clusters.begin(), run.clusters.end());
                }
            }
            if (base == ResolvedDirection::RightToLeft)
            {
                std::reverse(runs.begin(), runs.end());
            }
            return runs;
        }

        f32 MeasureVisualLine(
            const FontAtlas& font,
            const std::vector<LogicalCluster>& clusters,
            const std::vector<RunBuild>& runs,
            f32 scale)
        {
            f32 width = 0.0f;
            char32_t previous = 0;
            for (const RunBuild& run : runs)
            {
                for (std::size_t clusterIndex : run.clusters)
                {
                    const LogicalCluster& cluster = clusters[clusterIndex];
                    for (char32_t raw : cluster.render)
                    {
                        const char32_t codepoint = run.direction == ResolvedDirection::RightToLeft
                            ? Mirror(raw)
                            : raw;
                        if (previous != 0)
                        {
                            width += font.GetKerning(previous, codepoint) * scale;
                        }
                        width += font.GetGlyph(codepoint).advance * scale;
                        previous = codepoint;
                    }
                }
            }
            return width;
        }

        bool SameSource(const FontFamily& family, const Font::BakedKerning& pair, u32 index)
        {
            return family.ResolveFontIndex(pair.left) == index &&
                family.ResolveFontIndex(pair.right) == index;
        }
    } // namespace

    bool FontFamily::IsValid() const
    {
        return atlas.IsValid() && !familyNames.empty() &&
            std::is_sorted(glyphSources.begin(), glyphSources.end(),
                [](const FontFamilyGlyphSource& left, const FontFamilyGlyphSource& right)
                {
                    return left.codepoint < right.codepoint;
                });
    }

    u32 FontFamily::ResolveFontIndex(char32_t codepoint) const
    {
        const auto found = std::lower_bound(
            glyphSources.begin(), glyphSources.end(), codepoint,
            [](const FontFamilyGlyphSource& source, char32_t value)
            {
                return source.codepoint < value;
            });
        return found != glyphSources.end() && found->codepoint == codepoint
            ? found->fontIndex
            : 0U;
    }

    bool BuildFontFamily(
        const std::vector<FontAtlas>& fonts,
        FontFamily& output,
        std::string* error)
    {
        output = {};
        if (error)
        {
            error->clear();
        }
        if (fonts.empty())
        {
            SetError(error, "font family requires at least one atlas");
            return false;
        }
        for (const FontAtlas& font : fonts)
        {
            if (!font.IsValid())
            {
                SetError(error, "font family contains an invalid atlas");
                return false;
            }
        }

        u32 width = 0;
        std::uint64_t totalHeight = 0;
        for (const FontAtlas& font : fonts)
        {
            width = (std::max)(width, font.width);
            totalHeight += font.height;
        }
        if (width == 0 || totalHeight == 0 ||
            totalHeight > static_cast<std::uint64_t>((std::numeric_limits<u32>::max)()) ||
            static_cast<std::uint64_t>(width) * totalHeight >
                static_cast<std::uint64_t>((std::numeric_limits<std::size_t>::max)() / 4U))
        {
            SetError(error, "font family atlas dimensions overflow");
            return false;
        }

        FontFamily family;
        family.familyNames.reserve(fonts.size());
        family.atlas.familyName.clear();
        family.atlas.width = width;
        family.atlas.height = static_cast<u32>(totalHeight);
        family.atlas.pixelHeight = fonts.front().pixelHeight;
        family.atlas.lineHeight = fonts.front().lineHeight;
        family.atlas.fallbackCodepoint = fonts.front().fallbackCodepoint;
        family.atlas.rgbaPixels.assign(
            static_cast<std::size_t>(family.atlas.width) * family.atlas.height * 4U,
            0U);

        std::map<char32_t, std::pair<Glyph, u32>> selected;
        std::vector<u32> yOffsets(fonts.size(), 0U);
        u32 yOffset = 0;
        for (std::size_t fontIndex = 0; fontIndex < fonts.size(); ++fontIndex)
        {
            const FontAtlas& font = fonts[fontIndex];
            yOffsets[fontIndex] = yOffset;
            family.familyNames.push_back(font.familyName);
            if (!family.atlas.familyName.empty())
            {
                family.atlas.familyName += ", ";
            }
            family.atlas.familyName += font.familyName;
            family.atlas.lineHeight = (std::max)(family.atlas.lineHeight, font.lineHeight);

            for (u32 row = 0; row < font.height; ++row)
            {
                const std::size_t source = static_cast<std::size_t>(row) * font.width * 4U;
                const std::size_t destination =
                    (static_cast<std::size_t>(yOffset + row) * family.atlas.width) * 4U;
                std::copy_n(
                    font.rgbaPixels.begin() + static_cast<std::ptrdiff_t>(source),
                    static_cast<std::size_t>(font.width) * 4U,
                    family.atlas.rgbaPixels.begin() + static_cast<std::ptrdiff_t>(destination));
            }

            for (const Glyph& sourceGlyph : font.glyphs)
            {
                if (selected.find(sourceGlyph.codepoint) != selected.end())
                {
                    continue;
                }
                Glyph glyph = sourceGlyph;
                glyph.u0 = sourceGlyph.u0 * static_cast<f32>(font.width) /
                    static_cast<f32>(family.atlas.width);
                glyph.u1 = sourceGlyph.u1 * static_cast<f32>(font.width) /
                    static_cast<f32>(family.atlas.width);
                glyph.v0 = (static_cast<f32>(yOffset) +
                    sourceGlyph.v0 * static_cast<f32>(font.height)) /
                    static_cast<f32>(family.atlas.height);
                glyph.v1 = (static_cast<f32>(yOffset) +
                    sourceGlyph.v1 * static_cast<f32>(font.height)) /
                    static_cast<f32>(family.atlas.height);
                selected.emplace(glyph.codepoint, std::make_pair(glyph, static_cast<u32>(fontIndex)));
            }
            yOffset += font.height;
        }

        family.atlas.glyphs.reserve(selected.size());
        family.glyphSources.reserve(selected.size());
        for (const auto& [codepoint, value] : selected)
        {
            family.atlas.glyphs.push_back(value.first);
            family.glyphSources.push_back({codepoint, value.second});
        }

        const FontAtlas& primary = fonts.front();
        family.atlas.whitePixelUv = Math::Vec2(
            primary.whitePixelUv.x * static_cast<f32>(primary.width) /
                static_cast<f32>(family.atlas.width),
            primary.whitePixelUv.y * static_cast<f32>(primary.height) /
                static_cast<f32>(family.atlas.height));

        for (std::size_t fontIndex = 0; fontIndex < fonts.size(); ++fontIndex)
        {
            for (const Font::BakedKerning& pair : fonts[fontIndex].kerning)
            {
                if (SameSource(family, pair, static_cast<u32>(fontIndex)))
                {
                    family.atlas.kerning.push_back(pair);
                }
            }
        }
        std::sort(family.atlas.kerning.begin(), family.atlas.kerning.end(),
            [](const Font::BakedKerning& left, const Font::BakedKerning& right)
            {
                return std::tie(left.left, left.right) < std::tie(right.left, right.right);
            });

        if (!family.IsValid())
        {
            SetError(error, "combined font family atlas is invalid");
            return false;
        }
        output = std::move(family);
        return true;
    }

    bool InternationalLayoutOptions::IsValid() const
    {
        return std::isfinite(scale) && scale > 0.0f &&
            std::isfinite(maximumWidth) && maximumWidth >= 0.0f &&
            std::isfinite(lineSpacing) && lineSpacing >= 0.0f && tabWidth > 0 &&
            (alignment == HorizontalAlignment::Left ||
             alignment == HorizontalAlignment::Center ||
             alignment == HorizontalAlignment::Right) &&
            (wrap == WrapMode::None || wrap == WrapMode::Character ||
             wrap == WrapMode::Word) &&
            (direction == TextDirection::Auto ||
             direction == TextDirection::LeftToRight ||
             direction == TextDirection::RightToLeft);
    }

    InternationalLayoutResult LayoutInternational(
        const FontAtlas& font,
        std::u32string_view text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options)
    {
        InternationalLayoutResult result;
        if (!font.IsValid() || !options.IsValid() || !std::isfinite(origin.x) ||
            !std::isfinite(origin.y))
        {
            return result;
        }

        std::vector<LogicalCluster> clusters = BuildClusters(font, text, options);
        result.paragraphDirection = ResolveParagraphDirection(clusters, options.direction);
        const std::vector<LogicalLine> lines = BuildLogicalLines(clusters, text.size(), options);
        result.lines.reserve(lines.size());

        const f32 lineHeight = font.lineHeight * options.scale;
        f32 widest = 0.0f;
        std::vector<std::vector<RunBuild>> visualRuns;
        std::vector<f32> lineWidths;
        visualRuns.reserve(lines.size());
        lineWidths.reserve(lines.size());
        for (const LogicalLine& line : lines)
        {
            ShapeLineClusters(font, text, clusters, line, options);
            ResolveDirections(clusters, line, result.paragraphDirection);
            visualRuns.push_back(BuildVisualRuns(clusters, line, result.paragraphDirection));
            lineWidths.push_back(MeasureVisualLine(
                font, clusters, visualRuns.back(), options.scale));
            widest = (std::max)(widest, lineWidths.back());
        }
        const f32 alignmentWidth = options.maximumWidth > 0.0f
            ? options.maximumWidth
            : widest;

        for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
        {
            const LogicalLine& line = lines[lineIndex];
            const auto& runs = visualRuns[lineIndex];
            const f32 lineWidth = lineWidths[lineIndex];
            f32 x = origin.x;
            if (options.alignment == HorizontalAlignment::Center)
            {
                x += (alignmentWidth - lineWidth) * 0.5f;
            }
            else if (options.alignment == HorizontalAlignment::Right ||
                (options.alignment == HorizontalAlignment::Left &&
                 options.direction == TextDirection::Auto &&
                 result.paragraphDirection == ResolvedDirection::RightToLeft &&
                 options.maximumWidth > 0.0f))
            {
                x += alignmentWidth - lineWidth;
            }
            const f32 y = origin.y + static_cast<f32>(lineIndex) *
                (lineHeight + options.lineSpacing);

            LineMetrics metrics;
            metrics.firstGlyph = static_cast<u32>(result.glyphs.size());
            metrics.width = lineWidth;
            char32_t previous = 0;

            if (runs.empty())
            {
                result.carets.push_back({line.logicalBegin, static_cast<u32>(lineIndex), x});
            }

            for (const RunBuild& run : runs)
            {
                if (!run.clusters.empty())
                {
                    const LogicalCluster& first = clusters[run.clusters.front()];
                    const LogicalCluster& last = clusters[run.clusters.back()];
                    const std::size_t begin = (std::min)(first.range.begin, last.range.begin);
                    const std::size_t end = (std::max)(first.range.end, last.range.end);
                    result.runs.push_back({{begin, end}, static_cast<u32>(lineIndex), run.direction});
                }

                for (std::size_t clusterIndex : run.clusters)
                {
                    const LogicalCluster& cluster = clusters[clusterIndex];
                    const f32 clusterStart = x;
                    const u32 firstGlyph = static_cast<u32>(result.glyphs.size());
                    for (char32_t raw : cluster.render)
                    {
                        const char32_t codepoint = run.direction == ResolvedDirection::RightToLeft
                            ? Mirror(raw)
                            : raw;
                        if (!font.HasGlyph(codepoint))
                        {
                            ++result.fallbackGlyphs;
                        }
                        if (previous != 0)
                        {
                            x += font.GetKerning(previous, codepoint) * options.scale;
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
                            ++metrics.glyphCount;
                        }
                        x += glyph.advance * options.scale;
                        previous = codepoint;
                    }
                    const f32 clusterEnd = x;
                    result.clusters.push_back({
                        cluster.range,
                        static_cast<u32>(lineIndex),
                        (std::min)(clusterStart, clusterEnd),
                        (std::max)(clusterStart, clusterEnd),
                        run.direction,
                        firstGlyph,
                        static_cast<u32>(result.glyphs.size()) - firstGlyph});
                    if (run.direction == ResolvedDirection::LeftToRight)
                    {
                        result.carets.push_back({cluster.range.begin,
                            static_cast<u32>(lineIndex), clusterStart});
                        result.carets.push_back({cluster.range.end,
                            static_cast<u32>(lineIndex), clusterEnd});
                    }
                    else
                    {
                        result.carets.push_back({cluster.range.begin,
                            static_cast<u32>(lineIndex), clusterEnd});
                        result.carets.push_back({cluster.range.end,
                            static_cast<u32>(lineIndex), clusterStart});
                    }
                }
            }

            if (line.clusterIndices.empty())
            {
                result.carets.push_back({line.logicalEnd, static_cast<u32>(lineIndex), x});
            }
            metrics.width = lineWidth;
            result.lines.push_back(metrics);
            result.metrics.width = (std::max)(result.metrics.width, lineWidth);
        }

        std::sort(result.carets.begin(), result.carets.end(),
            [](const CaretStop& left, const CaretStop& right)
            {
                return std::tie(left.logicalIndex, left.lineIndex, left.x) <
                    std::tie(right.logicalIndex, right.lineIndex, right.x);
            });
        result.carets.erase(std::unique(result.carets.begin(), result.carets.end(),
            [](const CaretStop& left, const CaretStop& right)
            {
                return left.logicalIndex == right.logicalIndex &&
                    left.lineIndex == right.lineIndex && left.x == right.x;
            }), result.carets.end());

        result.metrics.lineCount = static_cast<u32>(lines.size());
        result.metrics.height = lines.empty()
            ? 0.0f
            : lineHeight * static_cast<f32>(lines.size()) +
                options.lineSpacing * static_cast<f32>(lines.size() - 1U);
        return result;
    }

    InternationalLayoutResult LayoutInternational(
        const FontFamily& family,
        std::u32string_view text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options)
    {
        return family.IsValid()
            ? LayoutInternational(family.atlas, text, origin, options)
            : InternationalLayoutResult{};
    }

    InternationalLayoutResult LayoutInternationalUtf8(
        const FontAtlas& font,
        std::string_view utf8Text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options)
    {
        const Utf8DecodeResult decoded = DecodeUtf8(utf8Text);
        InternationalLayoutResult result = LayoutInternational(font, decoded.text, origin, options);
        result.invalidUtf8Sequences = decoded.invalidSequences;
        return result;
    }

    InternationalLayoutResult LayoutInternationalUtf8(
        const FontFamily& family,
        std::string_view utf8Text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options)
    {
        const Utf8DecodeResult decoded = DecodeUtf8(utf8Text);
        InternationalLayoutResult result = LayoutInternational(family, decoded.text, origin, options);
        result.invalidUtf8Sequences = decoded.invalidSequences;
        return result;
    }

    CaretLocation GetCaretLocation(
        const InternationalLayoutResult& layout,
        std::size_t logicalIndex)
    {
        if (layout.carets.empty())
        {
            return {};
        }
        const auto exact = std::find_if(layout.carets.begin(), layout.carets.end(),
            [logicalIndex](const CaretStop& stop)
            {
                return stop.logicalIndex == logicalIndex;
            });
        if (exact != layout.carets.end())
        {
            return {exact->lineIndex, exact->x};
        }

        const CaretStop* best = &layout.carets.front();
        std::size_t bestDistance = best->logicalIndex > logicalIndex
            ? best->logicalIndex - logicalIndex
            : logicalIndex - best->logicalIndex;
        for (const CaretStop& stop : layout.carets)
        {
            const std::size_t distance = stop.logicalIndex > logicalIndex
                ? stop.logicalIndex - logicalIndex
                : logicalIndex - stop.logicalIndex;
            if (distance < bestDistance)
            {
                best = &stop;
                bestDistance = distance;
            }
        }
        return {best->lineIndex, best->x};
    }

    std::size_t HitTestInternational(
        const InternationalLayoutResult& layout,
        u32 lineIndex,
        f32 x)
    {
        const CaretStop* best = nullptr;
        f32 bestDistance = (std::numeric_limits<f32>::max)();
        for (const CaretStop& stop : layout.carets)
        {
            if (stop.lineIndex != lineIndex)
            {
                continue;
            }
            const f32 distance = std::fabs(stop.x - x);
            if (!best || distance < bestDistance ||
                (distance == bestDistance && stop.logicalIndex < best->logicalIndex))
            {
                best = &stop;
                bestDistance = distance;
            }
        }
        return best ? best->logicalIndex : 0U;
    }

    std::size_t MoveCaretVisual(
        const InternationalLayoutResult& layout,
        std::size_t logicalIndex,
        bool moveRight)
    {
        if (layout.carets.empty())
        {
            return logicalIndex;
        }

        const CaretLocation current = GetCaretLocation(layout, logicalIndex);
        std::vector<const CaretStop*> lineStops;
        for (const CaretStop& stop : layout.carets)
        {
            if (stop.lineIndex == current.lineIndex)
            {
                lineStops.push_back(&stop);
            }
        }
        std::sort(lineStops.begin(), lineStops.end(),
            [](const CaretStop* left, const CaretStop* right)
            {
                return std::tie(left->x, left->logicalIndex) <
                    std::tie(right->x, right->logicalIndex);
            });
        lineStops.erase(std::unique(lineStops.begin(), lineStops.end(),
            [](const CaretStop* left, const CaretStop* right)
            {
                return left->x == right->x && left->logicalIndex == right->logicalIndex;
            }), lineStops.end());

        std::size_t currentIndex = 0;
        f32 currentDistance = (std::numeric_limits<f32>::max)();
        for (std::size_t index = 0; index < lineStops.size(); ++index)
        {
            const f32 distance = std::fabs(lineStops[index]->x - current.x);
            if ((lineStops[index]->logicalIndex == logicalIndex &&
                 distance <= currentDistance) ||
                (currentDistance == (std::numeric_limits<f32>::max)() &&
                 distance < currentDistance))
            {
                currentIndex = index;
                currentDistance = distance;
            }
        }
        if (moveRight && currentIndex + 1U < lineStops.size())
        {
            return lineStops[currentIndex + 1U]->logicalIndex;
        }
        if (!moveRight && currentIndex > 0)
        {
            return lineStops[currentIndex - 1U]->logicalIndex;
        }

        const i32 adjacentLine = static_cast<i32>(current.lineIndex) +
            (moveRight ? 1 : -1);
        if (adjacentLine < 0 || adjacentLine >= static_cast<i32>(layout.lines.size()))
        {
            return logicalIndex;
        }
        const CaretStop* adjacent = nullptr;
        for (const CaretStop& stop : layout.carets)
        {
            if (stop.lineIndex != static_cast<u32>(adjacentLine))
            {
                continue;
            }
            if (!adjacent || (moveRight ? stop.x < adjacent->x : stop.x > adjacent->x))
            {
                adjacent = &stop;
            }
        }
        return adjacent ? adjacent->logicalIndex : logicalIndex;
    }

    std::vector<SelectionSpan> BuildSelectionSpans(
        const InternationalLayoutResult& layout,
        TextRange selection)
    {
        if (selection.begin > selection.end)
        {
            std::swap(selection.begin, selection.end);
        }
        std::vector<SelectionSpan> spans;
        if (selection.Empty())
        {
            return spans;
        }
        for (const VisualCluster& cluster : layout.clusters)
        {
            if (cluster.logicalRange.end <= selection.begin ||
                cluster.logicalRange.begin >= selection.end)
            {
                continue;
            }
            if (!spans.empty() && spans.back().lineIndex == cluster.lineIndex &&
                cluster.minimumX <= spans.back().maximumX + 0.01f &&
                cluster.maximumX >= spans.back().minimumX - 0.01f)
            {
                spans.back().minimumX = (std::min)(spans.back().minimumX, cluster.minimumX);
                spans.back().maximumX = (std::max)(spans.back().maximumX, cluster.maximumX);
            }
            else
            {
                spans.push_back({cluster.lineIndex, cluster.minimumX, cluster.maximumX});
            }
        }
        return spans;
    }
} // namespace Pyramid::Text
