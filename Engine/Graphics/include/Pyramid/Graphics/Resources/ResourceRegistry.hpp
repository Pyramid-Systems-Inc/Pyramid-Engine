#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Graphics/Geometry/MeshCache.hpp>
#include <Pyramid/Graphics/Material/MaterialCache.hpp>
#include <Pyramid/Graphics/Shader/ShaderCache.hpp>
#include <Pyramid/Graphics/Texture/TextureCache.hpp>
#include <Pyramid/Graphics/Resources/ResourceHandle.hpp>

namespace Pyramid
{
    class IGraphicsDevice;

    /** Counts resources released by one registry collection or clear pass. */
    struct ResourceRegistryReleaseStats
    {
        u32 materials = 0;
        u32 textures = 0;
        u32 shaders = 0;
        u32 meshes = 0;

        u32 GetTotal() const
        {
            return materials + textures + shaders + meshes;
        }

        bool HasChanges() const { return GetTotal() != 0; }
    };

    /** Combined residency and activity snapshot for every graphics cache. */
    struct ResourceRegistryStats
    {
        MeshCacheStats meshes;
        ShaderCacheStats shaders;
        TextureCacheStats textures;
        MaterialCacheStats materials;

        u64 GetEstimatedResidentBytes() const
        {
            return meshes.GetResidentGeometryBytes() + shaders.residentSourceBytes +
                textures.estimatedResidentTextureBytes +
                materials.estimatedResidentMetadataBytes;
        }
    };

    /**
     * Central owner for immutable graphics resource caches.
     *
     * The registry enforces dependency-safe release order: materials first,
     * followed by textures, shaders, and meshes. The registry itself and every
     * external resource owner must be destroyed before the graphics device and
     * native graphics context.
     *
     * ResourceRegistry is intended for the graphics thread and is not internally
     * synchronized.
     */
    class ResourceRegistry final
    {
    public:
        explicit ResourceRegistry(IGraphicsDevice& device);

        ResourceRegistry(const ResourceRegistry&) = delete;
        ResourceRegistry& operator=(const ResourceRegistry&) = delete;
        ResourceRegistry(ResourceRegistry&&) = delete;
        ResourceRegistry& operator=(ResourceRegistry&&) = delete;
        ~ResourceRegistry();

        IGraphicsDevice& GetDevice() const { return *m_device; }

        MeshCache& Meshes() { return m_meshes; }
        const MeshCache& Meshes() const { return m_meshes; }

        ShaderCache& Shaders() { return m_shaders; }
        const ShaderCache& Shaders() const { return m_shaders; }

        TextureCache& Textures() { return m_textures; }
        const TextureCache& Textures() const { return m_textures; }

        MaterialCache& Materials() { return m_materials; }
        const MaterialCache& Materials() const { return m_materials; }

        // Handle-first acquisition. Returned handles never own GPU resources.
        MeshHandle AcquireMesh(const MeshSpecification& specification);
        ShaderHandle AcquireShader(const ShaderProgramSpecification& specification);
        TextureHandle AcquireTexture(const TextureResourceSpecification& specification);
        TextureHandle AcquireTexture(const TextureFileSpecification& specification);
        MaterialHandle AcquireMaterial(const MaterialSpecification& specification);

        // Convert a resident asset identifier into its current typed generation.
        MeshHandle GetHandle(MeshAssetId assetId) const;
        ShaderHandle GetHandle(ShaderAssetId assetId) const;
        TextureHandle GetHandle(TextureAssetId assetId) const;
        MaterialHandle GetHandle(MaterialAssetId assetId) const;

        // Generation-checked resolution. Stale or mismatched handles return nullptr.
        std::shared_ptr<Mesh> Resolve(MeshHandle handle) const;
        std::shared_ptr<ShaderProgram> Resolve(ShaderHandle handle) const;
        std::shared_ptr<TextureResource> Resolve(TextureHandle handle) const;
        std::shared_ptr<Material> Resolve(MaterialHandle handle) const;

        bool IsAlive(MeshHandle handle) const;
        bool IsAlive(ShaderHandle handle) const;
        bool IsAlive(TextureHandle handle) const;
        bool IsAlive(MaterialHandle handle) const;

        bool Evict(MeshHandle handle);
        bool Evict(ShaderHandle handle);
        bool Evict(TextureHandle handle);
        bool Evict(MaterialHandle handle);

        /** Replace a current stable alias and return its new generation. */
        ShaderHandle RecompileShader(
            ShaderHandle handle,
            const ShaderProgramSpecification& replacement);
        TextureHandle ReloadTexture(TextureHandle handle);
        TextureHandle ReloadTexture(
            TextureHandle handle,
            const TextureFileSpecification& replacement);
        MaterialHandle ReplaceMaterial(
            MaterialHandle handle,
            const MaterialSpecification& replacement);

        /**
         * Collect cache-only resources in dependency-safe order.
         * Externally referenced materials keep their shaders and textures alive.
         */
        ResourceRegistryReleaseStats CollectUnused();

        /**
         * Remove every cached alias/resource in dependency-safe order.
         * Existing external shared_ptr owners remain object-valid, but they must
         * still be released before the graphics device/context is destroyed.
         */
        ResourceRegistryReleaseStats Clear();

        ResourceRegistryStats GetStats() const;

    private:
        IGraphicsDevice* m_device = nullptr;

        // Members are declared dependency-last so normal reverse destruction also
        // releases materials before textures/shaders and all GPU resources before
        // the externally owned graphics device.
        MeshCache m_meshes;
        ShaderCache m_shaders;
        TextureCache m_textures;
        MaterialCache m_materials;
    };
}
