#include <Pyramid/Platform/Clipboard.hpp>
#include <algorithm>

namespace Pyramid::ClipboardEncoding
{
    namespace
    {
        bool IsScalar(char32_t value)
        {
            return value <= 0x10ffffU && !(value >= 0xd800U && value <= 0xdfffU);
        }

        void SetError(std::string* error, const char* message)
        {
            if (error)
            {
                *error = message;
            }
        }
    } // namespace

    bool Utf32ToUtf16(
        std::u32string_view input,
        std::u16string& output,
        std::size_t maximumCodeUnits,
        std::string* error)
    {
        output.clear();
        if (error)
        {
            error->clear();
        }
        if (maximumCodeUnits == 0)
        {
            SetError(error, "UTF-16 output limit is zero");
            return false;
        }

        if (input.size() > maximumCodeUnits)
        {
            SetError(error, "clipboard text exceeds the UTF-16 output limit");
            return false;
        }

        output.reserve(input.size());
        for (char32_t codepoint : input)
        {
            if (!IsScalar(codepoint))
            {
                SetError(error, "clipboard text contains an invalid Unicode scalar value");
                output.clear();
                return false;
            }

            if (codepoint <= 0xffffU)
            {
                if (output.size() >= maximumCodeUnits)
                {
                    SetError(error, "clipboard text exceeds the UTF-16 output limit");
                    output.clear();
                    return false;
                }
                output.push_back(static_cast<char16_t>(codepoint));
            }
            else
            {
                if (maximumCodeUnits - output.size() < 2U)
                {
                    SetError(error, "clipboard text exceeds the UTF-16 output limit");
                    output.clear();
                    return false;
                }
                const char32_t adjusted = codepoint - 0x10000U;
                output.push_back(static_cast<char16_t>(0xd800U + (adjusted >> 10U)));
                output.push_back(static_cast<char16_t>(0xdc00U + (adjusted & 0x3ffU)));
            }
        }
        return true;
    }

    bool Utf16ToUtf32(
        std::u16string_view input,
        std::u32string& output,
        std::size_t maximumCodepoints,
        std::string* error)
    {
        output.clear();
        if (error)
        {
            error->clear();
        }
        if (maximumCodepoints == 0)
        {
            SetError(error, "UTF-32 output limit is zero");
            return false;
        }

        output.reserve((std::min)(input.size(), maximumCodepoints));
        for (std::size_t index = 0; index < input.size(); ++index)
        {
            if (output.size() >= maximumCodepoints)
            {
                SetError(error, "clipboard text exceeds the UTF-32 output limit");
                output.clear();
                return false;
            }

            const char16_t unit = input[index];
            if (unit >= 0xd800U && unit <= 0xdbffU)
            {
                if (index + 1U >= input.size())
                {
                    SetError(error, "clipboard UTF-16 ends with an unmatched high surrogate");
                    output.clear();
                    return false;
                }
                const char16_t low = input[++index];
                if (low < 0xdc00U || low > 0xdfffU)
                {
                    SetError(error, "clipboard UTF-16 contains an invalid surrogate pair");
                    output.clear();
                    return false;
                }
                const char32_t codepoint = 0x10000U +
                    ((static_cast<char32_t>(unit) - 0xd800U) << 10U) +
                    (static_cast<char32_t>(low) - 0xdc00U);
                output.push_back(codepoint);
            }
            else if (unit >= 0xdc00U && unit <= 0xdfffU)
            {
                SetError(error, "clipboard UTF-16 contains an unmatched low surrogate");
                output.clear();
                return false;
            }
            else
            {
                output.push_back(static_cast<char32_t>(unit));
            }
        }
        return true;
    }
} // namespace Pyramid::ClipboardEncoding
