#include <Pyramid/Graphics/Renderer/ShaderPathResolver.hpp>
#include <Pyramid/Util/Log.hpp>
#include <fstream>
#include <sstream>
#include <vector>

namespace Pyramid
{
    namespace Renderer
    {
        namespace ShaderPathResolver
        {
            std::filesystem::path Resolve(const std::string& relativePath)
            {
                namespace fs = std::filesystem;

                std::vector<fs::path> candidates;
                candidates.emplace_back(fs::path(relativePath));
                candidates.emplace_back(fs::current_path() / relativePath);
#ifdef PYRAMID_SOURCE_DIR
                candidates.emplace_back(fs::path(PYRAMID_SOURCE_DIR) / relativePath);
                candidates.emplace_back(fs::path(PYRAMID_SOURCE_DIR) / "Engine" / "Graphics" / "shaders" / fs::path(relativePath).filename());
#endif

                for (const auto& candidate : candidates)
                {
                    std::error_code ec;
                    if (fs::exists(candidate, ec))
                    {
                        return fs::weakly_canonical(candidate, ec);
                    }
                }

                return {};
            }

            std::string LoadTextFile(const std::string& relativePath)
            {
                const std::filesystem::path resolvedPath = Resolve(relativePath);
                if (resolvedPath.empty())
                {
                    PYRAMID_LOG_ERROR("Failed to resolve shader path: ", relativePath);
                    return {};
                }

                std::ifstream file(resolvedPath);
                if (!file.is_open())
                {
                    PYRAMID_LOG_ERROR("Failed to open shader file: ", resolvedPath.string());
                    return {};
                }

                std::stringstream stream;
                stream << file.rdbuf();
                return stream.str();
            }
        } // namespace ShaderPathResolver
    } // namespace Renderer
} // namespace Pyramid

