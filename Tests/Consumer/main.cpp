#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Graphics/Geometry/MeshCache.hpp>
#include <Pyramid/Graphics/Shader/ShaderCache.hpp>
#include <Pyramid/Graphics/Texture/TextureCache.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Material/MaterialCache.hpp>
#include <Pyramid/Graphics/Resources/ResourceHandle.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/Resources/ResourceManifest.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Scene/SceneSerializer.hpp>

#include <iostream>

int main()
{
    const Pyramid::Math::Vec3 value(1.0f, 2.0f, 3.0f);
    const auto meshId = Pyramid::MeshAssetId::FromString("consumer/mesh");
    const auto shaderId = Pyramid::ShaderAssetId::FromString("consumer/shader");
    const auto textureId = Pyramid::TextureAssetId::FromString("consumer/texture");
    const auto materialId = Pyramid::MaterialAssetId::FromString("consumer/material");
    Pyramid::MaterialCache materialCache;
    Pyramid::ResourceRegistryReleaseStats released;
    const auto meshHandle = Pyramid::MeshHandle::FromParts(meshId, 1);
    const auto materialHandle = Pyramid::MaterialHandle::FromParts(materialId, 1);
    Pyramid::ResourceManifest manifest;
    manifest.Add("consumer.mesh", meshHandle);
    Pyramid::Scene scene("Consumer Scene");
    const Pyramid::Entity entity = scene.CreateEntityWithId(
        Pyramid::EntityId(42),
        "Consumer Entity");
    Pyramid::SceneSerializationResult sceneSerialization;
    std::cout << "Pyramid Engine " << PYRAMID_VERSION_STRING
              << " | vector length: " << value.Length()
              << " | mesh id: " << meshId.ToString()
              << " | shader id: " << shaderId.ToString()
              << " | texture id: " << textureId.ToString()
              << " | material id: " << materialId.ToString()
              << " | mesh generation: " << meshHandle.GetGeneration()
              << " | material generation: " << materialHandle.GetGeneration()
              << " | cached materials: " << materialCache.GetResidentCount()
              << " | manifest entries: " << manifest.GetEntryCount()
              << " | serialized scene objects: " << sceneSerialization.serializedObjects
              << " | released resources: " << released.GetTotal() << '\n';
    return value.LengthSquared() > 0.0f && meshId.IsValid() && shaderId.IsValid() &&
        textureId.IsValid() && materialId.IsValid() && meshHandle.IsValid() &&
        materialHandle.IsValid() && entity.IsValid() && entity.GetId() == Pyramid::EntityId(42) &&
        manifest.GetMeshHandle("consumer.mesh") == meshHandle
        ? 0
        : 1;
}
