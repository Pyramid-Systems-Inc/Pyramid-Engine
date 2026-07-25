#pragma once

#include <Pyramid/Core/Prerequisites.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Pyramid
{
    class ResourceManifest;
    class ResourceRegistry;
    class Scene;

    constexpr u32 kSceneSerializationVersion = 2;

    enum class SceneSerializationDiagnosticCode : u8
    {
        InvalidHeader,
        UnsupportedVersion,
        MissingSceneRecord,
        DuplicateSceneRecord,
        MalformedRecord,
        InvalidEntityId,
        DuplicateEntityId,
        InvalidParent,
        HierarchyCycle,
        DuplicateComponent,
        InvalidNameEncoding,
        InvalidNumber,
        NonFiniteValue,
        InvalidBoolean,
        InvalidBoundsMode,
        InvalidLightType,
        InvalidRotation,
        MissingManifestResource,
        ResourceTypeMismatch,
        MissingAsset,
        StaleGeneration,
        UnresolvedResource
    };

    struct SceneSerializationDiagnostic
    {
        SceneSerializationDiagnosticCode code =
            SceneSerializationDiagnosticCode::MalformedRecord;
        u32 line = 0;
        std::string key;
        std::string message;
    };

    struct SceneSerializationResult
    {
        std::string text;
        u32 serializedEntities = 0;
        u32 serializedObjects = 0;
        u32 serializedLights = 0;
        std::vector<SceneSerializationDiagnostic> diagnostics;

        bool Succeeded() const
        {
            return diagnostics.empty() && !text.empty();
        }
    };

    struct SceneDeserializationResult
    {
        std::shared_ptr<Scene> scene;
        u32 restoredEntities = 0;
        u32 restoredObjects = 0;
        u32 restoredLights = 0;
        u32 missingAssets = 0;
        u32 staleGenerations = 0;
        std::vector<SceneSerializationDiagnostic> diagnostics;

        bool Succeeded() const
        {
            return scene != nullptr && diagnostics.empty();
        }
    };

    /**
     * Versioned deterministic persistence for the authoritative entity scene.
     *
     * Version 2 stores stable entity IDs, parent IDs, local transforms,
     * visibility, MeshRendererComponent, LightComponent, and the primary-light
     * entity. Mesh/material references use ResourceManifest keys and are restored
     * through generation-checked ResourceRegistry handles.
     */
    class SceneSerializer final
    {
    public:
        static SceneSerializationResult Serialize(
            const Scene& scene,
            const ResourceManifest& manifest,
            const ResourceRegistry& registry);

        static SceneDeserializationResult Deserialize(
            std::string_view text,
            const ResourceManifest& manifest,
            const ResourceRegistry& registry);
    };
}
