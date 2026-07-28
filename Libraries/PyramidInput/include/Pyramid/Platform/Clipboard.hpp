#pragma once

#include <Pyramid/Core/Prerequisites.hpp>

#include <string>
#include <string_view>

namespace Pyramid
{
    struct ClipboardTextResult
    {
        bool success = false;
        std::u32string text;
        std::string error;
    };

    /**
     * Platform-neutral Unicode clipboard service.
     *
     * UI widgets depend on this interface instead of native window APIs. Native
     * platform adapters own the implementation and normalize line endings to LF.
     */
    class Clipboard
    {
    public:
        virtual ~Clipboard() = default;

        virtual bool SetText(
            std::u32string_view text,
            std::string* error = nullptr) = 0;
        [[nodiscard]] virtual ClipboardTextResult GetText() const = 0;
    };

    namespace ClipboardEncoding
    {
        [[nodiscard]] bool Utf32ToUtf16(
            std::u32string_view input,
            std::u16string& output,
            std::size_t maximumCodeUnits = 4U * 1024U * 1024U,
            std::string* error = nullptr);

        [[nodiscard]] bool Utf16ToUtf32(
            std::u16string_view input,
            std::u32string& output,
            std::size_t maximumCodepoints = 4U * 1024U * 1024U,
            std::string* error = nullptr);
    } // namespace ClipboardEncoding
} // namespace Pyramid
