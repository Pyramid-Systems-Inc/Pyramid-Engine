#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Resources/ResourceHandle.hpp>
#include <Pyramid/Graphics/Texture/TextureResource.hpp>
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

    enum class ModelMissingTextureBehavior : u8
    {
        Error = 0,
        Ignore
    };

    struct ModelTextureImportSettings
    {
        TextureColorSpace colorSpace = TextureColorSpace::SRGB;
        bool generateMips = true;
        TextureFilter minFilter = TextureFilter::LinearMipmapLinear;
        TextureFilter magFilter = TextureFilter::Linear;
        TextureWrap wrapS = TextureWrap::Repeat;
        TextureWrap wrapT = TextureWrap::Repeat;
        f32 borderColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        f32 maxAnisotropy = 1.0f;
        bool flipY = false;
    };

    /** Shader-facing names used while converting MTL properties to uniforms. */
    struct ModelMaterialUniformNames
    {
        std::string diffuseTexture = "u_AlbedoMap";
        std::string hasDiffuseTexture = "u_HasAlbedoMap";
        std::string ambientColor = "u_AmbientColor";
        std::string diffuseColor = "u_BaseColor";
        std::string specularColor = "u_SpecularColor";
        std::string specularExponent = "u_SpecularExponent";
        std::string opacity = "u_Opacity";
        std::string illuminationModel = "u_IlluminationModel";
    };

    /** Configurable graphics profile for renderer-independent imported materials. */
    struct ModelMaterialImportProfile
    {
        ShaderHandle shader;
        MaterialHandle fallbackMaterial;
        ModelTextureImportSettings diffuseTexture;
        ModelMaterialUniformNames uniforms;
        MaterialRenderState renderState;
        ModelMissingTextureBehavior missingTextureBehavior =
            ModelMissingTextureBehavior::Error;
        bool useFallbackForUnassignedPrimitives = true;
        bool alphaBlendFromOpacity = true;
    };

    struct ModelResourceImportOptions
    {
        /** Stable namespace used for generated mesh, texture, and material IDs. */
        std::string assetPrefix;

        /** Optional base directory for relative texture paths. */
        std::string sourceDirectory;

        ModelMaterialImportProfile materialProfile;
    };

    struct ImportedMaterialResource
    {
        i32 materialIndex = -1;
        MaterialHandle material;
        TextureHandle diffuseTexture;
    };

    struct ImportedRenderableResource
    {
        u32 primitiveIndex = 0;
        i32 materialIndex = -1;
        MeshHandle mesh;
        MaterialHandle material;
        TextureHandle diffuseTexture;
    };

    struct ModelResourceImportResult
    {
        std::vector<ImportedMaterialResource> materials;
        std::vector<ImportedRenderableResource> renderables;
        std::vector<Model::ImportDiagnostic> diagnostics;

        u32 GetErrorCount() const;
        bool IsSuccess() const;
    };

    /**
     * @brief Publish renderer-independent model data through graphics caches.
     *
     * Mesh-only uploads remain available for tools that assign materials later.
     * ImportModel additionally converts MTL diffuse textures and properties into
     * immutable texture/material resources through a caller-supplied shader
     * profile. The complete operation is transactional: failure removes only
     * aliases and resources created by that operation.
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

        static ModelResourceImportResult ImportModel(
            ResourceRegistry& resources,
            const Model::ImportedModel& model,
            const ModelResourceImportOptions& options);
    };
}
