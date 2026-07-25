#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Graphics/Texture/TextureResource.hpp>
#include <Pyramid/Graphics/Texture/TextureCache.hpp>
#include <Pyramid/Graphics/Scene/SceneManager.hpp>
#include <Pyramid/Graphics/Scene/Octree.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLFramebuffer.hpp>
#include <Pyramid/Graphics/Geometry/MeshBounds.hpp>
#include <Pyramid/Graphics/Geometry/Mesh.hpp>
#include <Pyramid/Graphics/Geometry/MeshCache.hpp>
#include <Pyramid/Graphics/Shader/ShaderProgram.hpp>
#include <Pyramid/Graphics/Shader/ShaderCache.hpp>
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Material/MaterialCache.hpp>
#include <Pyramid/Graphics/Resources/ResourceHandle.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/Resources/ResourceManifest.hpp>
#include <Pyramid/Graphics/Scene/SceneSerializer.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace
{
    class GameLinkageProbe final : public Pyramid::Game
    {
    public:
        using Pyramid::Game::GetResourceRegistry;
        using Pyramid::Game::SetRenderSystem;
    };

    using Pyramid::ITexture2D;
    using Pyramid::TextureFormat;
    using Pyramid::TextureSpecification;
    using Pyramid::SceneManagement::SceneManager;

    using CreateTextureFromSpec = std::shared_ptr<ITexture2D> (*)(const TextureSpecification&, const void*);
    using CreateTextureFromFile = std::shared_ptr<ITexture2D> (*)(const std::string&, bool, bool);
    using CreateTextureBySize = std::shared_ptr<ITexture2D> (*)(Pyramid::u32, Pyramid::u32, TextureFormat);

    volatile CreateTextureFromSpec g_createTextureFromSpec =
        static_cast<CreateTextureFromSpec>(&ITexture2D::Create);
    volatile CreateTextureFromFile g_createTextureFromFile =
        static_cast<CreateTextureFromFile>(&ITexture2D::Create);
    volatile CreateTextureBySize g_createTextureBySize =
        static_cast<CreateTextureBySize>(&ITexture2D::Create);

    volatile decltype(&ITexture2D::CreateRenderTarget) g_createRenderTarget = &ITexture2D::CreateRenderTarget;
    volatile decltype(&ITexture2D::CreateDepthTarget) g_createDepthTarget = &ITexture2D::CreateDepthTarget;
    volatile decltype(&ITexture2D::CreateFromColor) g_createFromColor = &ITexture2D::CreateFromColor;

    using TextureAssetIdFromString = Pyramid::TextureAssetId (*)(std::string_view);
    volatile TextureAssetIdFromString g_textureAssetIdFromString =
        static_cast<TextureAssetIdFromString>(&Pyramid::TextureAssetId::FromString);
    volatile decltype(&Pyramid::TextureAssetId::ToString) g_textureAssetIdToString =
        &Pyramid::TextureAssetId::ToString;
    using CalculateMemoryTextureId = Pyramid::TextureAssetId (*)(
        const Pyramid::TextureResourceSpecification&);
    using CalculateFileTextureId = Pyramid::TextureAssetId (*)(
        const Pyramid::TextureFileSpecification&);
    volatile CalculateMemoryTextureId g_calculateMemoryTextureId =
        static_cast<CalculateMemoryTextureId>(&Pyramid::TextureResource::CalculateContentId);
    volatile CalculateFileTextureId g_calculateFileTextureId =
        static_cast<CalculateFileTextureId>(&Pyramid::TextureResource::CalculateContentId);
    volatile decltype(&Pyramid::TextureResource::Create) g_createTextureResource =
        &Pyramid::TextureResource::Create;
    volatile decltype(&Pyramid::TextureResource::CreateFromFile) g_createFileTextureResource =
        &Pyramid::TextureResource::CreateFromFile;
    using GetOrCreateMemoryTexture = std::shared_ptr<Pyramid::TextureResource>
        (Pyramid::TextureCache::*)(const Pyramid::TextureResourceSpecification&);
    using GetOrCreateFileTexture = std::shared_ptr<Pyramid::TextureResource>
        (Pyramid::TextureCache::*)(const Pyramid::TextureFileSpecification&);
    volatile GetOrCreateMemoryTexture g_getOrCreateMemoryTexture =
        static_cast<GetOrCreateMemoryTexture>(&Pyramid::TextureCache::GetOrCreate);
    volatile GetOrCreateFileTexture g_getOrCreateFileTexture =
        static_cast<GetOrCreateFileTexture>(&Pyramid::TextureCache::GetOrCreate);
    using ReloadTexture = bool (Pyramid::TextureCache::*)(Pyramid::TextureAssetId);
    using ReloadTextureReplacement = bool (Pyramid::TextureCache::*)(
        Pyramid::TextureAssetId, const Pyramid::TextureFileSpecification&);
    volatile ReloadTexture g_reloadTexture =
        static_cast<ReloadTexture>(&Pyramid::TextureCache::Reload);
    volatile ReloadTextureReplacement g_reloadTextureReplacement =
        static_cast<ReloadTextureReplacement>(&Pyramid::TextureCache::Reload);
    volatile decltype(&Pyramid::TextureCache::Find) g_findCachedTexture =
        &Pyramid::TextureCache::Find;
    volatile decltype(&Pyramid::TextureCache::Evict) g_evictCachedTexture =
        &Pyramid::TextureCache::Evict;
    volatile decltype(&Pyramid::TextureCache::CollectUnused) g_collectUnusedTextures =
        &Pyramid::TextureCache::CollectUnused;
    volatile decltype(&Pyramid::TextureCache::Clear) g_clearTextureCache =
        &Pyramid::TextureCache::Clear;
    volatile decltype(&Pyramid::TextureCache::GetStats) g_getTextureCacheStats =
        &Pyramid::TextureCache::GetStats;
    volatile decltype(&Pyramid::TextureCache::GetGeneration) g_getTextureGeneration =
        &Pyramid::TextureCache::GetGeneration;

    volatile decltype(&SceneManager::LoadScene) g_loadScene = &SceneManager::LoadScene;
    volatile decltype(&SceneManager::SaveScene) g_saveScene = &SceneManager::SaveScene;
    volatile decltype(&SceneManager::GetObjectsInBox) g_getObjectsInBox = &SceneManager::GetObjectsInBox;
    volatile decltype(&SceneManager::QueryScene) g_queryScene = &SceneManager::QueryScene;
    volatile decltype(&SceneManager::GetNearestObject) g_getNearestObject =
        &SceneManager::GetNearestObject;
    volatile decltype(&SceneManager::GetKNearestObjects) g_getKNearestObjects =
        &SceneManager::GetKNearestObjects;
    volatile decltype(&Pyramid::SceneManagement::AABB::DistanceSquaredToPoint) g_aabbDistanceSquared =
        &Pyramid::SceneManagement::AABB::DistanceSquaredToPoint;
    volatile decltype(&Pyramid::SceneManagement::AABB::DistanceToPoint) g_aabbDistance =
        &Pyramid::SceneManagement::AABB::DistanceToPoint;
    volatile decltype(&Pyramid::SceneManagement::Octree::FindNearest) g_findOctreeNearest =
        &Pyramid::SceneManagement::Octree::FindNearest;
    volatile decltype(&Pyramid::SceneManagement::Octree::FindKNearest) g_findOctreeKNearest =
        &Pyramid::SceneManagement::Octree::FindKNearest;
    volatile decltype(&Pyramid::SceneManagement::Octree::QueryPoint) g_queryOctreePoint =
        &Pyramid::SceneManagement::Octree::QueryPoint;
    volatile decltype(&Pyramid::SceneManagement::Octree::QuerySphere) g_queryOctreeSphere =
        &Pyramid::SceneManagement::Octree::QuerySphere;
    volatile decltype(&Pyramid::SceneManagement::Octree::QueryBox) g_queryOctreeBox =
        &Pyramid::SceneManagement::Octree::QueryBox;
    volatile decltype(&Pyramid::SceneManagement::Octree::QueryRay) g_queryOctreeRay =
        &Pyramid::SceneManagement::Octree::QueryRay;
    volatile decltype(&SceneManager::UpdateVisibility) g_updateVisibility = &SceneManager::UpdateVisibility;
    volatile decltype(&SceneManager::RegisterEventCallback) g_registerEvent = &SceneManager::RegisterEventCallback;
    volatile decltype(&SceneManager::TriggerEvent) g_triggerEvent = &SceneManager::TriggerEvent;
    volatile decltype(&SceneManager::DrawDebugInfo) g_drawDebugInfo = &SceneManager::DrawDebugInfo;
    volatile decltype(&Pyramid::SceneManagement::Octree::UpdateIfMoved) g_updateOctreeIfMoved =
        &Pyramid::SceneManagement::Octree::UpdateIfMoved;
    volatile decltype(&Pyramid::SceneManagement::Octree::Synchronize) g_synchronizeOctree =
        &Pyramid::SceneManagement::Octree::Synchronize;
    volatile decltype(&Pyramid::SceneManagement::Octree::Configure) g_configureOctree =
        &Pyramid::SceneManagement::Octree::Configure;
    volatile decltype(&Pyramid::SceneManagement::Octree::GetConfiguration) g_getOctreeConfiguration =
        &Pyramid::SceneManagement::Octree::GetConfiguration;
    volatile decltype(&Pyramid::SceneManagement::Octree::Compact) g_compactOctree =
        &Pyramid::SceneManagement::Octree::Compact;
    volatile decltype(&Pyramid::SceneManagement::Octree::GetStats) g_getOctreeStats =
        &Pyramid::SceneManagement::Octree::GetStats;
    volatile decltype(&Pyramid::SceneManagement::Octree::GetLastCompactionStats) g_getLastOctreeCompaction =
        &Pyramid::SceneManagement::Octree::GetLastCompactionStats;
    volatile decltype(&Pyramid::Camera::SetViewportSize) g_setCameraViewport =
        &Pyramid::Camera::SetViewportSize;
    volatile decltype(&Pyramid::Camera::GetFrustumPlanes) g_getCameraFrustumPlanes =
        &Pyramid::Camera::GetFrustumPlanes;
    volatile decltype(&Pyramid::Camera::IsAABBVisible) g_isCameraAABBVisible =
        &Pyramid::Camera::IsAABBVisible;
    volatile decltype(&Pyramid::RenderObject::TryGetGeometryBounds) g_tryGetGeometryBounds =
        &Pyramid::RenderObject::TryGetGeometryBounds;
    volatile decltype(&Pyramid::RenderObject::GetLocalBounds) g_getRenderObjectLocalBounds =
        &Pyramid::RenderObject::GetLocalBounds;
    volatile decltype(&Pyramid::RenderObject::UseAutomaticBounds) g_useAutomaticBounds =
        &Pyramid::RenderObject::UseAutomaticBounds;
    volatile decltype(&Pyramid::RenderObject::GetWorldBounds) g_getRenderObjectWorldBounds =
        &Pyramid::RenderObject::GetWorldBounds;
    volatile decltype(&Pyramid::RenderObject::SetMeshHandle) g_setRenderObjectMeshHandle =
        &Pyramid::RenderObject::SetMeshHandle;
    volatile decltype(&Pyramid::RenderObject::SetMaterialHandle) g_setRenderObjectMaterialHandle =
        &Pyramid::RenderObject::SetMaterialHandle;
    volatile decltype(&Pyramid::RenderObject::ResolveMesh) g_resolveRenderObjectMesh =
        &Pyramid::RenderObject::ResolveMesh;
    volatile decltype(&Pyramid::RenderObject::ResolveMaterial) g_resolveRenderObjectMaterial =
        &Pyramid::RenderObject::ResolveMaterial;
    volatile decltype(&Pyramid::Geometry::CalculateLocalBounds) g_calculateGeometryBounds =
        &Pyramid::Geometry::CalculateLocalBounds;
    using MeshAssetIdFromString = Pyramid::MeshAssetId (*)(std::string_view);
    volatile MeshAssetIdFromString g_meshAssetIdFromString =
        static_cast<MeshAssetIdFromString>(&Pyramid::MeshAssetId::FromString);
    volatile decltype(&Pyramid::MeshAssetId::ToString) g_meshAssetIdToString =
        &Pyramid::MeshAssetId::ToString;
    volatile decltype(&Pyramid::Mesh::CalculateContentId) g_calculateMeshContentId =
        &Pyramid::Mesh::CalculateContentId;
    volatile decltype(&Pyramid::Mesh::Create) g_createMesh = &Pyramid::Mesh::Create;
    volatile decltype(&Pyramid::Mesh::IsValid) g_isMeshValid = &Pyramid::Mesh::IsValid;
    volatile decltype(&Pyramid::Mesh::GetLocalBounds) g_getMeshLocalBounds =
        &Pyramid::Mesh::GetLocalBounds;
    volatile decltype(&Pyramid::MeshCache::GetOrCreate) g_getOrCreateCachedMesh =
        &Pyramid::MeshCache::GetOrCreate;
    volatile decltype(&Pyramid::MeshCache::Find) g_findCachedMesh =
        &Pyramid::MeshCache::Find;
    volatile decltype(&Pyramid::MeshCache::Evict) g_evictCachedMesh =
        &Pyramid::MeshCache::Evict;
    volatile decltype(&Pyramid::MeshCache::CollectUnused) g_collectUnusedMeshes =
        &Pyramid::MeshCache::CollectUnused;
    volatile decltype(&Pyramid::MeshCache::Clear) g_clearMeshCache =
        &Pyramid::MeshCache::Clear;
    volatile decltype(&Pyramid::MeshCache::GetStats) g_getMeshCacheStats =
        &Pyramid::MeshCache::GetStats;
    volatile decltype(&Pyramid::MeshCache::GetGeneration) g_getMeshGeneration =
        &Pyramid::MeshCache::GetGeneration;
    using ShaderAssetIdFromString = Pyramid::ShaderAssetId (*)(std::string_view);
    volatile ShaderAssetIdFromString g_shaderAssetIdFromString =
        static_cast<ShaderAssetIdFromString>(&Pyramid::ShaderAssetId::FromString);
    volatile decltype(&Pyramid::ShaderAssetId::ToString) g_shaderAssetIdToString =
        &Pyramid::ShaderAssetId::ToString;
    volatile decltype(&Pyramid::ShaderProgram::CalculateContentId) g_calculateShaderContentId =
        &Pyramid::ShaderProgram::CalculateContentId;
    volatile decltype(&Pyramid::ShaderProgram::Create) g_createShaderProgram =
        &Pyramid::ShaderProgram::Create;
    volatile decltype(&Pyramid::ShaderProgram::IsValid) g_isShaderProgramValid =
        &Pyramid::ShaderProgram::IsValid;
    volatile decltype(&Pyramid::ShaderCache::GetOrCreate) g_getOrCreateCachedShader =
        &Pyramid::ShaderCache::GetOrCreate;
    volatile decltype(&Pyramid::ShaderCache::Recompile) g_recompileCachedShader =
        &Pyramid::ShaderCache::Recompile;
    volatile decltype(&Pyramid::ShaderCache::Find) g_findCachedShader =
        &Pyramid::ShaderCache::Find;
    volatile decltype(&Pyramid::ShaderCache::Evict) g_evictCachedShader =
        &Pyramid::ShaderCache::Evict;
    volatile decltype(&Pyramid::ShaderCache::CollectUnused) g_collectUnusedShaders =
        &Pyramid::ShaderCache::CollectUnused;
    volatile decltype(&Pyramid::ShaderCache::Clear) g_clearShaderCache =
        &Pyramid::ShaderCache::Clear;
    volatile decltype(&Pyramid::ShaderCache::GetStats) g_getShaderCacheStats =
        &Pyramid::ShaderCache::GetStats;
    volatile decltype(&Pyramid::ShaderCache::GetGeneration) g_getShaderGeneration =
        &Pyramid::ShaderCache::GetGeneration;
    using MaterialAssetIdFromString = Pyramid::MaterialAssetId (*)(std::string_view);
    volatile MaterialAssetIdFromString g_materialAssetIdFromString =
        static_cast<MaterialAssetIdFromString>(&Pyramid::MaterialAssetId::FromString);
    volatile decltype(&Pyramid::MaterialAssetId::ToString) g_materialAssetIdToString =
        &Pyramid::MaterialAssetId::ToString;
    volatile decltype(&Pyramid::Material::CalculateContentId) g_calculateMaterialContentId =
        &Pyramid::Material::CalculateContentId;
    volatile decltype(&Pyramid::Material::Create) g_createMaterial =
        &Pyramid::Material::Create;
    volatile decltype(&Pyramid::Material::Apply) g_applyMaterial =
        &Pyramid::Material::Apply;
    volatile decltype(&Pyramid::MaterialCache::GetOrCreate) g_getOrCreateCachedMaterial =
        &Pyramid::MaterialCache::GetOrCreate;
    volatile decltype(&Pyramid::MaterialCache::Replace) g_replaceCachedMaterial =
        &Pyramid::MaterialCache::Replace;
    volatile decltype(&Pyramid::MaterialCache::Find) g_findCachedMaterial =
        &Pyramid::MaterialCache::Find;
    volatile decltype(&Pyramid::MaterialCache::Evict) g_evictCachedMaterial =
        &Pyramid::MaterialCache::Evict;
    volatile decltype(&Pyramid::MaterialCache::CollectUnused) g_collectUnusedMaterials =
        &Pyramid::MaterialCache::CollectUnused;
    volatile decltype(&Pyramid::MaterialCache::Clear) g_clearMaterialCache =
        &Pyramid::MaterialCache::Clear;
    volatile decltype(&Pyramid::MaterialCache::GetStats) g_getMaterialCacheStats =
        &Pyramid::MaterialCache::GetStats;
    volatile decltype(&Pyramid::MaterialCache::GetGeneration) g_getMaterialGeneration =
        &Pyramid::MaterialCache::GetGeneration;
    using AcquireTextureMemory = Pyramid::TextureHandle (Pyramid::ResourceRegistry::*)(
        const Pyramid::TextureResourceSpecification&);
    using AcquireTextureFile = Pyramid::TextureHandle (Pyramid::ResourceRegistry::*)(
        const Pyramid::TextureFileSpecification&);
    volatile decltype(&Pyramid::ResourceRegistry::AcquireMesh) g_acquireMeshHandle =
        &Pyramid::ResourceRegistry::AcquireMesh;
    volatile decltype(&Pyramid::ResourceRegistry::AcquireShader) g_acquireShaderHandle =
        &Pyramid::ResourceRegistry::AcquireShader;
    volatile AcquireTextureMemory g_acquireMemoryTextureHandle =
        static_cast<AcquireTextureMemory>(&Pyramid::ResourceRegistry::AcquireTexture);
    volatile AcquireTextureFile g_acquireFileTextureHandle =
        static_cast<AcquireTextureFile>(&Pyramid::ResourceRegistry::AcquireTexture);
    volatile decltype(&Pyramid::ResourceRegistry::AcquireMaterial) g_acquireMaterialHandle =
        &Pyramid::ResourceRegistry::AcquireMaterial;
    using ResolveMeshHandle = std::shared_ptr<Pyramid::Mesh>
        (Pyramid::ResourceRegistry::*)(Pyramid::MeshHandle) const;
    using ResolveShaderHandle = std::shared_ptr<Pyramid::ShaderProgram>
        (Pyramid::ResourceRegistry::*)(Pyramid::ShaderHandle) const;
    using ResolveTextureHandle = std::shared_ptr<Pyramid::TextureResource>
        (Pyramid::ResourceRegistry::*)(Pyramid::TextureHandle) const;
    using ResolveMaterialHandle = std::shared_ptr<Pyramid::Material>
        (Pyramid::ResourceRegistry::*)(Pyramid::MaterialHandle) const;
    volatile ResolveMeshHandle g_resolveMeshHandle =
        static_cast<ResolveMeshHandle>(&Pyramid::ResourceRegistry::Resolve);
    volatile ResolveShaderHandle g_resolveShaderHandle =
        static_cast<ResolveShaderHandle>(&Pyramid::ResourceRegistry::Resolve);
    volatile ResolveTextureHandle g_resolveTextureHandle =
        static_cast<ResolveTextureHandle>(&Pyramid::ResourceRegistry::Resolve);
    volatile ResolveMaterialHandle g_resolveMaterialHandle =
        static_cast<ResolveMaterialHandle>(&Pyramid::ResourceRegistry::Resolve);
    volatile decltype(&Pyramid::ResourceRegistry::RecompileShader) g_recompileShaderHandle =
        &Pyramid::ResourceRegistry::RecompileShader;
    volatile decltype(&Pyramid::ResourceRegistry::ReplaceMaterial) g_replaceMaterialHandle =
        &Pyramid::ResourceRegistry::ReplaceMaterial;
    volatile decltype(&Pyramid::ResourceRegistry::CollectUnused) g_collectUnusedResources =
        &Pyramid::ResourceRegistry::CollectUnused;
    volatile decltype(&Pyramid::ResourceRegistry::Clear) g_clearResourceRegistry =
        &Pyramid::ResourceRegistry::Clear;
    volatile decltype(&Pyramid::ResourceRegistry::GetStats) g_getResourceRegistryStats =
        &Pyramid::ResourceRegistry::GetStats;
    using AddMeshManifestEntry = bool (Pyramid::ResourceManifest::*)(
        std::string, Pyramid::MeshHandle);
    volatile AddMeshManifestEntry g_addMeshManifestEntry =
        static_cast<AddMeshManifestEntry>(&Pyramid::ResourceManifest::Add);
    volatile decltype(&Pyramid::ResourceManifest::Serialize) g_serializeResourceManifest =
        &Pyramid::ResourceManifest::Serialize;
    volatile decltype(&Pyramid::ResourceManifest::Deserialize) g_deserializeResourceManifest =
        &Pyramid::ResourceManifest::Deserialize;
    volatile decltype(&Pyramid::ResourceManifest::Restore) g_restoreResourceManifest =
        &Pyramid::ResourceManifest::Restore;
    volatile decltype(&Pyramid::ResourceManifest::GetMeshHandle) g_getManifestMeshHandle =
        &Pyramid::ResourceManifest::GetMeshHandle;
    volatile decltype(&Pyramid::SceneSerializer::Serialize) g_serializeScene =
        &Pyramid::SceneSerializer::Serialize;
    volatile decltype(&Pyramid::SceneSerializer::Deserialize) g_deserializeScene =
        &Pyramid::SceneSerializer::Deserialize;
    using GetRegistryMeshes = Pyramid::MeshCache& (Pyramid::ResourceRegistry::*)();
    using GetRegistryShaders = Pyramid::ShaderCache& (Pyramid::ResourceRegistry::*)();
    using GetRegistryTextures = Pyramid::TextureCache& (Pyramid::ResourceRegistry::*)();
    using GetRegistryMaterials = Pyramid::MaterialCache& (Pyramid::ResourceRegistry::*)();
    volatile GetRegistryMeshes g_getRegistryMeshes =
        static_cast<GetRegistryMeshes>(&Pyramid::ResourceRegistry::Meshes);
    volatile GetRegistryShaders g_getRegistryShaders =
        static_cast<GetRegistryShaders>(&Pyramid::ResourceRegistry::Shaders);
    volatile GetRegistryTextures g_getRegistryTextures =
        static_cast<GetRegistryTextures>(&Pyramid::ResourceRegistry::Textures);
    volatile GetRegistryMaterials g_getRegistryMaterials =
        static_cast<GetRegistryMaterials>(&Pyramid::ResourceRegistry::Materials);
    volatile decltype(&GameLinkageProbe::GetResourceRegistry) g_getGameResourceRegistry =
        &GameLinkageProbe::GetResourceRegistry;
    volatile decltype(&Pyramid::Renderer::CommandBuffer::SetMaterial) g_setMaterial =
        &Pyramid::Renderer::CommandBuffer::SetMaterial;
    volatile decltype(&Pyramid::Renderer::CommandBuffer::SetUniformMat4) g_setCommandUniformMat4 =
        &Pyramid::Renderer::CommandBuffer::SetUniformMat4;
    volatile decltype(&Pyramid::Renderer::CommandBuffer::DrawMesh) g_drawMesh =
        &Pyramid::Renderer::CommandBuffer::DrawMesh;
    volatile decltype(&Pyramid::OpenGLFramebuffer::Resize) g_resizeFramebuffer =
        &Pyramid::OpenGLFramebuffer::Resize;
    volatile decltype(&Pyramid::Renderer::RenderTarget::Resize) g_resizeRenderTarget =
        &Pyramid::Renderer::RenderTarget::Resize;
    volatile decltype(&Pyramid::Renderer::RenderSystem::Resize) g_resizeRenderSystem =
        &Pyramid::Renderer::RenderSystem::Resize;
    volatile decltype(&Pyramid::Renderer::RenderSystem::SetResourceRegistry) g_setRenderResourceRegistry =
        &Pyramid::Renderer::RenderSystem::SetResourceRegistry;
    volatile decltype(&GameLinkageProbe::SetRenderSystem) g_setRenderSystem =
        &GameLinkageProbe::SetRenderSystem;
    volatile decltype(&Pyramid::Entity::SetLocalPosition) g_setEntityLocalPosition =
        &Pyramid::Entity::SetLocalPosition;
    volatile decltype(&Pyramid::Entity::SetLocalRotation) g_setEntityLocalRotation =
        &Pyramid::Entity::SetLocalRotation;
    volatile decltype(&Pyramid::Entity::SetLocalScale) g_setEntityLocalScale =
        &Pyramid::Entity::SetLocalScale;
    volatile decltype(&Pyramid::Entity::SetParent) g_setEntityParent =
        &Pyramid::Entity::SetParent;
    volatile decltype(&Pyramid::Entity::GetWorldMatrix) g_getEntityWorldMatrix =
        &Pyramid::Entity::GetWorldMatrix;
    volatile decltype(&Pyramid::Entity::SetMeshRenderer) g_setEntityMeshRenderer =
        &Pyramid::Entity::SetMeshRenderer;
    volatile decltype(&Pyramid::Entity::SetLight) g_setEntityLight =
        &Pyramid::Entity::SetLight;
    volatile decltype(&Pyramid::Scene::CreateEntity) g_createEntity =
        &Pyramid::Scene::CreateEntity;
    volatile decltype(&Pyramid::Scene::CreateEntityWithId) g_createEntityWithId =
        &Pyramid::Scene::CreateEntityWithId;
    using DestroyEntityFunction = bool (Pyramid::Scene::*)(Pyramid::Entity);
    volatile DestroyEntityFunction g_destroyEntity =
        static_cast<DestroyEntityFunction>(&Pyramid::Scene::DestroyEntity);
}

int main()
{
    return g_createTextureFromSpec &&
                   g_createTextureFromFile &&
                   g_createTextureBySize &&
                   g_createRenderTarget &&
                   g_createDepthTarget &&
                   g_createFromColor &&
                   g_textureAssetIdFromString &&
                   g_textureAssetIdToString &&
                   g_calculateMemoryTextureId &&
                   g_calculateFileTextureId &&
                   g_createTextureResource &&
                   g_createFileTextureResource &&
                   g_getOrCreateMemoryTexture &&
                   g_getOrCreateFileTexture &&
                   g_reloadTexture &&
                   g_reloadTextureReplacement &&
                   g_findCachedTexture &&
                   g_evictCachedTexture &&
                   g_collectUnusedTextures &&
                   g_clearTextureCache &&
                   g_getTextureCacheStats &&
                   g_getTextureGeneration &&
                   g_loadScene &&
                   g_saveScene &&
                   g_getObjectsInBox &&
                   g_queryScene &&
                   g_getNearestObject &&
                   g_getKNearestObjects &&
                   g_aabbDistanceSquared &&
                   g_aabbDistance &&
                   g_findOctreeNearest &&
                   g_findOctreeKNearest &&
                   g_queryOctreePoint &&
                   g_queryOctreeSphere &&
                   g_queryOctreeBox &&
                   g_queryOctreeRay &&
                   g_updateVisibility &&
                   g_registerEvent &&
                   g_triggerEvent &&
                   g_drawDebugInfo &&
                   g_updateOctreeIfMoved &&
                   g_synchronizeOctree &&
                   g_configureOctree &&
                   g_getOctreeConfiguration &&
                   g_compactOctree &&
                   g_getOctreeStats &&
                   g_getLastOctreeCompaction &&
                   g_setCameraViewport &&
                   g_getCameraFrustumPlanes &&
                   g_isCameraAABBVisible &&
                   g_tryGetGeometryBounds &&
                   g_getRenderObjectLocalBounds &&
                   g_useAutomaticBounds &&
                   g_getRenderObjectWorldBounds &&
                   g_setRenderObjectMeshHandle &&
                   g_setRenderObjectMaterialHandle &&
                   g_resolveRenderObjectMesh &&
                   g_resolveRenderObjectMaterial &&
                   g_calculateGeometryBounds &&
                   g_meshAssetIdFromString &&
                   g_meshAssetIdToString &&
                   g_calculateMeshContentId &&
                   g_createMesh &&
                   g_isMeshValid &&
                   g_getMeshLocalBounds &&
                   g_getOrCreateCachedMesh &&
                   g_findCachedMesh &&
                   g_evictCachedMesh &&
                   g_collectUnusedMeshes &&
                   g_clearMeshCache &&
                   g_getMeshCacheStats &&
                   g_getMeshGeneration &&
                   g_shaderAssetIdFromString &&
                   g_shaderAssetIdToString &&
                   g_calculateShaderContentId &&
                   g_createShaderProgram &&
                   g_isShaderProgramValid &&
                   g_getOrCreateCachedShader &&
                   g_recompileCachedShader &&
                   g_findCachedShader &&
                   g_evictCachedShader &&
                   g_collectUnusedShaders &&
                   g_clearShaderCache &&
                   g_getShaderCacheStats &&
                   g_getShaderGeneration &&
                   g_materialAssetIdFromString &&
                   g_materialAssetIdToString &&
                   g_calculateMaterialContentId &&
                   g_createMaterial &&
                   g_applyMaterial &&
                   g_getOrCreateCachedMaterial &&
                   g_replaceCachedMaterial &&
                   g_findCachedMaterial &&
                   g_evictCachedMaterial &&
                   g_collectUnusedMaterials &&
                   g_clearMaterialCache &&
                   g_getMaterialCacheStats &&
                   g_getMaterialGeneration &&
                   g_acquireMeshHandle &&
                   g_acquireShaderHandle &&
                   g_acquireMemoryTextureHandle &&
                   g_acquireFileTextureHandle &&
                   g_acquireMaterialHandle &&
                   g_resolveMeshHandle &&
                   g_resolveShaderHandle &&
                   g_resolveTextureHandle &&
                   g_resolveMaterialHandle &&
                   g_recompileShaderHandle &&
                   g_replaceMaterialHandle &&
                   g_collectUnusedResources &&
                   g_clearResourceRegistry &&
                   g_getResourceRegistryStats &&
                   g_addMeshManifestEntry &&
                   g_serializeResourceManifest &&
                   g_deserializeResourceManifest &&
                   g_restoreResourceManifest &&
                   g_getManifestMeshHandle &&
                   g_serializeScene &&
                   g_deserializeScene &&
                   g_getRegistryMeshes &&
                   g_getRegistryShaders &&
                   g_getRegistryTextures &&
                   g_getRegistryMaterials &&
                   g_getGameResourceRegistry &&
                   g_setMaterial &&
                   g_setCommandUniformMat4 &&
                   g_drawMesh &&
                   g_resizeFramebuffer &&
                   g_resizeRenderTarget &&
                   g_resizeRenderSystem &&
                   g_setRenderResourceRegistry &&
                   g_setRenderSystem &&
                   g_setEntityLocalPosition &&
                   g_setEntityLocalRotation &&
                   g_setEntityLocalScale &&
                   g_setEntityParent &&
                   g_getEntityWorldMatrix &&
                   g_setEntityMeshRenderer &&
                   g_setEntityLight &&
                   g_createEntity &&
                   g_createEntityWithId &&
                   g_destroyEntity
               ? 0
               : 1;
}
