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
