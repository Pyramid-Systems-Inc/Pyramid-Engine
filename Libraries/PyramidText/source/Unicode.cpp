#include <Pyramid/Text/Text.hpp>

#include <algorithm>

namespace Pyramid::Text
{
    namespace
    {
        bool InRange(char32_t value, char32_t first, char32_t last)
        {
            return value >= first && value <= last;
        }

        bool IsControl(char32_t codepoint)
        {
            return codepoint == U'\r' || codepoint == U'\n' ||
                InRange(codepoint, 0x0000, 0x001F) ||
                InRange(codepoint, 0x007F, 0x009F) ||
                codepoint == 0x00AD || codepoint == 0x061C ||
                InRange(codepoint, 0x200B, 0x200C) ||
                InRange(codepoint, 0x200E, 0x200F) ||
                InRange(codepoint, 0x2028, 0x202E) ||
                InRange(codepoint, 0x2060, 0x206F) || codepoint == 0xFEFF;
        }

        bool IsExtend(char32_t codepoint)
        {
            return InRange(codepoint, 0x0300, 0x036F) ||
                InRange(codepoint, 0x0483, 0x0489) ||
                InRange(codepoint, 0x0591, 0x05BD) || codepoint == 0x05BF ||
                InRange(codepoint, 0x05C1, 0x05C2) ||
                InRange(codepoint, 0x05C4, 0x05C5) || codepoint == 0x05C7 ||
                InRange(codepoint, 0x0610, 0x061A) ||
                InRange(codepoint, 0x064B, 0x065F) || codepoint == 0x0670 ||
                InRange(codepoint, 0x06D6, 0x06ED) ||
                InRange(codepoint, 0x0711, 0x0711) ||
                InRange(codepoint, 0x0730, 0x074A) ||
                InRange(codepoint, 0x07A6, 0x07B0) ||
                InRange(codepoint, 0x07EB, 0x07F3) ||
                InRange(codepoint, 0x0816, 0x082D) ||
                InRange(codepoint, 0x0859, 0x085B) ||
                InRange(codepoint, 0x08D3, 0x0902) ||
                InRange(codepoint, 0x093A, 0x093C) || codepoint == 0x094D ||
                InRange(codepoint, 0x0951, 0x0957) ||
                InRange(codepoint, 0x0962, 0x0963) ||
                InRange(codepoint, 0x1AB0, 0x1AFF) ||
                InRange(codepoint, 0x1DC0, 0x1DFF) ||
                InRange(codepoint, 0x20D0, 0x20FF) ||
                InRange(codepoint, 0xFE00, 0xFE0F) ||
                InRange(codepoint, 0xFE20, 0xFE2F) ||
                InRange(codepoint, 0x1F3FB, 0x1F3FF) ||
                InRange(codepoint, 0xE0100, 0xE01EF);
        }

        bool IsSpacingMark(char32_t codepoint)
        {
            return codepoint == 0x0903 || codepoint == 0x093B ||
                InRange(codepoint, 0x093E, 0x0940) ||
                InRange(codepoint, 0x0949, 0x094C) ||
                InRange(codepoint, 0x0982, 0x0983) ||
                InRange(codepoint, 0x09BE, 0x09C0) ||
                InRange(codepoint, 0x0A3E, 0x0A40) ||
                InRange(codepoint, 0x0ABE, 0x0AC0) ||
                InRange(codepoint, 0x0B3E, 0x0B40) ||
                InRange(codepoint, 0x0BBE, 0x0BC2) ||
                InRange(codepoint, 0x0C01, 0x0C03) ||
                InRange(codepoint, 0x0C41, 0x0C44) ||
                InRange(codepoint, 0x0D3E, 0x0D40) ||
                InRange(codepoint, 0x0DCF, 0x0DD1) ||
                InRange(codepoint, 0x102B, 0x102C) ||
                InRange(codepoint, 0x17B6, 0x17C8);
        }

        bool IsPrepend(char32_t codepoint)
        {
            return InRange(codepoint, 0x0600, 0x0605) || codepoint == 0x06DD ||
                codepoint == 0x070F || codepoint == 0x08E2 ||
                codepoint == 0x110BD || codepoint == 0x110CD;
        }

        bool IsRegionalIndicator(char32_t codepoint)
        {
            return InRange(codepoint, 0x1F1E6, 0x1F1FF);
        }

        bool IsExtendedPictographic(char32_t codepoint)
        {
            return InRange(codepoint, 0x1F000, 0x1FAFF) ||
                InRange(codepoint, 0x2600, 0x27BF) ||
                InRange(codepoint, 0x2300, 0x23FF);
        }

        enum class HangulClass
        {
            None,
            L,
            V,
            T,
            LV,
            LVT
        };

        HangulClass GetHangulClass(char32_t codepoint)
        {
            if (InRange(codepoint, 0x1100, 0x115F) ||
                InRange(codepoint, 0xA960, 0xA97C))
            {
                return HangulClass::L;
            }
            if (InRange(codepoint, 0x1160, 0x11A7) ||
                InRange(codepoint, 0xD7B0, 0xD7C6))
            {
                return HangulClass::V;
            }
            if (InRange(codepoint, 0x11A8, 0x11FF) ||
                InRange(codepoint, 0xD7CB, 0xD7FB))
            {
                return HangulClass::T;
            }
            if (InRange(codepoint, 0xAC00, 0xD7A3))
            {
                return ((codepoint - 0xAC00) % 28) == 0
                    ? HangulClass::LV
                    : HangulClass::LVT;
            }
            return HangulClass::None;
        }

        bool HasExtendedPictographicBeforeZwj(
            std::u32string_view text,
            std::size_t zwjIndex)
        {
            if (zwjIndex == 0 || text[zwjIndex] != 0x200D)
            {
                return false;
            }
            std::size_t index = zwjIndex;
            while (index > 0)
            {
                --index;
                if (IsExtend(text[index]))
                {
                    continue;
                }
                return IsExtendedPictographic(text[index]);
            }
            return false;
        }

        bool ShouldBreak(
            std::u32string_view text,
            std::size_t index,
            std::size_t regionalIndicatorCount)
        {
            const char32_t left = text[index - 1U];
            const char32_t right = text[index];
            if (left == U'\r' && right == U'\n')
            {
                return false;
            }
            if (IsControl(left) || IsControl(right))
            {
                return true;
            }

            const HangulClass leftHangul = GetHangulClass(left);
            const HangulClass rightHangul = GetHangulClass(right);
            if (leftHangul == HangulClass::L &&
                (rightHangul == HangulClass::L || rightHangul == HangulClass::V ||
                 rightHangul == HangulClass::LV || rightHangul == HangulClass::LVT))
            {
                return false;
            }
            if ((leftHangul == HangulClass::LV || leftHangul == HangulClass::V) &&
                (rightHangul == HangulClass::V || rightHangul == HangulClass::T))
            {
                return false;
            }
            if ((leftHangul == HangulClass::LVT || leftHangul == HangulClass::T) &&
                rightHangul == HangulClass::T)
            {
                return false;
            }

            if (IsExtend(right) || right == 0x200D || IsSpacingMark(right))
            {
                return false;
            }
            if (IsPrepend(left))
            {
                return false;
            }
            if (IsExtendedPictographic(right) && left == 0x200D &&
                HasExtendedPictographicBeforeZwj(text, index - 1U))
            {
                return false;
            }
            if (IsRegionalIndicator(left) && IsRegionalIndicator(right))
            {
                return regionalIndicatorCount % 2U == 0U;
            }
            return true;
        }
    } // namespace

    std::vector<TextRange> SegmentGraphemes(std::u32string_view text)
    {
        std::vector<TextRange> clusters;
        if (text.empty())
        {
            return clusters;
        }

        std::size_t clusterStart = 0;
        std::size_t regionalIndicators = IsRegionalIndicator(text.front()) ? 1U : 0U;
        for (std::size_t index = 1; index < text.size(); ++index)
        {
            if (ShouldBreak(text, index, regionalIndicators))
            {
                clusters.push_back({clusterStart, index});
                clusterStart = index;
                regionalIndicators = IsRegionalIndicator(text[index]) ? 1U : 0U;
            }
            else if (IsRegionalIndicator(text[index]))
            {
                ++regionalIndicators;
            }
            else if (!IsExtend(text[index]) && text[index] != 0x200D)
            {
                regionalIndicators = 0;
            }
        }
        clusters.push_back({clusterStart, text.size()});
        return clusters;
    }

    bool IsGraphemeBoundary(std::u32string_view text, std::size_t index)
    {
        if (index == 0 || index >= text.size())
        {
            return index <= text.size();
        }
        const auto clusters = SegmentGraphemes(text);
        return std::any_of(clusters.begin(), clusters.end(), [index](const TextRange& range)
        {
            return range.begin == index || range.end == index;
        });
    }

    std::size_t PreviousGraphemeBoundary(std::u32string_view text, std::size_t index)
    {
        index = (std::min)(index, text.size());
        if (index == 0)
        {
            return 0;
        }
        const auto clusters = SegmentGraphemes(text);
        std::size_t previous = 0;
        for (const TextRange& range : clusters)
        {
            if (range.end >= index)
            {
                return range.begin;
            }
            previous = range.end;
        }
        return previous;
    }

    std::size_t NextGraphemeBoundary(std::u32string_view text, std::size_t index)
    {
        index = (std::min)(index, text.size());
        if (index >= text.size())
        {
            return text.size();
        }
        const auto clusters = SegmentGraphemes(text);
        for (const TextRange& range : clusters)
        {
            if (index < range.end)
            {
                return range.end;
            }
        }
        return text.size();
    }
} // namespace Pyramid::Text
