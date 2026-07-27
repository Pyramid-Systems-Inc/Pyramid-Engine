#include <Pyramid/Graphics/Model/ModelResourceImporter.hpp>

#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Graphics/Geometry/Mesh.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>

#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace Pyramid
{
    namespace
    {
        struct PackedModelVertex
        {
            f32 position[3];
            f32 normal[3];
            f32 texCoord[2];
            f32 color[3];
        };

        struct PreparedPrimitive
        {
            std::vector<PackedModelVertex> vertices;
            std::vector<u32> indices;
            MeshSpecification specification;
            MeshAssetId contentId;
            u32 primitiveIndex = 0;
            i32 materialIndex = -1;
        };

        struct RollbackRecord
        {
            MeshAssetId stableId;
            MeshAssetId contentId;
            bool stableExisted = false;
            bool contentExisted = false;
        };

        bool IsFinite(const Model::ModelVertex& vertex)
        {
            return std::isfinite(vertex.position.x) &&
                std::isfinite(vertex.position.y) &&
                std::isfinite(vertex.position.z) &&
                std::isfinite(vertex.normal.x) &&
                std::isfinite(vertex.normal.y) &&
                std::isfinite(vertex.normal.z) &&
                std::isfinite(vertex.texCoord.x) &&
                std::isfinite(vertex.texCoord.y) &&
                std::isfinite(vertex.color.x) &&
                std::isfinite(vertex.color.y) &&
                std::isfinite(vertex.color.z) &&
                std::isfinite(vertex.color.w);
        }

        void AddError(
            ModelMeshImportResult& result,
            const Model::ImportedModel& model,
            std::string message)
        {
            result.diagnostics.push_back({
                Model::ImportDiagnosticSeverity::Error,
                0,
                model.sourcePath,
                std::move(message)});
        }

        std::string MakeAssetName(
            const Model::ImportedModel& model,
            const ModelMeshImportOptions& options,
            const Model::ImportedPrimitive& primitive,
            u32 primitiveIndex)
        {
            std::string prefix = options.assetPrefix.empty()
                ? model.sourcePath
                : options.assetPrefix;
            if (prefix.empty())
            {
                prefix = "memory-model";
            }
            return prefix + "/primitive/" + std::to_string(primitiveIndex) + "/" +
                (primitive.name.empty() ? "unnamed" : primitive.name);
        }

        bool PreparePrimitive(
            const Model::ImportedModel& model,
            const ModelMeshImportOptions& options,
            u32 primitiveIndex,
            PreparedPrimitive& prepared,
            ModelMeshImportResult& result)
        {
            const auto& primitive = model.primitives[primitiveIndex];
            if (primitive.vertices.empty() || primitive.indices.empty() ||
                (primitive.indices.size() % 3u) != 0u)
            {
                AddError(result, model, "primitive " + std::to_string(primitiveIndex) +
                    " does not contain indexed triangles");
                return false;
            }
            if (primitive.vertices.size() > std::numeric_limits<u32>::max() ||
                primitive.indices.size() > std::numeric_limits<u32>::max() ||
                primitive.vertices.size() >
                    std::numeric_limits<u32>::max() / sizeof(PackedModelVertex))
            {
                AddError(result, model, "primitive " + std::to_string(primitiveIndex) +
                    " exceeds mesh size limits");
                return false;
            }

            prepared.vertices.reserve(primitive.vertices.size());
            for (const auto& source : primitive.vertices)
            {
                if (!IsFinite(source))
                {
                    AddError(result, model, "primitive " + std::to_string(primitiveIndex) +
                        " contains non-finite vertex data");
                    return false;
                }
                prepared.vertices.push_back({
                    {source.position.x, source.position.y, source.position.z},
                    {source.normal.x, source.normal.y, source.normal.z},
                    {source.texCoord.x, source.texCoord.y},
                    {source.color.x, source.color.y, source.color.z}});
            }

            for (u32 index : primitive.indices)
            {
                if (index >= primitive.vertices.size())
                {
                    AddError(result, model, "primitive " + std::to_string(primitiveIndex) +
                        " contains an out-of-range index");
                    return false;
                }
            }
            prepared.indices = primitive.indices;
            prepared.primitiveIndex = primitiveIndex;
            prepared.materialIndex = primitive.materialIndex;

            auto& specification = prepared.specification;
            specification.vertexData = prepared.vertices.data();
            specification.vertexDataSize = static_cast<u32>(
                prepared.vertices.size() * sizeof(PackedModelVertex));
            specification.vertexCount = static_cast<u32>(prepared.vertices.size());
            specification.layout = {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float3, "a_Normal"},
                {ShaderDataType::Float2, "a_TexCoord"},
                {ShaderDataType::Float3, "a_Color"}};
            specification.indexData = prepared.indices.data();
            specification.indexCount = static_cast<u32>(prepared.indices.size());
            specification.topology = PrimitiveTopology::Triangles;
            specification.name = primitive.name;
            specification.assetId = MeshAssetId::FromString(
                MakeAssetName(model, options, primitive, primitiveIndex));
            prepared.contentId = Mesh::CalculateContentId(specification);
            if (!prepared.contentId.IsValid())
            {
                AddError(result, model, "primitive " + std::to_string(primitiveIndex) +
                    " could not produce a valid mesh specification");
                return false;
            }
            return true;
        }

        void RollBack(
            MeshCache& cache,
            const std::vector<RollbackRecord>& records)
        {
            for (auto iterator = records.rbegin(); iterator != records.rend(); ++iterator)
            {
                if (!iterator->stableExisted && iterator->stableId != iterator->contentId)
                {
                    cache.RemoveAlias(iterator->stableId);
                }
                if (!iterator->contentExisted)
                {
                    cache.Evict(iterator->contentId);
                }
            }
        }
    }

    u32 ModelMeshImportResult::GetErrorCount() const
    {
        u32 count = 0;
        for (const auto& diagnostic : diagnostics)
        {
            count += diagnostic.IsError() ? 1u : 0u;
        }
        return count;
    }

    bool ModelMeshImportResult::IsSuccess() const
    {
        return GetErrorCount() == 0 && !meshes.empty();
    }

    ModelMeshImportResult ModelResourceImporter::UploadMeshes(
        MeshCache& cache,
        const Model::ImportedModel& model,
        const ModelMeshImportOptions& options)
    {
        ModelMeshImportResult result;
        result.diagnostics = model.diagnostics;
        if (!model.IsValid())
        {
            AddError(result, model, "model is not valid and cannot be uploaded");
            return result;
        }

        std::vector<PreparedPrimitive> prepared(model.primitives.size());
        for (u32 index = 0; index < model.primitives.size(); ++index)
        {
            if (!PreparePrimitive(model, options, index, prepared[index], result))
            {
                return result;
            }
        }

        for (const auto& primitive : prepared)
        {
            const auto existing = cache.Find(primitive.specification.assetId);
            if (existing && existing->GetContentId() != primitive.contentId)
            {
                AddError(result, model, "stable mesh identifier conflicts with resident geometry for primitive " +
                    std::to_string(primitive.primitiveIndex));
                return result;
            }
        }

        std::vector<RollbackRecord> rollback;
        rollback.reserve(prepared.size());
        result.meshes.reserve(prepared.size());

        for (auto& primitive : prepared)
        {
            RollbackRecord record;
            record.stableId = primitive.specification.assetId;
            record.contentId = primitive.contentId;
            record.stableExisted = cache.Contains(record.stableId);
            record.contentExisted = cache.Contains(record.contentId);

            auto mesh = cache.GetOrCreate(primitive.specification);
            if (!mesh)
            {
                result.meshes.clear();
                RollBack(cache, rollback);
                AddError(result, model, "graphics upload failed for primitive " +
                    std::to_string(primitive.primitiveIndex));
                return result;
            }

            rollback.push_back(record);
            result.meshes.push_back({
                primitive.primitiveIndex,
                primitive.materialIndex,
                MeshHandle::FromParts(
                    record.stableId,
                    cache.GetGeneration(record.stableId))});
        }
        return result;
    }

    ModelMeshImportResult ModelResourceImporter::UploadMeshes(
        ResourceRegistry& resources,
        const Model::ImportedModel& model,
        const ModelMeshImportOptions& options)
    {
        return UploadMeshes(resources.Meshes(), model, options);
    }
}
