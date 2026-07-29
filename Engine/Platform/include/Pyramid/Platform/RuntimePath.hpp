#pragma once

#include <filesystem>

namespace Pyramid::Platform
{
    /** Returns the directory containing the running executable. */
    [[nodiscard]] std::filesystem::path GetExecutableDirectory();

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
