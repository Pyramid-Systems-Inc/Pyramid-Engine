#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Graphics/Resources/ResourceHandle.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace Pyramid
{
    class ResourceRegistry;

    constexpr u32 kResourceManifestVersion = 1;

    enum class ResourceManifestDiagnosticCode : u8
    {
        InvalidHeader,
        UnsupportedVersion,
        MalformedEntry,
        InvalidResourceType,
        InvalidKey,
        DuplicateKey,
        InvalidAssetId,
        InvalidGeneration,
        MissingAsset,
        StaleGeneration
    };

    struct ResourceManifestDiagnostic
    {
        ResourceManifestDiagnosticCode code = ResourceManifestDiagnosticCode::MalformedEntry;
        u32 line = 0;
        std::string key;
        std::string message;
    };

    struct ResourceManifestEntry
    {
        std::string key;
        ResourceType type = ResourceType::Mesh;
        u64 assetIdHigh = 0;
        u64 assetIdLow = 0;
        u32 generation = 0;
        u32 sourceLine = 0;

        bool IsValid() const;
    };

    struct ResourceManifestRestoreReport
    {
        u32 resolved = 0;
        u32 missingAssets = 0;
        u32 staleGenerations = 0;
        std::vector<ResourceManifestDiagnostic> diagnostics;

        bool IsComplete() const
        {
            return missingAssets == 0 && staleGenerations == 0 && diagnostics.empty();
        }
    };

    /**
     * Versioned, deterministic list of typed resource handles.
     *
     * The text format is intentionally compact and dependency-free:
     *
     *   PYRAMID_RESOURCE_MANIFEST\t1
     *   mesh\tplayer.mesh\t0123...cdef\t4
     *
     * Keys are stable caller-owned reference names. Asset identifiers are stored
     * as their exact 128-bit hexadecimal value rather than re-hashing a path.
     */
    class ResourceManifest final
    {
    public:
        u32 GetVersion() const { return kResourceManifestVersion; }
        u32 GetEntryCount() const { return static_cast<u32>(m_entries.size()); }
        bool Empty() const { return m_entries.empty(); }
        const std::vector<ResourceManifestEntry>& GetEntries() const { return m_entries; }

        bool Add(std::string key, MeshHandle handle);
        bool Add(std::string key, ShaderHandle handle);
        bool Add(std::string key, TextureHandle handle);
        bool Add(std::string key, MaterialHandle handle);

        bool Remove(std::string_view key);
        bool Contains(std::string_view key) const;
        void Clear() { m_entries.clear(); }

        MeshHandle GetMeshHandle(std::string_view key) const;
        ShaderHandle GetShaderHandle(std::string_view key) const;
        TextureHandle GetTextureHandle(std::string_view key) const;
        MaterialHandle GetMaterialHandle(std::string_view key) const;

        std::string Serialize() const;

        /** Transactional parse: output remains unchanged when parsing fails. */
        static bool Deserialize(
            std::string_view text,
            ResourceManifest& output,
            std::vector<ResourceManifestDiagnostic>& diagnostics);

        /** Validate every serialized generation against the current registry. */
        ResourceManifestRestoreReport Restore(const ResourceRegistry& registry) const;

    private:
        bool AddEntry(std::string key, ResourceType type, u64 high, u64 low, u32 generation);
        const ResourceManifestEntry* FindEntry(std::string_view key, ResourceType type) const;

        std::vector<ResourceManifestEntry> m_entries;
    };
}
