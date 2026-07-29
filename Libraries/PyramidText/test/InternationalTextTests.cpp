#include <Pyramid/Text/Text.hpp>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "International text test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    bool ContainsGlyph(
        const Pyramid::Text::InternationalLayoutResult& layout,
        char32_t codepoint)
    {
        return std::any_of(layout.glyphs.begin(), layout.glyphs.end(),
            [codepoint](const Pyramid::Text::GlyphQuad& glyph)
            {
                return glyph.codepoint == codepoint;
            });
    }
}

int main()
{
    using namespace Pyramid::Text;

    const std::u32string graphemeText = U"A\u0301\U0001F469\u200D\U0001F4BB\U0001F1F8\U0001F1E6";
    const auto graphemes = SegmentGraphemes(graphemeText);
    Require(graphemes.size() == 3, "combining, emoji ZWJ, or flag segmentation failed");
    Require(graphemes[0].Length() == 2 && graphemes[1].Length() == 3 &&
            graphemes[2].Length() == 2,
        "grapheme ranges are incorrect");
    Require(NextGraphemeBoundary(graphemeText, 0) == 2,
        "next grapheme boundary split a combining sequence");
    Require(PreviousGraphemeBoundary(graphemeText, 5) == 2,
        "previous grapheme boundary split an emoji sequence");

    TextBuffer buffer(U"e\u0301x");
    buffer.SetCursor(1);
    Require(buffer.GetCursor() == 0, "cursor was allowed inside a grapheme cluster");
    buffer.SetCursor(2);
    Require(buffer.Backspace() && buffer.GetText() == U"x",
        "backspace did not remove the complete grapheme cluster");

    TextBuffer limited;
    limited.SetMaximumCharacters(1);
    Require(!limited.Insert(U"e\u0301") && limited.Empty(),
        "character limit retained a partial grapheme cluster");
    limited.SetMaximumCharacters(2);
    Require(limited.Insert(U"e\u0301") && limited.Size() == 2,
        "character limit rejected a complete grapheme cluster");
    limited.SetMaximumCharacters(1);
    Require(limited.Empty(),
        "normalization truncated an existing grapheme cluster internally");

    FontAtlas latin;
    FontAtlas arabic;
    std::string error;
    Require(LoadFontAtlas(PYRAMID_TEXT_TEST_FONT, latin, &error),
        "Latin processed font did not load");
    Require(LoadFontAtlas(PYRAMID_TEXT_TEST_ARABIC_FONT, arabic, &error),
        "Arabic processed font did not load");

    FontFamily family;
    Require(BuildFontFamily({latin, arabic}, family, &error),
        "font fallback family did not build");
    Require(family.familyNames.size() == 2 && family.ResolveFontIndex(0xFEB3) == 1,
        "fallback family did not resolve the Arabic source font");
    Require(family.ResolveFontIndex(U'A') == 0,
        "fallback family did not preserve primary-font precedence");

    InternationalLayoutOptions options;
    options.maximumWidth = 420.0f;
    const auto arabicLayout = LayoutInternational(
        family, U"\u0633\u0644\u0627\u0645", Pyramid::Math::Vec2::Zero, options);
    Require(arabicLayout.paragraphDirection == ResolvedDirection::RightToLeft,
        "Arabic paragraph direction was not resolved as RTL");
    Require(arabicLayout.fallbackGlyphs == 0,
        "Arabic shaping unexpectedly used the replacement glyph");
    Require(ContainsGlyph(arabicLayout, 0xFEB3) &&
            ContainsGlyph(arabicLayout, 0xFEE0) &&
            ContainsGlyph(arabicLayout, 0xFE8E) &&
            ContainsGlyph(arabicLayout, 0xFEE1),
        "Arabic contextual forms were not selected");
    Require(arabicLayout.glyphs.size() == 4 &&
            arabicLayout.glyphs.front().codepoint == 0xFEE1 &&
            arabicLayout.glyphs.back().codepoint == 0xFEB3,
        "RTL visual glyph order is incorrect");

    InternationalLayoutOptions narrowRtl = options;
    narrowRtl.maximumWidth = arabic.GetGlyph(0xFEE1).advance * 2.0f;
    const auto overflowingRtl = LayoutInternational(
        family, U"\u0633\u0644\u0627\u0645", Pyramid::Math::Vec2::Zero, narrowRtl);
    const auto minimumRtlCaret = std::min_element(
        overflowingRtl.carets.begin(), overflowingRtl.carets.end(),
        [](const CaretStop& left, const CaretStop& right) { return left.x < right.x; });
    Require(minimumRtlCaret != overflowingRtl.carets.end() && minimumRtlCaret->x < 0.0f,
        "unwrapped RTL overflow did not preserve its negative visual extent");

    const CaretLocation logicalStart = GetCaretLocation(arabicLayout, 0);
    const CaretLocation logicalEnd = GetCaretLocation(arabicLayout, 4);
    Require(logicalStart.x > logicalEnd.x,
        "RTL caret mapping did not reverse logical edge positions");
    Require(HitTestInternational(arabicLayout, 0, logicalStart.x) == 0,
        "RTL hit testing did not recover the logical start");

    Require(MoveCaretVisual(arabicLayout, 4, true) == 3,
        "visual-right caret movement did not move backward logically in RTL text");
    Require(MoveCaretVisual(arabicLayout, 0, false) == 1,
        "visual-left caret movement did not move forward logically in RTL text");

    const auto selection = BuildSelectionSpans(arabicLayout, {1, 3});
    Require(!selection.empty() && selection.front().maximumX > selection.front().minimumX,
        "bidi selection geometry was not produced");

    const auto mixed = LayoutInternational(
        family,
        U"Kingdom 123 \u0645\u0645\u0644\u0643\u0629",
        Pyramid::Math::Vec2::Zero,
        options);
    Require(mixed.runs.size() >= 2,
        "mixed Arabic/Latin text did not produce multiple bidi runs");
    Require(mixed.fallbackGlyphs == 0,
        "mixed text did not resolve through the fallback family");

    InternationalLayoutOptions wrap;
    wrap.maximumWidth = latin.GetGlyph(U'A').advance * 3.2f;
    wrap.wrap = WrapMode::Word;
    const auto wrapped = LayoutInternational(
        family,
        U"AB CD \u4E00\u4E01",
        Pyramid::Math::Vec2::Zero,
        wrap);
    Require(wrapped.metrics.lineCount >= 3,
        "international word/CJK line opportunities were not applied");

    std::cout << "International text tests passed\n";
    return EXIT_SUCCESS;
}
