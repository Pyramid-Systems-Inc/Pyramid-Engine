#include <Pyramid/Model/Model.hpp>

namespace Pyramid::Model
{
    u32 ImportedModel::GetErrorCount() const
    {
        u32 count = 0;
        for (const auto& diagnostic : diagnostics)
        {
            count += diagnostic.IsError() ? 1u : 0u;
        }
        return count;
    }

    u32 ImportedModel::GetWarningCount() const
    {
        u32 count = 0;
        for (const auto& diagnostic : diagnostics)
        {
            count += diagnostic.IsError() ? 0u : 1u;
        }
        return count;
    }

    bool ImportedModel::IsValid() const
    {
        if (HasErrors() || primitives.empty())
        {
            return false;
        }

        for (const auto& primitive : primitives)
        {
            if (primitive.vertices.empty() || primitive.indices.empty() ||
                (primitive.indices.size() % 3u) != 0u)
            {
                return false;
            }
        }
        return true;
    }
}
