#include <Pyramid/Graphics/Renderer/ShaderPathResolver.hpp>
#include <Pyramid/Platform/RuntimePath.hpp>
#include <Pyramid/Util/Log.hpp>
#include <fstream>
#include <sstream>

namespace Pyramid
{
    namespace Renderer
    {
        namespace ShaderPathResolver
        {
            std::filesystem::path Resolve(const std::string& relativePath)
            {
                const std::filesystem::path resolved =
                    Platform::ResolveRuntimePath(relativePath);
                std::error_code error;
                return std::filesystem::is_regular_file(resolved, error) && !error
                    ? resolved : std::filesystem::path{};
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

