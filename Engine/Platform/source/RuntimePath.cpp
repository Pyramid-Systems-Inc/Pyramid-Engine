#include <Pyramid/Platform/RuntimePath.hpp>

#include <cstdlib>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace Pyramid::Platform
{
    namespace
    {
        std::filesystem::path NormalizeExistingPath(
            const std::filesystem::path& candidate)
        {
            std::error_code error;
            if (!std::filesystem::exists(candidate, error) || error)
            {
                return {};
            }

            error.clear();
            const std::filesystem::path canonical =
                std::filesystem::weakly_canonical(candidate, error);
            return error ? candidate.lexically_normal() : canonical;
        }


        std::filesystem::path CreateCacheDirectory(
            const std::filesystem::path& root,
            std::string_view applicationName)
        {
            if (root.empty() || applicationName.empty())
            {
                return {};
            }
            const std::filesystem::path directory =
                root / "Ruqoom" / std::filesystem::path(applicationName);
            std::error_code error;
            std::filesystem::create_directories(directory, error);
            return error ? std::filesystem::path{} : directory.lexically_normal();
        }
    } // namespace

    std::filesystem::path GetExecutableDirectory()
    {
#ifdef _WIN32
        std::vector<wchar_t> buffer(512U);
        while (buffer.size() <= 32768U)
        {
            ::SetLastError(ERROR_SUCCESS);
            const DWORD length = ::GetModuleFileNameW(
                nullptr,
                buffer.data(),
                static_cast<DWORD>(buffer.size()));
            if (length == 0U)
            {
                return {};
            }
            if (length < buffer.size())
            {
                return std::filesystem::path(
                    buffer.data(),
                    buffer.data() + length).parent_path();
            }
            buffer.resize(buffer.size() * 2U);
        }
        return {};
#elif defined(__linux__)
        std::vector<char> buffer(512U);
        while (buffer.size() <= 65536U)
        {
            const ssize_t length = ::readlink(
                "/proc/self/exe",
                buffer.data(),
                buffer.size());
            if (length < 0)
            {
                return {};
            }
            if (static_cast<std::size_t>(length) < buffer.size())
            {
                return std::filesystem::path(
                    buffer.data(),
                    buffer.data() + length).parent_path();
            }
            buffer.resize(buffer.size() * 2U);
        }
        return {};
#else
        return {};
#endif
    }

    std::filesystem::path GetUserCacheDirectory(std::string_view applicationName)
    {
#ifdef _WIN32
        const DWORD required = ::GetEnvironmentVariableW(L"LOCALAPPDATA", nullptr, 0U);
        if (required > 1U)
        {
            std::vector<wchar_t> buffer(required);
            const DWORD written = ::GetEnvironmentVariableW(
                L"LOCALAPPDATA", buffer.data(), static_cast<DWORD>(buffer.size()));
            if (written > 0U && written < buffer.size())
            {
                return CreateCacheDirectory(
                    std::filesystem::path(buffer.data(), buffer.data() + written),
                    applicationName);
            }
        }
#elif defined(__linux__)
        if (const char* xdg = std::getenv("XDG_CACHE_HOME"); xdg && *xdg)
        {
            return CreateCacheDirectory(std::filesystem::path(xdg), applicationName);
        }
        if (const char* home = std::getenv("HOME"); home && *home)
        {
            return CreateCacheDirectory(
                std::filesystem::path(home) / ".cache", applicationName);
        }
#endif
        std::error_code error;
        const std::filesystem::path temporary =
            std::filesystem::temp_directory_path(error);
        return error ? std::filesystem::path{} :
            CreateCacheDirectory(temporary, applicationName);
    }

    std::filesystem::path ResolveRuntimePath(const std::filesystem::path& path)
    {
        if (path.empty())
        {
            return {};
        }

        if (path.is_absolute())
        {
            const std::filesystem::path normalized = NormalizeExistingPath(path);
            return normalized.empty() ? path.lexically_normal() : normalized;
        }

        const std::filesystem::path executableDirectory = GetExecutableDirectory();
        if (!executableDirectory.empty())
        {
            const std::filesystem::path executableRelative = executableDirectory / path;
            const std::filesystem::path normalized =
                NormalizeExistingPath(executableRelative);
            if (!normalized.empty())
            {
                return normalized;
            }
        }

        std::error_code error;
        const std::filesystem::path workingDirectory =
            std::filesystem::current_path(error);
        if (!error)
        {
            const std::filesystem::path workingRelative = workingDirectory / path;
            const std::filesystem::path normalized = NormalizeExistingPath(workingRelative);
            if (!normalized.empty())
            {
                return normalized;
            }
        }

        return executableDirectory.empty()
            ? path.lexically_normal()
            : (executableDirectory / path).lexically_normal();
    }
} // namespace Pyramid::Platform
