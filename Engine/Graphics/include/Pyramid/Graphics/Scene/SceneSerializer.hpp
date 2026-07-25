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

    constexpr u32 kSceneSerializationVersion = 1;

    enum class SceneSerializationDiagnosticCode : u8
    {
        InvalidHeader,
        UnsupportedVersion,
        MissingSceneRecord,
        DuplicateSceneRecord,
        MalformedRecord,
        InvalidObjectKey,
        DuplicateObjectKey,
        InvalidNameEncoding,
        InvalidNumber,
        NonFiniteValue,
        InvalidBoolean,
        InvalidBoundsMode,
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
        u32 serializedObjects = 0;
        std::vector<SceneSerializationDiagnostic> diagnostics;

        bool Succeeded() const
        {
            return diagnostics.empty() && !text.empty();
        }
    };

    struct SceneDeserializationResult
    {
        std::shared_ptr<Scene> scene;
        u32 restoredObjects = 0;
        u32 missingAssets = 0;
        u32 staleGenerations = 0;
        std::vector<SceneSerializationDiagnostic> diagnostics;

        bool Succeeded() const
        {
            return scene != nullptr && diagnostics.empty();
        }
    };

    /**
     * Versioned, deterministic serialization for a Scene's flat render-object list.
     *
     * Meshes and materials are referenced by ResourceManifest keys rather than by
     * raw asset IDs or owning pointers. Deserialization restores generation-checked
     * handles through ResourceRegistry and fails transactionally when the document,
     * manifest, or registry state is invalid.
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
