#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Scene/Entity.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Pyramid
{
    class Camera;
    class Mesh;
    class ITexture2D;
    class ResourceRegistry;

    /**
     * Low-level rendering/spatial proxy.
     *
     * Scene authoring is entity/component based. Scene builds these proxies from
     * MeshRendererComponent data for the renderer and octree. Direct construction
     * remains supported for low-level rendering and spatial tests.
     */
    struct RenderObject
    {
        EntityId entityId;

        Math::Vec3 position = Math::Vec3::Zero;
        Math::Quat rotation = Math::Quat::Identity;
        Math::Vec3 scale = Math::Vec3::One;
        Math::Mat4 worldTransform = Math::Mat4::Identity;
        bool hasWorldTransform = false;

        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Material> material;
        MeshHandle meshHandle;
        MaterialHandle materialHandle;

        Math::Vec3 localBoundsMin = Math::Vec3(-0.5f);
        Math::Vec3 localBoundsMax = Math::Vec3(0.5f);
        RenderBoundsMode boundsMode = RenderBoundsMode::Automatic;
        Math::Vec3 handleBoundsMin = Math::Vec3(-0.5f);
        Math::Vec3 handleBoundsMax = Math::Vec3(0.5f);
        bool hasHandleBounds = false;

        std::string name;
        bool visible = true;
        bool castShadows = true;
        bool receiveShadows = true;

        Math::Mat4 GetTransformMatrix() const;
        Math::Vec3 GetWorldPosition() const;
        void SetWorldPosition(const Math::Vec3& value);
        void SetWorldTransform(const Math::Mat4& value);
        void ClearWorldTransformOverride();
        void SetLocalBounds(const Math::Vec3& minPoint, const Math::Vec3& maxPoint);
        void UseAutomaticBounds() { boundsMode = RenderBoundsMode::Automatic; }
        RenderBoundsMode GetBoundsMode() const { return boundsMode; }
        bool SetMeshHandle(MeshHandle handle, const ResourceRegistry& registry);
        bool SetMaterialHandle(MaterialHandle handle, const ResourceRegistry& registry);
        std::shared_ptr<Mesh> ResolveMesh(const ResourceRegistry* registry) const;
        std::shared_ptr<Material> ResolveMaterial(const ResourceRegistry* registry) const;
        bool TryGetGeometryBounds(Math::Vec3& minPoint, Math::Vec3& maxPoint) const;
        bool GetLocalBounds(Math::Vec3& minPoint, Math::Vec3& maxPoint) const;
        void GetWorldBounds(Math::Vec3& minPoint, Math::Vec3& maxPoint) const;
    };

    /** Renderer-facing light proxy generated from Entity + LightComponent. */
    struct Light
    {
        EntityId entityId;
        LightType type = LightType::Directional;
        Math::Vec3 position = Math::Vec3::Zero;
        Math::Vec3 direction = Math::Vec3(0.0f, -1.0f, 0.0f);
        Math::Vec3 color = Math::Vec3::One;
        f32 intensity = 1.0f;
        f32 range = 10.0f;
        f32 innerConeAngle = 30.0f;
        f32 outerConeAngle = 45.0f;
        bool castShadows = true;
        f32 shadowBias = 0.005f;
        u32 shadowMapSize = 1024;
        std::string name;
        bool enabled = true;
    };

    struct Environment
    {
        Math::Vec3 skyColor = Math::Vec3(0.5f, 0.7f, 1.0f);
        std::shared_ptr<ITexture2D> skyboxTexture;
        std::shared_ptr<ITexture2D> environmentMap;
        Math::Vec3 ambientColor = Math::Vec3(0.1f, 0.1f, 0.1f);
        f32 ambientIntensity = 1.0f;
        bool fogEnabled = false;
        Math::Vec3 fogColor = Math::Vec3(0.7f, 0.7f, 0.7f);
        f32 fogDensity = 0.01f;
        f32 fogStart = 10.0f;
        f32 fogEnd = 100.0f;
        f32 exposure = 1.0f;
        f32 gamma = 2.2f;
    };

    /**
     * Authoritative hybrid entity/component scene.
     *
     * Every entity has a stable EntityId, name, visibility flag, and
     * TransformComponent. MeshRendererComponent and LightComponent are optional.
     * Parent-child hierarchy is represented exclusively by entity IDs; the former
     * independent SceneNode transform graph has been removed.
     */
    class Scene
    {
    public:
        explicit Scene(const std::string& name = "Scene");
        ~Scene() = default;
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene&&) = delete;

        Entity CreateEntity(const std::string& name = "Entity");
        Entity CreateEntityWithId(EntityId id, const std::string& name = "Entity");
        bool DestroyEntity(Entity entity);
        bool DestroyEntity(EntityId id);
        bool Contains(EntityId id) const;
        Entity FindEntity(EntityId id);
        Entity FindEntity(EntityId id) const;
        Entity FindEntity(const std::string& name);
        Entity FindEntity(const std::string& name) const;
        std::vector<Entity> GetEntities();
        std::vector<Entity> GetEntities() const;
        std::vector<Entity> GetRootEntities();
        std::vector<Entity> GetRootEntities() const;
        size_t GetEntityCount() const { return m_entityOrder.size(); }

        // Transitional import helpers. New code should author Entity components.
        Entity AddRenderObject(std::shared_ptr<RenderObject> object);
        void RemoveRenderObject(std::shared_ptr<RenderObject> object);
        Entity AddLight(std::shared_ptr<Light> light);
        void RemoveLight(std::shared_ptr<Light> light);

        const std::vector<std::shared_ptr<RenderObject>>& GetRenderObjects() const;
        const std::vector<std::shared_ptr<Light>>& GetLights() const;

        void SetPrimaryLight(Entity entity);
        void SetPrimaryLight(std::shared_ptr<Light> light);
        Entity GetPrimaryLightEntity() const;
        std::shared_ptr<Light> GetPrimaryLight() const;

        Environment& GetEnvironment() { return m_environment; }
        const Environment& GetEnvironment() const { return m_environment; }

        std::vector<std::shared_ptr<RenderObject>> GetVisibleObjects(const Camera& camera) const;
        std::vector<std::shared_ptr<Light>> GetVisibleLights(const Camera& camera) const;

        void Clear();
        size_t GetObjectCount() const { return GetRenderObjects().size(); }
        size_t GetLightCount() const { return GetLights().size(); }

        const std::string& GetName() const { return m_name; }
        void SetName(const std::string& name) { m_name = name; }

    private:
        friend class Entity;
        friend class SceneSerializer;

        struct EntityRecord
        {
            EntityId id;
            std::string name;
            bool visible = true;
            TransformComponent transform;
            EntityId parent;
            std::vector<EntityId> children;
            std::optional<MeshRendererComponent> meshRenderer;
            std::optional<LightComponent> light;

            mutable Math::Mat4 localMatrix = Math::Mat4::Identity;
            mutable Math::Mat4 worldMatrix = Math::Mat4::Identity;
            mutable Math::Quat worldRotation = Math::Quat::Identity;
            mutable Math::Vec3 worldScale = Math::Vec3::One;
            mutable bool localDirty = true;
            mutable bool worldDirty = true;

            std::shared_ptr<RenderObject> renderProxy;
            std::shared_ptr<Light> lightProxy;
            bool legacyRenderProxyAuthoring = false;
            bool legacyLightProxyAuthoring = false;
        };

        EntityRecord* FindRecord(EntityId id);
        const EntityRecord* FindRecord(EntityId id) const;
        Entity MakeEntity(EntityId id) const;
        EntityId AllocateEntityId();
        bool IsDescendant(EntityId candidate, EntityId ancestor) const;
        bool SetParent(EntityId child, EntityId parent);
        bool SetLocalTransform(EntityId id, const TransformComponent& transform);
        void MarkWorldDirty(EntityId id);
        const Math::Mat4& GetLocalMatrix(EntityId id) const;
        const Math::Mat4& GetWorldMatrix(EntityId id) const;
        Math::Quat GetWorldRotation(EntityId id) const;
        Math::Vec3 GetWorldScale(EntityId id) const;
        bool IsEffectivelyVisible(EntityId id) const;
        bool SetMeshRenderer(
            EntityId id,
            const MeshRendererComponent& component,
            const ResourceRegistry* registry);
        bool SetLight(EntityId id, const LightComponent& component);
        void SynchronizeRenderProxies() const;
        void SynchronizeLightProxies() const;

        std::string m_name;
        std::shared_ptr<u8> m_lifetimeToken = std::make_shared<u8>(0);
        u64 m_nextEntityId = 1;
        std::unordered_map<EntityId, EntityRecord, EntityIdHash> m_entities;
        std::vector<EntityId> m_entityOrder;
        EntityId m_primaryLight;
        Environment m_environment;

        mutable std::vector<std::shared_ptr<RenderObject>> m_renderObjects;
        mutable std::vector<std::shared_ptr<Light>> m_lights;
    };

    namespace SceneUtils
    {
        std::shared_ptr<Scene> CreateTestScene();
        std::shared_ptr<RenderObject> CreateCube(f32 size = 1.0f);
        std::shared_ptr<RenderObject> CreateSphere(f32 radius = 1.0f, u32 segments = 32);
        std::shared_ptr<RenderObject> CreatePlane(f32 width = 1.0f, f32 height = 1.0f);
        std::shared_ptr<Light> CreateDirectionalLight(
            const Math::Vec3& direction,
            const Math::Vec3& color = Math::Vec3::One,
            f32 intensity = 1.0f);
    }
}
