#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Graphics/Geometry/Mesh.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Shader/ShaderProgram.hpp>
#include <Pyramid/Graphics/Texture/TextureResource.hpp>

#include <cstddef>

namespace Pyramid
{
    enum class ResourceType : u8
    {
        Mesh,
        Shader,
        Texture,
        Material
    };

    /**
     * Serializable, non-owning reference to one registry resource generation.
     *
     * Handles remain small value types: an asset identifier plus the generation
     * that was current when the handle was issued. Resolving a stale handle
     * returns nullptr rather than silently binding a replacement resource that
     * reused the same stable asset identifier.
     */
    template <typename AssetIdType, typename ResourceTypeValue, ResourceType TypeValue>
    class ResourceHandle final
    {
    public:
        using AssetId = AssetIdType;
        using Resource = ResourceTypeValue;

        constexpr ResourceHandle() = default;

        static constexpr ResourceType GetResourceType() { return TypeValue; }

        static ResourceHandle FromParts(AssetId assetId, u32 generation)
        {
            return ResourceHandle(assetId, generation);
        }

        bool IsValid() const
        {
            return m_assetId.IsValid() && m_generation != 0;
        }

        explicit operator bool() const { return IsValid(); }

        AssetId GetAssetId() const { return m_assetId; }
        u32 GetGeneration() const { return m_generation; }

        void Reset()
        {
            m_assetId = {};
            m_generation = 0;
        }

        bool operator==(const ResourceHandle& other) const
        {
            return m_assetId == other.m_assetId && m_generation == other.m_generation;
        }

        bool operator!=(const ResourceHandle& other) const
        {
            return !(*this == other);
        }

    private:
        ResourceHandle(AssetId assetId, u32 generation)
            : m_assetId(assetId)
            , m_generation(generation)
        {
        }

        AssetId m_assetId;
        u32 m_generation = 0;
    };

    using MeshHandle = ResourceHandle<MeshAssetId, Mesh, ResourceType::Mesh>;
    using ShaderHandle = ResourceHandle<ShaderAssetId, ShaderProgram, ResourceType::Shader>;
    using TextureHandle = ResourceHandle<TextureAssetId, TextureResource, ResourceType::Texture>;
    using MaterialHandle = ResourceHandle<MaterialAssetId, Material, ResourceType::Material>;

    template <typename HandleType, typename AssetIdHash>
    struct ResourceHandleHashBase
    {
        std::size_t operator()(const HandleType& handle) const noexcept
        {
            std::size_t value = AssetIdHash{}(handle.GetAssetId());
            value ^= static_cast<std::size_t>(handle.GetGeneration()) +
                0x9e3779b97f4a7c15ULL + (value << 6U) + (value >> 2U);
            return value;
        }
    };

    using MeshHandleHash = ResourceHandleHashBase<MeshHandle, MeshAssetIdHash>;
    using ShaderHandleHash = ResourceHandleHashBase<ShaderHandle, ShaderAssetIdHash>;
    using TextureHandleHash = ResourceHandleHashBase<TextureHandle, TextureAssetIdHash>;
    using MaterialHandleHash = ResourceHandleHashBase<MaterialHandle, MaterialAssetIdHash>;
}
