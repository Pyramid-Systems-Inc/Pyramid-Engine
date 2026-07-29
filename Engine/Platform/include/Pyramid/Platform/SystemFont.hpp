#pragma once

#include <Pyramid/Core/Prerequisites.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace Pyramid::Platform
{
    struct SystemFontRequest
    {
        std::vector<std::string> preferredFamilies;
        u16 weight = 400;
        bool italic = false;
        u32 maximumBytes = 64U * 1024U * 1024U;
    };

    struct SystemFontData
    {
        std::string requestedFamily;
        std::string resolvedFamily;
        std::vector<u8> bytes;

        [[nodiscard]] bool IsValid() const
        {
            return !resolvedFamily.empty() && !bytes.empty();
        }
    };

    /**
     * Extracts a system-installed TrueType face through the host platform API.
     * Pyramid still owns parsing, shaping, rasterization, caching, and rendering;
     * the operating system is used only as a source of legally installed font data.
     */
    [[nodiscard]] bool LoadSystemFont(
        const SystemFontRequest& request,
        SystemFontData& output,
        std::string* error = nullptr);
} // namespace Pyramid::Platform
