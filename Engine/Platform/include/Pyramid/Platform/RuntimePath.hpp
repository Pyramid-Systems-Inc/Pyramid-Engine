#pragma once

#include <filesystem>
#include <string_view>

namespace Pyramid::Platform
{
    /** Returns the directory containing the running executable. */
    [[nodiscard]] std::filesystem::path GetExecutableDirectory();

    /**
     * Returns a writable per-user cache directory for a Pyramid application.
     * The directory is created on demand. An empty result indicates that no
     * suitable writable cache location could be established.
     */
    [[nodiscard]] std::filesystem::path GetUserCacheDirectory(
        std::string_view applicationName = "PyramidEngine");

    /**
     * Resolves a runtime asset path independently of the process working directory.
     *
     * Relative paths prefer the directory containing the executable, because CMake
     * and install rules deploy runtime assets beside Pyramid executables. The
     * current working directory remains a development fallback. Absolute paths are
     * preserved. Existing results are normalized when possible.
     */
    [[nodiscard]] std::filesystem::path ResolveRuntimePath(
        const std::filesystem::path& path);
} // namespace Pyramid::Platform
