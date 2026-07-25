#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>

#include <Pyramid/Graphics/GraphicsDevice.hpp>

namespace Pyramid
{
    ResourceRegistry::ResourceRegistry(IGraphicsDevice& device)
        : m_device(&device)
        , m_meshes(device)
        , m_shaders(device)
        , m_textures(device)
        , m_materials()
    {
    }

    ResourceRegistry::~ResourceRegistry()
    {
        Clear();
    }

    MeshHandle ResourceRegistry::AcquireMesh(const MeshSpecification& specification)
    {
        const auto mesh = m_meshes.GetOrCreate(specification);
        if (!mesh)
        {
            return {};
        }
        const MeshAssetId assetId = specification.assetId.IsValid()
            ? specification.assetId
            : mesh->GetContentId();
        return GetHandle(assetId);
    }

    ShaderHandle ResourceRegistry::AcquireShader(
        const ShaderProgramSpecification& specification)
    {
        const auto shader = m_shaders.GetOrCreate(specification);
        if (!shader)
        {
            return {};
        }
        const ShaderAssetId assetId = specification.assetId.IsValid()
            ? specification.assetId
            : shader->GetContentId();
        return GetHandle(assetId);
    }

    TextureHandle ResourceRegistry::AcquireTexture(
        const TextureResourceSpecification& specification)
    {
        const auto texture = m_textures.GetOrCreate(specification);
        if (!texture)
        {
            return {};
        }
        const TextureAssetId assetId = specification.assetId.IsValid()
            ? specification.assetId
            : texture->GetContentId();
        return GetHandle(assetId);
    }

    TextureHandle ResourceRegistry::AcquireTexture(
        const TextureFileSpecification& specification)
    {
        const auto texture = m_textures.GetOrCreate(specification);
        if (!texture)
        {
            return {};
        }
        const TextureAssetId assetId = specification.assetId.IsValid()
            ? specification.assetId
            : texture->GetContentId();
        return GetHandle(assetId);
    }

    MaterialHandle ResourceRegistry::AcquireMaterial(
        const MaterialSpecification& specification)
    {
        const auto material = m_materials.GetOrCreate(specification);
        if (!material)
        {
            return {};
        }
        const MaterialAssetId assetId = specification.assetId.IsValid()
            ? specification.assetId
            : material->GetContentId();
        return GetHandle(assetId);
    }

    MeshHandle ResourceRegistry::GetHandle(MeshAssetId assetId) const
    {
        const u32 generation = m_meshes.GetGeneration(assetId);
        return m_meshes.Find(assetId) && generation != 0
            ? MeshHandle::FromParts(assetId, generation)
            : MeshHandle{};
    }

    ShaderHandle ResourceRegistry::GetHandle(ShaderAssetId assetId) const
    {
        const u32 generation = m_shaders.GetGeneration(assetId);
        return m_shaders.Find(assetId) && generation != 0
            ? ShaderHandle::FromParts(assetId, generation)
            : ShaderHandle{};
    }

    TextureHandle ResourceRegistry::GetHandle(TextureAssetId assetId) const
    {
        const u32 generation = m_textures.GetGeneration(assetId);
        return m_textures.Find(assetId) && generation != 0
            ? TextureHandle::FromParts(assetId, generation)
            : TextureHandle{};
    }

    MaterialHandle ResourceRegistry::GetHandle(MaterialAssetId assetId) const
    {
        const u32 generation = m_materials.GetGeneration(assetId);
        return m_materials.Find(assetId) && generation != 0
            ? MaterialHandle::FromParts(assetId, generation)
            : MaterialHandle{};
    }

    std::shared_ptr<Mesh> ResourceRegistry::Resolve(MeshHandle handle) const
    {
        return IsAlive(handle) ? m_meshes.Find(handle.GetAssetId()) : nullptr;
    }

    std::shared_ptr<ShaderProgram> ResourceRegistry::Resolve(ShaderHandle handle) const
    {
        return IsAlive(handle) ? m_shaders.Find(handle.GetAssetId()) : nullptr;
    }

    std::shared_ptr<TextureResource> ResourceRegistry::Resolve(TextureHandle handle) const
    {
        return IsAlive(handle) ? m_textures.Find(handle.GetAssetId()) : nullptr;
    }

    std::shared_ptr<Material> ResourceRegistry::Resolve(MaterialHandle handle) const
    {
        return IsAlive(handle) ? m_materials.Find(handle.GetAssetId()) : nullptr;
    }

    bool ResourceRegistry::IsAlive(MeshHandle handle) const
    {
        return handle.IsValid() &&
            m_meshes.GetGeneration(handle.GetAssetId()) == handle.GetGeneration() &&
            m_meshes.Contains(handle.GetAssetId());
    }

    bool ResourceRegistry::IsAlive(ShaderHandle handle) const
    {
        return handle.IsValid() &&
            m_shaders.GetGeneration(handle.GetAssetId()) == handle.GetGeneration() &&
            m_shaders.Contains(handle.GetAssetId());
    }

    bool ResourceRegistry::IsAlive(TextureHandle handle) const
    {
        return handle.IsValid() &&
            m_textures.GetGeneration(handle.GetAssetId()) == handle.GetGeneration() &&
            m_textures.Contains(handle.GetAssetId());
    }

    bool ResourceRegistry::IsAlive(MaterialHandle handle) const
    {
        return handle.IsValid() &&
            m_materials.GetGeneration(handle.GetAssetId()) == handle.GetGeneration() &&
            m_materials.Contains(handle.GetAssetId());
    }

    bool ResourceRegistry::Evict(MeshHandle handle)
    {
        return IsAlive(handle) && m_meshes.Evict(handle.GetAssetId());
    }

    bool ResourceRegistry::Evict(ShaderHandle handle)
    {
        return IsAlive(handle) && m_shaders.Evict(handle.GetAssetId());
    }

    bool ResourceRegistry::Evict(TextureHandle handle)
    {
        return IsAlive(handle) && m_textures.Evict(handle.GetAssetId());
    }

    bool ResourceRegistry::Evict(MaterialHandle handle)
    {
        return IsAlive(handle) && m_materials.Evict(handle.GetAssetId());
    }

    ShaderHandle ResourceRegistry::RecompileShader(
        ShaderHandle handle,
        const ShaderProgramSpecification& replacement)
    {
        if (!IsAlive(handle) ||
            !m_shaders.Recompile(handle.GetAssetId(), replacement))
        {
            return {};
        }
        return GetHandle(handle.GetAssetId());
    }

    TextureHandle ResourceRegistry::ReloadTexture(TextureHandle handle)
    {
        if (!IsAlive(handle) || !m_textures.Reload(handle.GetAssetId()))
        {
            return {};
        }
        return GetHandle(handle.GetAssetId());
    }

    TextureHandle ResourceRegistry::ReloadTexture(
        TextureHandle handle,
        const TextureFileSpecification& replacement)
    {
        if (!IsAlive(handle) ||
            !m_textures.Reload(handle.GetAssetId(), replacement))
        {
            return {};
        }
        return GetHandle(handle.GetAssetId());
    }

    MaterialHandle ResourceRegistry::ReplaceMaterial(
        MaterialHandle handle,
        const MaterialSpecification& replacement)
    {
        if (!IsAlive(handle) ||
            !m_materials.Replace(handle.GetAssetId(), replacement))
        {
            return {};
        }
        return GetHandle(handle.GetAssetId());
    }

    ResourceRegistryReleaseStats ResourceRegistry::CollectUnused()
    {
        ResourceRegistryReleaseStats released;
        released.materials = m_materials.CollectUnused();
        released.textures = m_textures.CollectUnused();
        released.shaders = m_shaders.CollectUnused();
        released.meshes = m_meshes.CollectUnused();
        return released;
    }

    ResourceRegistryReleaseStats ResourceRegistry::Clear()
    {
        ResourceRegistryReleaseStats released;
        released.materials = m_materials.Clear();
        released.textures = m_textures.Clear();
        released.shaders = m_shaders.Clear();
        released.meshes = m_meshes.Clear();
        return released;
    }

    ResourceRegistryStats ResourceRegistry::GetStats() const
    {
        ResourceRegistryStats statistics;
        statistics.meshes = m_meshes.GetStats();
        statistics.shaders = m_shaders.GetStats();
        statistics.textures = m_textures.GetStats();
        statistics.materials = m_materials.GetStats();
        return statistics;
    }
}
