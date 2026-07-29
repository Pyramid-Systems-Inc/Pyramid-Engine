#include <Pyramid/Text/Text.hpp>

#include <algorithm>
#include <cctype>
#include <utility>

namespace Pyramid::Text
{
    namespace
    {
        constexpr char32_t kReplacement = U'\uFFFD';

        bool IsContinuation(unsigned char value)
        {
            return (value & 0xc0U) == 0x80U;
        }

        bool IsScalar(char32_t value)
        {
            return value <= 0x10ffffU && !(value >= 0xd800U && value <= 0xdfffU);
        }
    } // namespace

    Utf8DecodeResult DecodeUtf8(std::string_view utf8Text)
    {
        Utf8DecodeResult result;
        result.text.reserve(utf8Text.size());

        std::size_t index = 0;
        while (index < utf8Text.size())
        {
            const unsigned char lead = static_cast<unsigned char>(utf8Text[index]);
            if (lead < 0x80U)
            {
                result.text.push_back(static_cast<char32_t>(lead));
                ++index;
                continue;
            }

            std::size_t length = 0;
            char32_t codepoint = 0;
            char32_t minimum = 0;
            if ((lead & 0xe0U) == 0xc0U)
            {
                length = 2;
                codepoint = lead & 0x1fU;
                minimum = 0x80U;
            }
            else if ((lead & 0xf0U) == 0xe0U)
            {
                length = 3;
                codepoint = lead & 0x0fU;
                minimum = 0x800U;
            }
            else if ((lead & 0xf8U) == 0xf0U)
            {
                length = 4;
                codepoint = lead & 0x07U;
                minimum = 0x10000U;
            }
            else
            {
                result.text.push_back(kReplacement);
                ++result.invalidSequences;
                ++index;
                continue;
            }

            if (index + length > utf8Text.size())
            {
                result.text.push_back(kReplacement);
                ++result.invalidSequences;
                break;
            }

            bool valid = true;
            for (std::size_t continuation = 1; continuation < length; ++continuation)
            {
                const unsigned char value =
                    static_cast<unsigned char>(utf8Text[index + continuation]);
                if (!IsContinuation(value))
                {
                    valid = false;
                    break;
                }
                codepoint = (codepoint << 6U) | (value & 0x3fU);
            }

            if (!valid || codepoint < minimum || !IsScalar(codepoint))
            {
                result.text.push_back(kReplacement);
                ++result.invalidSequences;
                ++index;
                continue;
            }

            result.text.push_back(codepoint);
            index += length;
        }
        return result;
    }

    std::string EncodeUtf8(std::u32string_view text)
    {
        std::string result;
        result.reserve(text.size());
        for (char32_t codepoint : text)
        {
            if (!IsScalar(codepoint))
            {
                codepoint = kReplacement;
            }
            if (codepoint <= 0x7fU)
            {
                result.push_back(static_cast<char>(codepoint));
            }
            else if (codepoint <= 0x7ffU)
            {
                result.push_back(static_cast<char>(0xc0U | (codepoint >> 6U)));
                result.push_back(static_cast<char>(0x80U | (codepoint & 0x3fU)));
            }
            else if (codepoint <= 0xffffU)
            {
                result.push_back(static_cast<char>(0xe0U | (codepoint >> 12U)));
                result.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3fU)));
                result.push_back(static_cast<char>(0x80U | (codepoint & 0x3fU)));
            }
            else
            {
                result.push_back(static_cast<char>(0xf0U | (codepoint >> 18U)));
                result.push_back(static_cast<char>(0x80U | ((codepoint >> 12U) & 0x3fU)));
                result.push_back(static_cast<char>(0x80U | ((codepoint >> 6U) & 0x3fU)));
                result.push_back(static_cast<char>(0x80U | (codepoint & 0x3fU)));
            }
        }
        return result;
    }

    TextBuffer::TextBuffer(std::u32string text)
        : m_text(std::move(text))
        , m_cursor(m_text.size())
        , m_anchor(m_cursor)
    {
        Normalize();
    }

    void TextBuffer::SetText(std::u32string text)
    {
        m_text = std::move(text);
        Normalize();
        m_cursor = ClampIndex(m_cursor);
        m_anchor = ClampIndex(m_anchor);
    }

    std::string TextBuffer::GetUtf8() const
    {
        return EncodeUtf8(m_text);
    }

    void TextBuffer::SetMaximumCharacters(std::size_t maximum)
    {
        m_maximumCharacters = maximum;
        Normalize();
    }

    void TextBuffer::SetSingleLine(bool singleLine)
    {
        m_singleLine = singleLine;
        Normalize();
    }

    TextRange TextBuffer::GetSelection() const
    {
        return {(std::min)(m_cursor, m_anchor), (std::max)(m_cursor, m_anchor)};
    }

    std::u32string TextBuffer::GetSelectedText() const
    {
        const TextRange range = GetSelection();
        return range.Empty() ? std::u32string{} : m_text.substr(range.begin, range.Length());
    }

    void TextBuffer::SetCursor(std::size_t index, bool extendSelection)
    {
        m_cursor = ClampIndex(index);
        if (!extendSelection)
        {
            m_anchor = m_cursor;
        }
    }

    void TextBuffer::SetSelection(TextRange range)
    {
        range.begin = ClampIndex(range.begin);
        range.end = ClampIndex(range.end);
        if (range.begin > range.end)
        {
            std::swap(range.begin, range.end);
        }
        m_anchor = range.begin;
        m_cursor = range.end;
    }

    void TextBuffer::SelectAll()
    {
        m_anchor = 0;
        m_cursor = m_text.size();
    }

    void TextBuffer::ClearSelection()
    {
        m_anchor = m_cursor;
    }

    bool TextBuffer::SelectWordAt(std::size_t index)
    {
        if (m_text.empty())
        {
            return false;
        }
        index = (std::min)(index, m_text.size() - 1U);
        const bool word = IsWordCharacter(m_text[index]);
        std::size_t begin = index;
        std::size_t end = index + 1U;
        while (begin > 0 && IsWordCharacter(m_text[begin - 1U]) == word &&
            m_text[begin - 1U] != U'\n')
        {
            --begin;
        }
        while (end < m_text.size() && IsWordCharacter(m_text[end]) == word &&
            m_text[end] != U'\n')
        {
            ++end;
        }
        SetSelection({begin, end});
        return true;
    }

    bool TextBuffer::SelectLineAt(std::size_t index)
    {
        index = ClampIndex(index);
        const std::size_t begin = FindLineStart(index);
        std::size_t end = FindLineEnd(index);
        if (end < m_text.size() && m_text[end] == U'\n')
        {
            ++end;
        }
        SetSelection({begin, end});
        return true;
    }

    bool TextBuffer::Insert(std::u32string_view text)
    {
        if (m_readOnly)
        {
            return false;
        }

        std::u32string filtered;
        filtered.reserve(text.size());
        for (char32_t codepoint : text)
        {
            if (!IsScalar(codepoint))
            {
                codepoint = kReplacement;
            }
            if (m_singleLine && (codepoint == U'\n' || codepoint == U'\r'))
            {
                codepoint = U' ';
            }
            if (codepoint == U'\r')
            {
                codepoint = U'\n';
            }
            filtered.push_back(codepoint);
        }

        const TextRange selection = GetSelection();
        const std::size_t available = m_maximumCharacters >= m_text.size() - selection.Length()
            ? m_maximumCharacters - (m_text.size() - selection.Length())
            : 0;
        if (filtered.size() > available)
        {
            std::size_t complete = available;
            if (!IsGraphemeBoundary(filtered, complete))
            {
                complete = PreviousGraphemeBoundary(filtered, complete);
            }
            filtered.resize(complete);
        }
        if (filtered.empty() && selection.Empty())
        {
            return false;
        }

        m_text.replace(selection.begin, selection.Length(), filtered);
        m_cursor = selection.begin + filtered.size();
        m_anchor = m_cursor;
        return true;
    }

    bool TextBuffer::DeleteSelection()
    {
        if (m_readOnly || !HasSelection())
        {
            return false;
        }
        const TextRange range = GetSelection();
        m_text.erase(range.begin, range.Length());
        m_cursor = range.begin;
        m_anchor = m_cursor;
        return true;
    }

    bool TextBuffer::Backspace()
    {
        if (DeleteSelection())
        {
            return true;
        }
        if (m_readOnly || m_cursor == 0)
        {
            return false;
        }
        const std::size_t previous = PreviousGraphemeBoundary(m_text, m_cursor);
        m_text.erase(previous, m_cursor - previous);
        m_cursor = previous;
        m_anchor = m_cursor;
        return true;
    }

    bool TextBuffer::DeleteForward()
    {
        if (DeleteSelection())
        {
            return true;
        }
        if (m_readOnly || m_cursor >= m_text.size())
        {
            return false;
        }
        const std::size_t next = NextGraphemeBoundary(m_text, m_cursor);
        m_text.erase(m_cursor, next - m_cursor);
        m_anchor = m_cursor;
        return true;
    }

    bool TextBuffer::MoveCursor(CursorMove move, bool extendSelection)
    {
        std::size_t target = m_cursor;
        switch (move)
        {
            case CursorMove::PreviousCodepoint:
                target = PreviousGraphemeBoundary(m_text, target);
                break;
            case CursorMove::NextCodepoint:
                target = NextGraphemeBoundary(m_text, target);
                break;
            case CursorMove::PreviousWord:
                while (target > 0 && !IsWordCharacter(m_text[target - 1U]))
                {
                    --target;
                }
                while (target > 0 && IsWordCharacter(m_text[target - 1U]))
                {
                    --target;
                }
                break;
            case CursorMove::NextWord:
                while (target < m_text.size() && IsWordCharacter(m_text[target]))
                {
                    ++target;
                }
                while (target < m_text.size() && !IsWordCharacter(m_text[target]))
                {
                    ++target;
                }
                break;
            case CursorMove::LineStart:
                target = FindLineStart(target);
                break;
            case CursorMove::LineEnd:
                target = FindLineEnd(target);
                break;
            case CursorMove::DocumentStart:
                target = 0;
                break;
            case CursorMove::DocumentEnd:
                target = m_text.size();
                break;
            case CursorMove::LineUp:
            {
                const std::size_t start = FindLineStart(target);
                if (start == 0)
                {
                    target = 0;
                    break;
                }
                const std::size_t column = target - start;
                const std::size_t previousEnd = start - 1U;
                const std::size_t previousStart = FindLineStart(previousEnd);
                target = (std::min)(previousStart + column, previousEnd);
                break;
            }
            case CursorMove::LineDown:
            {
                const std::size_t start = FindLineStart(target);
                const std::size_t end = FindLineEnd(target);
                if (end >= m_text.size())
                {
                    target = m_text.size();
                    break;
                }
                const std::size_t column = target - start;
                const std::size_t nextStart = end + 1U;
                const std::size_t nextEnd = FindLineEnd(nextStart);
                target = (std::min)(nextStart + column, nextEnd);
                break;
            }
        }
        const bool changed = target != m_cursor || (!extendSelection && m_anchor != target);
        SetCursor(target, extendSelection);
        return changed;
    }

    std::size_t TextBuffer::ClampIndex(std::size_t index) const
    {
        index = (std::min)(index, m_text.size());
        if (index == 0 || index == m_text.size() || IsGraphemeBoundary(m_text, index))
        {
            return index;
        }
        return PreviousGraphemeBoundary(m_text, index);
    }

    std::size_t TextBuffer::FindLineStart(std::size_t index) const
    {
        index = ClampIndex(index);
        while (index > 0 && m_text[index - 1U] != U'\n')
        {
            --index;
        }
        return index;
    }

    std::size_t TextBuffer::FindLineEnd(std::size_t index) const
    {
        index = ClampIndex(index);
        while (index < m_text.size() && m_text[index] != U'\n')
        {
            ++index;
        }
        return index;
    }

    bool TextBuffer::IsWordCharacter(char32_t codepoint)
    {
        if (codepoint == U'_')
        {
            return true;
        }
        if (codepoint < 128U)
        {
            return std::isalnum(static_cast<unsigned char>(codepoint)) != 0;
        }
        return codepoint != U' ' && codepoint != U'\t' && codepoint != U'\n' &&
            codepoint != U'\r';
    }

    void TextBuffer::Normalize()
    {
        std::u32string normalized;
        normalized.reserve(m_text.size());
        for (char32_t codepoint : m_text)
        {
            if (!IsScalar(codepoint))
            {
                codepoint = kReplacement;
            }
            if (codepoint == U'\r')
            {
                codepoint = U'\n';
            }
            if (m_singleLine && codepoint == U'\n')
            {
                codepoint = U' ';
            }
            normalized.push_back(codepoint);
        }
        if (normalized.size() > m_maximumCharacters)
        {
            std::size_t complete = m_maximumCharacters;
            if (!IsGraphemeBoundary(normalized, complete))
            {
                complete = PreviousGraphemeBoundary(normalized, complete);
            }
            normalized.resize(complete);
        }
        m_text.swap(normalized);
        m_cursor = ClampIndex(m_cursor);
        m_anchor = ClampIndex(m_anchor);
    }
} // namespace Pyramid::Text
