#include <Pyramid/Platform/SystemFont.hpp>

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Pyramid::Platform
{
#ifdef _WIN32
    namespace
    {
        std::wstring Utf8ToWide(std::string_view text)
        {
            if (text.empty())
            {
                return {};
            }
            if (text.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
            {
                return {};
            }
            const int length = ::MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                text.data(),
                static_cast<int>(text.size()),
                nullptr,
                0);
            if (length <= 0)
            {
                return {};
            }
            std::wstring result(static_cast<std::size_t>(length), L'\0');
            if (::MultiByteToWideChar(
                    CP_UTF8,
                    MB_ERR_INVALID_CHARS,
                    text.data(),
                    static_cast<int>(text.size()),
                    result.data(),
                    length) != length)
            {
                return {};
            }
            return result;
        }

        std::string WideToUtf8(const wchar_t* text, int length)
        {
            if (!text || length <= 0)
            {
                return {};
            }
            const int bytes = ::WideCharToMultiByte(
                CP_UTF8, 0, text, length, nullptr, 0, nullptr, nullptr);
            if (bytes <= 0)
            {
                return {};
            }
            std::string result(static_cast<std::size_t>(bytes), '\0');
            if (::WideCharToMultiByte(
                    CP_UTF8,
                    0,
                    text,
                    length,
                    result.data(),
                    bytes,
                    nullptr,
                    nullptr) != bytes)
            {
                return {};
            }
            return result;
        }

        bool TryLoadFamily(
            HDC deviceContext,
            std::string_view family,
            const SystemFontRequest& request,
            SystemFontData& output)
        {
            const std::wstring wideFamily = Utf8ToWide(family);
            if (wideFamily.empty() || wideFamily.size() >= LF_FACESIZE)
            {
                return false;
            }

            LOGFONTW descriptor{};
            descriptor.lfHeight = -64;
            descriptor.lfWeight = static_cast<LONG>((std::max)(100U, (std::min)(900U,
                static_cast<u32>(request.weight))));
            descriptor.lfItalic = request.italic ? TRUE : FALSE;
            descriptor.lfCharSet = DEFAULT_CHARSET;
            descriptor.lfOutPrecision = OUT_TT_ONLY_PRECIS;
            descriptor.lfQuality = CLEARTYPE_QUALITY;
            std::copy(wideFamily.begin(), wideFamily.end(), descriptor.lfFaceName);
            descriptor.lfFaceName[wideFamily.size()] = L'\0';

            HFONT font = ::CreateFontIndirectW(&descriptor);
            if (!font)
            {
                return false;
            }
            HGDIOBJ previous = ::SelectObject(deviceContext, font);
            if (!previous || previous == HGDI_ERROR)
            {
                ::DeleteObject(font);
                return false;
            }

            const DWORD byteCount = ::GetFontData(deviceContext, 0U, 0U, nullptr, 0U);
            if (byteCount == GDI_ERROR || byteCount == 0U || byteCount > request.maximumBytes)
            {
                ::SelectObject(deviceContext, previous);
                ::DeleteObject(font);
                return false;
            }

            std::vector<u8> bytes(byteCount);
            const DWORD copied = ::GetFontData(
                deviceContext, 0U, 0U, bytes.data(), byteCount);
            std::array<wchar_t, LF_FACESIZE> resolved{};
            const int resolvedLength = ::GetTextFaceW(
                deviceContext,
                static_cast<int>(resolved.size()),
                resolved.data());

            ::SelectObject(deviceContext, previous);
            ::DeleteObject(font);
            if (copied == GDI_ERROR || copied != byteCount)
            {
                return false;
            }

            output.requestedFamily.assign(family);
            output.resolvedFamily = resolvedLength > 0
                ? WideToUtf8(resolved.data(), resolvedLength)
                : std::string(family);
            while (!output.resolvedFamily.empty() &&
                output.resolvedFamily.back() == '\0')
            {
                output.resolvedFamily.pop_back();
            }
            output.bytes = std::move(bytes);
            return output.IsValid();
        }
    } // namespace
#endif

    bool LoadSystemFont(
        const SystemFontRequest& request,
        SystemFontData& output,
        std::string* error)
    {
        output = {};
        if (error)
        {
            error->clear();
        }
        if (request.maximumBytes == 0U)
        {
            if (error)
            {
                *error = "system-font byte limit must be positive";
            }
            return false;
        }

#ifdef _WIN32
        HDC deviceContext = ::CreateCompatibleDC(nullptr);
        if (!deviceContext)
        {
            if (error)
            {
                *error = "Win32 could not create a font device context";
            }
            return false;
        }

        const std::array<std::string_view, 3> defaults = {
            "Segoe UI", "Tahoma", "Arial"};
        bool loaded = false;
        if (request.preferredFamilies.empty())
        {
            for (const std::string_view family : defaults)
            {
                if (TryLoadFamily(deviceContext, family, request, output))
                {
                    loaded = true;
                    break;
                }
            }
        }
        else
        {
            for (const std::string& family : request.preferredFamilies)
            {
                if (TryLoadFamily(deviceContext, family, request, output))
                {
                    loaded = true;
                    break;
                }
            }
        }
        ::DeleteDC(deviceContext);
        if (!loaded && error)
        {
            *error = "none of the requested Win32 TrueType families could be extracted";
        }
        return loaded;
#else
        (void)request;
        if (error)
        {
            *error = "system-font loading is currently implemented for Win32 only";
        }
        return false;
#endif
    }
} // namespace Pyramid::Platform
