#include <Pyramid/Platform/RuntimePath.hpp>
#include <Pyramid/Graphics/Renderer/ShaderPathResolver.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    class CurrentDirectoryGuard final
    {
    public:
        CurrentDirectoryGuard()
            : m_original(std::filesystem::current_path())
        {
        }

        ~CurrentDirectoryGuard()
        {
            std::error_code ignored;
            std::filesystem::current_path(m_original, ignored);
        }

    private:
        std::filesystem::path m_original;
    };

    struct DirectoryCleanup final
    {
        std::filesystem::path path;

        ~DirectoryCleanup()
        {
            std::error_code ignored;
            std::filesystem::remove_all(path, ignored);
        }
    };

    bool SamePath(
        const std::filesystem::path& left,
        const std::filesystem::path& right)
    {
        std::error_code error;
        const bool equivalent = std::filesystem::equivalent(left, right, error);
        if (!error)
        {
            return equivalent;
        }

        error.clear();
        const auto normalizedLeft = std::filesystem::weakly_canonical(left, error);
        if (error)
        {
            return false;
        }
        error.clear();
        const auto normalizedRight = std::filesystem::weakly_canonical(right, error);
        return !error && normalizedLeft == normalizedRight;
    }
} // namespace

int main()
{
    namespace fs = std::filesystem;

    try
    {
        const fs::path executableDirectory = Pyramid::Platform::GetExecutableDirectory();
        Require(!executableDirectory.empty(), "executable directory was not resolved");

        const std::string token = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        const fs::path fixtureName = "PyramidRuntimePathTests-" + token;
        const fs::path userCache = Pyramid::Platform::GetUserCacheDirectory(
            std::string("PyramidRuntimePathTests-") + token);
        Require(!userCache.empty(), "per-user cache directory was not resolved");
        const DirectoryCleanup cacheCleanup{userCache};
        Require(fs::is_directory(userCache), "per-user cache directory was not created");
        const fs::path executableFixtureDirectory = executableDirectory / fixtureName;
        const fs::path executableAsset = executableFixtureDirectory / "asset.bin";
        const fs::path unrelatedWorkingDirectory =
            fs::temp_directory_path() / ("PyramidRuntimePathWorking-" + token);
        const DirectoryCleanup executableCleanup{executableFixtureDirectory};
        const DirectoryCleanup workingCleanup{unrelatedWorkingDirectory};

        fs::create_directories(executableFixtureDirectory);
        fs::create_directories(unrelatedWorkingDirectory);
        {
            std::ofstream stream(executableAsset, std::ios::binary);
            stream << "executable-relative";
        }

        CurrentDirectoryGuard currentDirectoryGuard;
        fs::current_path(unrelatedWorkingDirectory);

        const fs::path resolvedExecutableAsset =
            Pyramid::Platform::ResolveRuntimePath(fixtureName / "asset.bin");
        Require(
            SamePath(resolvedExecutableAsset, executableAsset),
            "runtime path did not prefer the executable directory");

        const fs::path shaderRelative = fixtureName / "fixture.vert";
        const fs::path executableShader = executableDirectory / shaderRelative;
        const fs::path decoyShader = unrelatedWorkingDirectory / shaderRelative;
        fs::create_directories(decoyShader.parent_path());
        {
            std::ofstream stream(executableShader);
            stream << "executable shader";
            Require(static_cast<bool>(stream), "executable shader fixture could not be written");
        }
        {
            std::ofstream stream(decoyShader);
            stream << "wrong working-directory shader";
            Require(static_cast<bool>(stream), "working-directory shader decoy could not be written");
        }
        Require(
            SamePath(Pyramid::Renderer::ShaderPathResolver::Resolve(shaderRelative.generic_string()),
                executableShader),
            "shader path preferred a working-directory decoy over the executable asset");
        Require(
            Pyramid::Renderer::ShaderPathResolver::LoadTextFile(shaderRelative.generic_string()) ==
                "executable shader",
            "shader source did not load from the executable directory");
        fs::remove(decoyShader);
        Require(
            SamePath(Pyramid::Renderer::ShaderPathResolver::Resolve(shaderRelative.generic_string()),
                executableShader),
            "shader path required a working-directory copy");
        const std::string missingShader = (fixtureName / "missing.vert").generic_string();
        Require(Pyramid::Renderer::ShaderPathResolver::Resolve(missingShader).empty(),
            "missing shader path did not fail explicitly");
        Require(Pyramid::Renderer::ShaderPathResolver::LoadTextFile(missingShader).empty(),
            "missing shader source did not fail explicitly");

        const fs::path absoluteAsset =
            Pyramid::Platform::ResolveRuntimePath(executableAsset);
        Require(SamePath(absoluteAsset, executableAsset), "absolute path was not preserved");

        const fs::path workingAsset = unrelatedWorkingDirectory / "working-only.bin";
        {
            std::ofstream stream(workingAsset, std::ios::binary);
            stream << "working-relative";
        }
        const fs::path resolvedWorkingAsset =
            Pyramid::Platform::ResolveRuntimePath("working-only.bin");
        Require(
            SamePath(resolvedWorkingAsset, workingAsset),
            "current-working-directory fallback did not resolve");

        const fs::path missingRelative = fixtureName / "missing.bin";
        const fs::path missingResolved =
            Pyramid::Platform::ResolveRuntimePath(missingRelative);
        Require(
            missingResolved.lexically_normal() ==
                (executableDirectory / missingRelative).lexically_normal(),
            "missing runtime path did not report the executable-relative candidate");

    }
    catch (const std::exception& error)
    {
        std::cerr << "RuntimePathTests failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "RuntimePathTests passed\n";
    return 0;
}
