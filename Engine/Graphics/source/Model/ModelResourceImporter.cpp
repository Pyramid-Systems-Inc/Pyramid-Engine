#include <Pyramid/Graphics/Model/ModelResourceImporter.hpp>

#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Graphics/Geometry/Mesh.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <limits>
#include <set>
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

        template <typename AssetId>
        struct ResourceRollbackRecord
        {
            AssetId stableId;
            AssetId contentId;
            bool stableExisted = false;
            bool contentExisted = false;
        };

        struct PreparedMaterialResource
        {
            i32 materialIndex = -1;
            MaterialHandle material;
            TextureHandle diffuseTexture;
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

        bool IsFinite(const Model::ImportedMaterial& material)
        {
            return std::isfinite(material.ambientColor.x) &&
                std::isfinite(material.ambientColor.y) &&
                std::isfinite(material.ambientColor.z) &&
                std::isfinite(material.diffuseColor.x) &&
                std::isfinite(material.diffuseColor.y) &&
                std::isfinite(material.diffuseColor.z) &&
                std::isfinite(material.specularColor.x) &&
                std::isfinite(material.specularColor.y) &&
                std::isfinite(material.specularColor.z) &&
                std::isfinite(material.specularExponent) &&
                std::isfinite(material.opacity);
        }

        template <typename Result>
        void AddDiagnostic(
            Result& result,
            const Model::ImportedModel& model,
            Model::ImportDiagnosticSeverity severity,
            std::string message,
            std::string source = {})
        {
            result.diagnostics.push_back({
                severity,
                0,
                source.empty() ? model.sourcePath : std::move(source),
                std::move(message)});
        }

        template <typename Result>
        void AddError(
            Result& result,
            const Model::ImportedModel& model,
            std::string message,
            std::string source = {})
        {
            AddDiagnostic(
                result,
                model,
                Model::ImportDiagnosticSeverity::Error,
                std::move(message),
                std::move(source));
        }

        template <typename Result>
        void AddWarning(
            Result& result,
            const Model::ImportedModel& model,
            std::string message,
            std::string source = {})
        {
            AddDiagnostic(
                result,
                model,
                Model::ImportDiagnosticSeverity::Warning,
                std::move(message),
                std::move(source));
        }

        bool ValidateMaterialProfile(
            const Model::ImportedModel& model,
            const ModelMaterialImportProfile& profile,
            ModelResourceImportResult& result)
        {
            const auto& texture = profile.diffuseTexture;
            if (!std::isfinite(texture.maxAnisotropy) || texture.maxAnisotropy < 1.0f)
            {
                AddError(result, model,
                    "diffuse texture anisotropy must be finite and at least 1.0");
                return false;
            }
            for (f32 component : texture.borderColor)
            {
                if (!std::isfinite(component))
                {
                    AddError(result, model,
                        "diffuse texture border color must contain only finite values");
                    return false;
                }
            }

            std::set<std::string> names;
            const auto addName = [&](const std::string& name)
            {
                return name.empty() || names.insert(name).second;
            };
            const auto& uniforms = profile.uniforms;
            if (!addName(uniforms.diffuseTexture) ||
                !addName(uniforms.hasDiffuseTexture) ||
                !addName(uniforms.ambientColor) ||
                !addName(uniforms.diffuseColor) ||
                !addName(uniforms.specularColor) ||
                !addName(uniforms.specularExponent) ||
                !addName(uniforms.opacity) ||
                !addName(uniforms.illuminationModel))
            {
                AddError(result, model,
                    "material import profile uniform and sampler names must be unique");
                return false;
            }
            return true;
        }

        std::string ResolveAssetPrefix(
            const Model::ImportedModel& model,
            const std::string& requestedPrefix)
        {
            if (!requestedPrefix.empty())
            {
                return requestedPrefix;
            }
            if (!model.sourcePath.empty())
            {
                return model.sourcePath;
            }
            return "memory-model";
        }

        std::string MakeMeshAssetName(
            const Model::ImportedModel& model,
            const ModelMeshImportOptions& options,
            const Model::ImportedPrimitive& primitive,
            u32 primitiveIndex)
        {
            return ResolveAssetPrefix(model, options.assetPrefix) +
                "/primitive/" + std::to_string(primitiveIndex) + "/" +
                (primitive.name.empty() ? "unnamed" : primitive.name);
        }

        std::string MakeMaterialAssetName(
            const std::string& prefix,
            i32 materialIndex,
            const std::string& name)
        {
            if (materialIndex < 0)
            {
                return prefix + "/material/default";
            }
            return prefix + "/material/" + std::to_string(materialIndex) + "/" +
                (name.empty() ? "unnamed" : name);
        }

        std::string MakeTextureAssetName(
            const std::string& prefix,
            i32 materialIndex)
        {
            return prefix + "/material/" + std::to_string(materialIndex) +
                "/diffuse-texture";
        }

        std::string ResolveTexturePath(
            const std::string& importedPath,
            const std::string& sourceDirectory)
        {
            std::filesystem::path path(importedPath);
            if (path.is_relative() && !sourceDirectory.empty())
            {
                path = std::filesystem::path(sourceDirectory) / path;
            }
            return path.lexically_normal().generic_string();
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
                MakeMeshAssetName(model, options, primitive, primitiveIndex));
            prepared.contentId = Mesh::CalculateContentId(specification);
            if (!prepared.contentId.IsValid())
            {
                AddError(result, model, "primitive " + std::to_string(primitiveIndex) +
                    " could not produce a valid mesh specification");
                return false;
            }
            return true;
        }

        void RollBackMeshes(
            MeshCache& cache,
            const std::vector<ResourceRollbackRecord<MeshAssetId>>& records)
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

        void RollBackMaterials(
            MaterialCache& cache,
            const std::vector<ResourceRollbackRecord<MaterialAssetId>>& records)
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

        void RollBackTextures(
            TextureCache& cache,
            const std::vector<ResourceRollbackRecord<TextureAssetId>>& records)
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

        void AddUniform(
            std::vector<MaterialUniform>& uniforms,
            const std::string& name,
            MaterialUniformValue value)
        {
            if (!name.empty())
            {
                uniforms.push_back({name, std::move(value)});
            }
        }

        TextureFileSpecification MakeTextureSpecification(
            const ModelTextureImportSettings& settings,
            const std::string& path,
            TextureAssetId assetId,
            std::string name)
        {
            TextureFileSpecification specification;
            specification.filepath = path;
            specification.colorSpace = settings.colorSpace;
            specification.generateMips = settings.generateMips;
            specification.minFilter = settings.minFilter;
            specification.magFilter = settings.magFilter;
            specification.wrapS = settings.wrapS;
            specification.wrapT = settings.wrapT;
            for (u32 index = 0; index < 4; ++index)
            {
                specification.borderColor[index] = settings.borderColor[index];
            }
            specification.maxAnisotropy = settings.maxAnisotropy;
            specification.flipY = settings.flipY;
            specification.assetId = assetId;
            specification.name = std::move(name);
            return specification;
        }

        bool PublishMaterial(
            ResourceRegistry& resources,
            const Model::ImportedModel& model,
            const Model::ImportedMaterial& imported,
            i32 materialIndex,
            const std::string& prefix,
            const ModelResourceImportOptions& options,
            const std::shared_ptr<ShaderProgram>& shader,
            PreparedMaterialResource& published,
            std::vector<ResourceRollbackRecord<TextureAssetId>>& textureRollback,
            std::vector<ResourceRollbackRecord<MaterialAssetId>>& materialRollback,
            ModelResourceImportResult& result)
        {
            if (!IsFinite(imported))
            {
                AddError(result, model, "material " + std::to_string(materialIndex) +
                    " contains non-finite properties");
                return false;
            }
            if (imported.opacity < 0.0f || imported.opacity > 1.0f)
            {
                AddError(result, model, "material " + std::to_string(materialIndex) +
                    " opacity must be in the range 0-1");
                return false;
            }
            if (imported.specularExponent < 0.0f)
            {
                AddError(result, model, "material " + std::to_string(materialIndex) +
                    " specular exponent cannot be negative");
                return false;
            }

            std::shared_ptr<TextureResource> texture;
            TextureHandle textureHandle;
            if (!imported.diffuseTexture.empty())
            {
                if (options.materialProfile.uniforms.diffuseTexture.empty())
                {
                    AddError(result, model,
                        "diffuse texture uniform name is empty for a textured material");
                    return false;
                }

                const std::string path = ResolveTexturePath(
                    imported.diffuseTexture,
                    options.sourceDirectory);
                const TextureAssetId stableId = TextureAssetId::FromString(
                    MakeTextureAssetName(prefix, materialIndex));
                const bool stableExisted = resources.Textures().Contains(stableId);

                const TextureFileSpecification specification = MakeTextureSpecification(
                    options.materialProfile.diffuseTexture,
                    path,
                    stableId,
                    imported.name + " Diffuse");
                const TextureAssetId expectedContentId =
                    TextureResource::CalculateContentId(specification);
                const auto existingTexture = resources.Textures().Find(stableId);
                if (existingTexture && expectedContentId.IsValid() &&
                    existingTexture->GetContentId() != expectedContentId)
                {
                    AddError(result, model,
                        "stable diffuse-texture identifier conflicts with resident content for material " +
                            std::to_string(materialIndex),
                        path);
                    return false;
                }
                const bool contentExisted = expectedContentId.IsValid() &&
                    resources.Textures().Contains(expectedContentId);
                texture = resources.Textures().GetOrCreate(specification);
                if (!texture)
                {
                    if (options.materialProfile.missingTextureBehavior ==
                        ModelMissingTextureBehavior::Error)
                    {
                        AddError(result, model,
                            "failed to load diffuse texture for material " +
                                std::to_string(materialIndex),
                            path);
                        return false;
                    }
                    AddWarning(result, model,
                        "diffuse texture was ignored because it could not be loaded",
                        path);
                }
                else
                {
                    const TextureAssetId contentId = texture->GetContentId();
                    textureRollback.push_back({
                        stableId,
                        contentId,
                        stableExisted,
                        contentExisted});
                    textureHandle = resources.GetHandle(stableId);
                    if (!textureHandle)
                    {
                        AddError(result, model,
                            "diffuse texture was created but no stable handle was available",
                            path);
                        return false;
                    }
                }
            }

            MaterialSpecification specification;
            specification.shader = shader;
            specification.renderState = options.materialProfile.renderState;
            if (options.materialProfile.alphaBlendFromOpacity && imported.opacity < 1.0f)
            {
                specification.renderState.blendMode = MaterialBlendMode::Alpha;
            }
            if (texture)
            {
                specification.textures.push_back({
                    options.materialProfile.uniforms.diffuseTexture,
                    0,
                    texture});
            }

            AddUniform(
                specification.uniforms,
                options.materialProfile.uniforms.hasDiffuseTexture,
                i32{texture ? 1 : 0});
            AddUniform(
                specification.uniforms,
                options.materialProfile.uniforms.ambientColor,
                imported.ambientColor);
            AddUniform(
                specification.uniforms,
                options.materialProfile.uniforms.diffuseColor,
                Math::Vec4(
                    imported.diffuseColor.x,
                    imported.diffuseColor.y,
                    imported.diffuseColor.z,
                    imported.opacity));
            AddUniform(
                specification.uniforms,
                options.materialProfile.uniforms.specularColor,
                imported.specularColor);
            AddUniform(
                specification.uniforms,
                options.materialProfile.uniforms.specularExponent,
                imported.specularExponent);
            AddUniform(
                specification.uniforms,
                options.materialProfile.uniforms.opacity,
                imported.opacity);
            AddUniform(
                specification.uniforms,
                options.materialProfile.uniforms.illuminationModel,
                imported.illuminationModel);

            const MaterialAssetId stableId = MaterialAssetId::FromString(
                MakeMaterialAssetName(prefix, materialIndex, imported.name));
            specification.assetId = stableId;
            specification.name = imported.name.empty()
                ? "Imported Material"
                : imported.name;

            const bool stableExisted = resources.Materials().Contains(stableId);
            const MaterialAssetId expectedContentId =
                Material::CalculateContentId(specification);
            const bool contentExisted = expectedContentId.IsValid() &&
                resources.Materials().Contains(expectedContentId);

            const auto material = resources.Materials().GetOrCreate(specification);
            if (!material)
            {
                AddError(result, model,
                    "failed to publish material " + std::to_string(materialIndex));
                return false;
            }

            materialRollback.push_back({
                stableId,
                material->GetContentId(),
                stableExisted,
                contentExisted});
            const MaterialHandle materialHandle = resources.GetHandle(stableId);
            if (!materialHandle)
            {
                AddError(result, model,
                    "material was created but no stable handle was available");
                return false;
            }

            published.materialIndex = materialIndex;
            published.material = materialHandle;
            published.diffuseTexture = textureHandle;
            return true;
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

    u32 ModelResourceImportResult::GetErrorCount() const
    {
        u32 count = 0;
        for (const auto& diagnostic : diagnostics)
        {
            count += diagnostic.IsError() ? 1u : 0u;
        }
        return count;
    }

    bool ModelResourceImportResult::IsSuccess() const
    {
        return GetErrorCount() == 0 && !renderables.empty();
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
                AddError(result, model,
                    "stable mesh identifier conflicts with resident geometry for primitive " +
                        std::to_string(primitive.primitiveIndex));
                return result;
            }
        }

        std::vector<ResourceRollbackRecord<MeshAssetId>> rollback;
        rollback.reserve(prepared.size());
        result.meshes.reserve(prepared.size());

        for (auto& primitive : prepared)
        {
            ResourceRollbackRecord<MeshAssetId> record;
            record.stableId = primitive.specification.assetId;
            record.contentId = primitive.contentId;
            record.stableExisted = cache.Contains(record.stableId);
            record.contentExisted = cache.Contains(record.contentId);

            auto mesh = cache.GetOrCreate(primitive.specification);
            if (!mesh)
            {
                result.meshes.clear();
                RollBackMeshes(cache, rollback);
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

    ModelResourceImportResult ModelResourceImporter::ImportModel(
        ResourceRegistry& resources,
        const Model::ImportedModel& model,
        const ModelResourceImportOptions& options)
    {
        ModelResourceImportResult result;
        result.diagnostics = model.diagnostics;
        if (!model.IsValid())
        {
            AddError(result, model, "model is not valid and cannot be imported");
            return result;
        }
        if (!ValidateMaterialProfile(model, options.materialProfile, result))
        {
            return result;
        }

        const auto shader = resources.Resolve(options.materialProfile.shader);
        if (!shader)
        {
            AddError(result, model,
                "material import profile requires a live render shader handle");
            return result;
        }

        std::shared_ptr<Material> fallbackMaterial;
        if (options.materialProfile.fallbackMaterial)
        {
            fallbackMaterial = resources.Resolve(options.materialProfile.fallbackMaterial);
            if (!fallbackMaterial)
            {
                AddError(result, model,
                    "material import profile contains a stale fallback material handle");
                return result;
            }
        }

        if (model.materials.size() >
            static_cast<std::size_t>(std::numeric_limits<i32>::max()))
        {
            AddError(result, model, "model material count exceeds graphics import limits");
            return result;
        }

        bool needsUnassignedMaterial = false;
        for (u32 index = 0; index < model.primitives.size(); ++index)
        {
            const i32 materialIndex = model.primitives[index].materialIndex;
            if (materialIndex < -1 ||
                materialIndex >= static_cast<i32>(model.materials.size()))
            {
                AddError(result, model,
                    "primitive " + std::to_string(index) +
                        " references an invalid material slot");
                return result;
            }
            needsUnassignedMaterial = needsUnassignedMaterial || materialIndex < 0;
        }

        if (needsUnassignedMaterial &&
            !options.materialProfile.useFallbackForUnassignedPrimitives)
        {
            AddError(result, model,
                "model contains unassigned primitives and the profile disables fallback handling");
            return result;
        }

        const std::string prefix = ResolveAssetPrefix(model, options.assetPrefix);
        std::vector<ResourceRollbackRecord<TextureAssetId>> textureRollback;
        std::vector<ResourceRollbackRecord<MaterialAssetId>> materialRollback;
        std::vector<PreparedMaterialResource> publishedMaterials;
        publishedMaterials.reserve(model.materials.size() + 1u);

        for (u32 index = 0; index < model.materials.size(); ++index)
        {
            PreparedMaterialResource published;
            if (!PublishMaterial(
                    resources,
                    model,
                    model.materials[index],
                    static_cast<i32>(index),
                    prefix,
                    options,
                    shader,
                    published,
                    textureRollback,
                    materialRollback,
                    result))
            {
                RollBackMaterials(resources.Materials(), materialRollback);
                RollBackTextures(resources.Textures(), textureRollback);
                return result;
            }
            publishedMaterials.push_back(published);
        }

        PreparedMaterialResource unassigned;
        if (needsUnassignedMaterial)
        {
            if (fallbackMaterial)
            {
                unassigned.materialIndex = -1;
                unassigned.material = options.materialProfile.fallbackMaterial;
            }
            else
            {
                Model::ImportedMaterial defaultMaterial;
                defaultMaterial.name = "Default";
                if (!PublishMaterial(
                        resources,
                        model,
                        defaultMaterial,
                        -1,
                        prefix,
                        options,
                        shader,
                        unassigned,
                        textureRollback,
                        materialRollback,
                        result))
                {
                    RollBackMaterials(resources.Materials(), materialRollback);
                    RollBackTextures(resources.Textures(), textureRollback);
                    return result;
                }
            }
            publishedMaterials.push_back(unassigned);
        }

        ModelMeshImportOptions meshOptions;
        meshOptions.assetPrefix = prefix;
        const auto meshResult = UploadMeshes(resources.Meshes(), model, meshOptions);
        if (!meshResult.IsSuccess())
        {
            if (meshResult.diagnostics.size() > model.diagnostics.size())
            {
                result.diagnostics.insert(
                    result.diagnostics.end(),
                    meshResult.diagnostics.begin() +
                        static_cast<std::ptrdiff_t>(model.diagnostics.size()),
                    meshResult.diagnostics.end());
            }
            RollBackMaterials(resources.Materials(), materialRollback);
            RollBackTextures(resources.Textures(), textureRollback);
            return result;
        }

        result.materials.reserve(publishedMaterials.size());
        for (const auto& material : publishedMaterials)
        {
            result.materials.push_back({
                material.materialIndex,
                material.material,
                material.diffuseTexture});
        }

        result.renderables.reserve(meshResult.meshes.size());
        for (const auto& mesh : meshResult.meshes)
        {
            const PreparedMaterialResource* material = nullptr;
            if (mesh.materialIndex >= 0)
            {
                material = &publishedMaterials[static_cast<std::size_t>(mesh.materialIndex)];
            }
            else
            {
                material = &unassigned;
            }
            result.renderables.push_back({
                mesh.primitiveIndex,
                mesh.materialIndex,
                mesh.mesh,
                material->material,
                material->diffuseTexture});
        }

        return result;
    }
}
