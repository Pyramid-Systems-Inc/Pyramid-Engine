#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Graphics/Resources/ResourceHandle.hpp>
#include <Pyramid/Model/Model.hpp>

#include <string>
#include <vector>

namespace Pyramid
{
    class MeshCache;
    class ResourceRegistry;

    struct ImportedMeshResource
    {
        u32 primitiveIndex = 0;
        i32 materialIndex = -1;
        MeshHandle mesh;
    };

    struct ModelMeshImportOptions
    {
        /** Stable namespace used for generated mesh asset identifiers. */
        std::string assetPrefix;
    };

    struct ModelMeshImportResult
    {
        std::vector<ImportedMeshResource> meshes;
        std::vector<Model::ImportDiagnostic> diagnostics;

        u32 GetErrorCount() const;
        bool IsSuccess() const;
    };

    /**
     * @brief Transactionally publish CPU-imported primitives as immutable meshes.
     *
     * The importer validates every primitive and stable identifier before creating
     * graphics resources. If any upload fails, aliases and newly created geometry
     * from the current operation are rolled back without disturbing resources that
     * were already resident in the registry.
     */
    class ModelResourceImporter final
    {
    public:
        static ModelMeshImportResult UploadMeshes(
            MeshCache& cache,
            const Model::ImportedModel& model,
            const ModelMeshImportOptions& options = {});

        static ModelMeshImportResult UploadMeshes(
            ResourceRegistry& resources,
            const Model::ImportedModel& model,
            const ModelMeshImportOptions& options = {});
    };
}
