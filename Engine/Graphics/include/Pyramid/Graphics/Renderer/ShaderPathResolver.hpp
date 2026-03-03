#pragma once

#include <string>
#include <filesystem>

namespace Pyramid
{
    namespace Renderer
    {
        namespace ShaderPathResolver
        {
            std::filesystem::path Resolve(const std::string& relativePath);
            std::string LoadTextFile(const std::string& relativePath);
        } // namespace ShaderPathResolver
    } // namespace Renderer
} // namespace Pyramid

