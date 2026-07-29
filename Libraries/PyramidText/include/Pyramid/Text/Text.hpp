#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Math/Vec2.hpp>
#include <Pyramid/Font/Font.hpp>

#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace Pyramid::Text
{

    struct Utf8DecodeResult
    {
        std::u32string text;
        u32 invalidSequences = 0;
    };

    [[nodiscard]] Utf8DecodeResult DecodeUtf8(std::string_view utf8Text);
    [[nodiscard]] std::string EncodeUtf8(std::u32string_view text);

    struct TextRange
    {
        std::size_t begin = 0;
        std::size_t end = 0;

        [[nodiscard]] bool Empty() const { return begin == end; }
        [[nodiscard]] std::size_t Length() const { return end >= begin ? end - begin : 0; }
    };

    /** Returns extended grapheme-cluster ranges using the owned Unicode segmenter. */
    [[nodiscard]] std::vector<TextRange> SegmentGraphemes(std::u32string_view text);
    [[nodiscard]] bool IsGraphemeBoundary(std::u32string_view text, std::size_t index);
    [[nodiscard]] std::size_t PreviousGraphemeBoundary(
        std::u32string_view text,
        std::size_t index);
    [[nodiscard]] std::size_t NextGraphemeBoundary(
        std::u32string_view text,
        std::size_t index);

    enum class CursorMove : u8
    {
        PreviousCodepoint = 0,
        NextCodepoint,
        PreviousWord,
        NextWord,
        LineStart,
        LineEnd,
        DocumentStart,
        DocumentEnd,
        LineUp,
        LineDown
    };

    /**
     * Renderer-independent Unicode editing model.
     *
     * Text is stored as Unicode scalar values so cursor and selection indices are
     * code-point indices rather than UTF-8 byte offsets. UI, console and editor
     * widgets share this model without depending on a native window backend.
     */
    class TextBuffer final
    {
    public:
        TextBuffer() = default;
        explicit TextBuffer(std::u32string text);

        void SetText(std::u32string text);
        [[nodiscard]] const std::u32string& GetText() const { return m_text; }
        [[nodiscard]] std::string GetUtf8() const;
        [[nodiscard]] std::size_t Size() const { return m_text.size(); }
        [[nodiscard]] bool Empty() const { return m_text.empty(); }

        void SetMaximumCharacters(std::size_t maximum);
        [[nodiscard]] std::size_t GetMaximumCharacters() const { return m_maximumCharacters; }
        void SetSingleLine(bool singleLine);
        [[nodiscard]] bool IsSingleLine() const { return m_singleLine; }
        void SetReadOnly(bool readOnly) { m_readOnly = readOnly; }
        [[nodiscard]] bool IsReadOnly() const { return m_readOnly; }

        [[nodiscard]] std::size_t GetCursor() const { return m_cursor; }
        [[nodiscard]] TextRange GetSelection() const;
        [[nodiscard]] bool HasSelection() const { return m_cursor != m_anchor; }
        [[nodiscard]] std::u32string GetSelectedText() const;

        void SetCursor(std::size_t index, bool extendSelection = false);
        void SetSelection(TextRange range);
        void SelectAll();
        void ClearSelection();
        [[nodiscard]] bool SelectWordAt(std::size_t index);
        [[nodiscard]] bool SelectLineAt(std::size_t index);

        [[nodiscard]] bool Insert(std::u32string_view text);
        [[nodiscard]] bool Backspace();
        [[nodiscard]] bool DeleteForward();
        [[nodiscard]] bool DeleteSelection();
        [[nodiscard]] bool MoveCursor(CursorMove move, bool extendSelection = false);

    private:
        [[nodiscard]] std::size_t ClampIndex(std::size_t index) const;
        [[nodiscard]] std::size_t FindLineStart(std::size_t index) const;
        [[nodiscard]] std::size_t FindLineEnd(std::size_t index) const;
        [[nodiscard]] static bool IsWordCharacter(char32_t codepoint);
        void Normalize();

        std::u32string m_text;
        std::size_t m_cursor = 0;
        std::size_t m_anchor = 0;
        std::size_t m_maximumCharacters = (std::numeric_limits<std::size_t>::max)();
        bool m_singleLine = false;
        bool m_readOnly = false;
    };
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
        std::string familyName;
        u32 width = 0;
        u32 height = 0;
        f32 pixelHeight = 0.0f;
        f32 lineHeight = 0.0f;
        std::vector<u8> rgbaPixels;
        std::vector<Glyph> glyphs;
        std::vector<Font::BakedKerning> kerning;
        char32_t fallbackCodepoint = U'?';
        Math::Vec2 whitePixelUv = Math::Vec2::Zero;

        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] bool HasGlyph(char32_t codepoint) const;
        [[nodiscard]] const Glyph& GetGlyph(char32_t codepoint) const;
        [[nodiscard]] f32 GetKerning(char32_t left, char32_t right) const;
    };

    struct FontFamilyGlyphSource
    {
        char32_t codepoint = 0;
        u32 fontIndex = 0;
    };

    /**
     * Ordered fallback family compiled into one renderer-ready atlas. Earlier
     * fonts win duplicate code points; missing glyphs resolve through the first
     * font's replacement glyph.
     */
    struct FontFamily
    {
        std::vector<std::string> familyNames;
        FontAtlas atlas;
        std::vector<FontFamilyGlyphSource> glyphSources;

        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] u32 ResolveFontIndex(char32_t codepoint) const;
    };

    [[nodiscard]] bool BuildFontFamily(
        const std::vector<FontAtlas>& fonts,
        FontFamily& output,
        std::string* error = nullptr);

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
        u32 fallbackGlyphs = 0;
    };

    enum class TextDirection : u8
    {
        Auto = 0,
        LeftToRight,
        RightToLeft
    };

    enum class ResolvedDirection : u8
    {
        LeftToRight = 0,
        RightToLeft
    };

    struct InternationalLayoutOptions
    {
        f32 scale = 1.0f;
        f32 maximumWidth = 0.0f;
        f32 lineSpacing = 0.0f;
        u32 tabWidth = 4;
        HorizontalAlignment alignment = HorizontalAlignment::Left;
        WrapMode wrap = WrapMode::None;
        TextDirection direction = TextDirection::Auto;
        char32_t maskCharacter = 0;

        [[nodiscard]] bool IsValid() const;
    };

    struct BidiRun
    {
        TextRange logicalRange;
        u32 lineIndex = 0;
        ResolvedDirection direction = ResolvedDirection::LeftToRight;
    };

    struct VisualCluster
    {
        TextRange logicalRange;
        u32 lineIndex = 0;
        f32 minimumX = 0.0f;
        f32 maximumX = 0.0f;
        ResolvedDirection direction = ResolvedDirection::LeftToRight;
        u32 firstGlyph = 0;
        u32 glyphCount = 0;
    };

    struct CaretStop
    {
        std::size_t logicalIndex = 0;
        u32 lineIndex = 0;
        f32 x = 0.0f;
    };

    struct CaretLocation
    {
        u32 lineIndex = 0;
        f32 x = 0.0f;
    };

    struct SelectionSpan
    {
        u32 lineIndex = 0;
        f32 minimumX = 0.0f;
        f32 maximumX = 0.0f;
    };

    struct InternationalLayoutResult
    {
        TextMetrics metrics;
        std::vector<GlyphQuad> glyphs;
        std::vector<LineMetrics> lines;
        std::vector<BidiRun> runs;
        std::vector<VisualCluster> clusters;
        std::vector<CaretStop> carets;
        ResolvedDirection paragraphDirection = ResolvedDirection::LeftToRight;
        u32 invalidUtf8Sequences = 0;
        u32 fallbackGlyphs = 0;
    };

    [[nodiscard]] InternationalLayoutResult LayoutInternational(
        const FontAtlas& font,
        std::u32string_view text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options = {});

    [[nodiscard]] InternationalLayoutResult LayoutInternational(
        const FontFamily& family,
        std::u32string_view text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options = {});

    [[nodiscard]] InternationalLayoutResult LayoutInternationalUtf8(
        const FontAtlas& font,
        std::string_view utf8Text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options = {});

    [[nodiscard]] InternationalLayoutResult LayoutInternationalUtf8(
        const FontFamily& family,
        std::string_view utf8Text,
        const Math::Vec2& origin,
        const InternationalLayoutOptions& options = {});

    [[nodiscard]] CaretLocation GetCaretLocation(
        const InternationalLayoutResult& layout,
        std::size_t logicalIndex);
    [[nodiscard]] std::size_t HitTestInternational(
        const InternationalLayoutResult& layout,
        u32 lineIndex,
        f32 x);
    [[nodiscard]] std::size_t MoveCaretVisual(
        const InternationalLayoutResult& layout,
        std::size_t logicalIndex,
        bool moveRight);
    [[nodiscard]] std::vector<SelectionSpan> BuildSelectionSpans(
        const InternationalLayoutResult& layout,
        TextRange selection);

    /**
     * Built-in deterministic debug font. It is intentionally small and ASCII-only;
     * unsupported Unicode code points resolve to the replacement glyph. UTF-8
     * decoding, wrapping and alignment are nevertheless handled by the owned text
     * layout layer so game/editor font backends can reuse the same layout contract.
     */
    [[nodiscard]] FontAtlas CreateDebugFontAtlas();
    [[nodiscard]] FontAtlas CreateFontAtlas(const Font::BakedFont& font);
    [[nodiscard]] bool LoadFontAtlas(
        std::string_view processedFontPath,
        FontAtlas& output,
        std::string* error = nullptr);

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
