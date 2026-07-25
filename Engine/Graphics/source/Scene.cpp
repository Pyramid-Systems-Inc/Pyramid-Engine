#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/Geometry/Mesh.hpp>
#include <Pyramid/Graphics/Geometry/Vertex.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace Pyramid
{
    namespace
    {
        Math::Quat NormalizeRotation(const Math::Quat& value)
        {
            return Math::IsZero(value.LengthSquared())
                ? Math::Quat::Identity
                : value.Normalized();
        }

        Math::Vec3 ExtractWorldScale(const Math::Mat4& transform)
        {
            const auto length = [](f32 x, f32 y, f32 z)
            {
                return std::sqrt(x * x + y * y + z * z);
            };
            return Math::Vec3(
                length(transform.m[0], transform.m[1], transform.m[2]),
                length(transform.m[4], transform.m[5], transform.m[6]),
                length(transform.m[8], transform.m[9], transform.m[10]));
        }

        Math::Vec3 ExtractTranslation(const Math::Mat4& transform)
        {
            return Math::Vec3(transform.m[12], transform.m[13], transform.m[14]);
        }

        void CanonicalizeBounds(Math::Vec3& minimum, Math::Vec3& maximum)
        {
            const Math::Vec3 canonicalMinimum(
                Math::Min(minimum.x, maximum.x),
                Math::Min(minimum.y, maximum.y),
                Math::Min(minimum.z, maximum.z));
            const Math::Vec3 canonicalMaximum(
                Math::Max(minimum.x, maximum.x),
                Math::Max(minimum.y, maximum.y),
                Math::Max(minimum.z, maximum.z));
            minimum = canonicalMinimum;
            maximum = canonicalMaximum;
        }
    }

    std::string EntityId::ToString() const
    {
        std::ostringstream stream;
        stream << std::hex << std::setfill('0') << std::setw(16) << m_value;
        return stream.str();
    }

    bool EntityId::TryParse(std::string_view text, EntityId& output)
    {
        if (text.empty() || text.size() > 16)
        {
            return false;
        }
        u64 value = 0;
        const auto result = std::from_chars(
            text.data(), text.data() + text.size(), value, 16);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value == 0)
        {
            return false;
        }
        output = EntityId(value);
        return true;
    }

    Math::Mat4 RenderObject::GetTransformMatrix() const
    {
        if (hasWorldTransform)
        {
            return worldTransform;
        }
        const Math::Mat4 scaleMatrix = Math::Mat4::CreateScale(scale);
        const Math::Mat4 rotationMatrix = NormalizeRotation(rotation).ToMatrix4();
        const Math::Mat4 translationMatrix = Math::Mat4::CreateTranslation(position);
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    Math::Vec3 RenderObject::GetWorldPosition() const
    {
        return hasWorldTransform ? ExtractTranslation(worldTransform) : position;
    }

    void RenderObject::SetWorldPosition(const Math::Vec3& value)
    {
        position = value;
        hasWorldTransform = false;
    }

    void RenderObject::SetWorldTransform(const Math::Mat4& value)
    {
        worldTransform = value;
        position = ExtractTranslation(value);
        scale = ExtractWorldScale(value);
        hasWorldTransform = true;
    }

    void RenderObject::ClearWorldTransformOverride()
    {
        hasWorldTransform = false;
        worldTransform = Math::Mat4::Identity;
    }

    void RenderObject::SetLocalBounds(const Math::Vec3& minPoint, const Math::Vec3& maxPoint)
    {
        localBoundsMin = minPoint;
        localBoundsMax = maxPoint;
        CanonicalizeBounds(localBoundsMin, localBoundsMax);
        boundsMode = RenderBoundsMode::Manual;
    }

    bool RenderObject::SetMeshHandle(MeshHandle handle, const ResourceRegistry& registry)
    {
        const auto resolved = registry.Resolve(handle);
        if (!resolved || !resolved->IsValid())
        {
            return false;
        }
        resolved->GetLocalBounds(handleBoundsMin, handleBoundsMax);
        hasHandleBounds = true;
        meshHandle = handle;
        mesh.reset();
        return true;
    }

    bool RenderObject::SetMaterialHandle(MaterialHandle handle, const ResourceRegistry& registry)
    {
        const auto resolved = registry.Resolve(handle);
        if (!resolved || !resolved->IsValid())
        {
            return false;
        }
        materialHandle = handle;
        material.reset();
        return true;
    }

    std::shared_ptr<Mesh> RenderObject::ResolveMesh(const ResourceRegistry* registry) const
    {
        return mesh ? mesh : (registry ? registry->Resolve(meshHandle) : nullptr);
    }

    std::shared_ptr<Material> RenderObject::ResolveMaterial(const ResourceRegistry* registry) const
    {
        return material ? material : (registry ? registry->Resolve(materialHandle) : nullptr);
    }

    bool RenderObject::TryGetGeometryBounds(Math::Vec3& minPoint, Math::Vec3& maxPoint) const
    {
        if (mesh && mesh->IsValid())
        {
            mesh->GetLocalBounds(minPoint, maxPoint);
            return true;
        }
        if (meshHandle && hasHandleBounds)
        {
            minPoint = handleBoundsMin;
            maxPoint = handleBoundsMax;
            return true;
        }
        return false;
    }

    bool RenderObject::GetLocalBounds(Math::Vec3& minPoint, Math::Vec3& maxPoint) const
    {
        const bool geometryBounds =
            boundsMode == RenderBoundsMode::Automatic && TryGetGeometryBounds(minPoint, maxPoint);
        if (!geometryBounds)
        {
            minPoint = localBoundsMin;
            maxPoint = localBoundsMax;
        }
        CanonicalizeBounds(minPoint, maxPoint);
        return geometryBounds;
    }

    void RenderObject::GetWorldBounds(Math::Vec3& minPoint, Math::Vec3& maxPoint) const
    {
        Math::Vec3 localMin;
        Math::Vec3 localMax;
        GetLocalBounds(localMin, localMax);
        const Math::Mat4 transform = GetTransformMatrix();
        const f32 maximum = std::numeric_limits<f32>::max();
        minPoint = Math::Vec3(maximum);
        maxPoint = Math::Vec3(-maximum);
        for (u32 corner = 0; corner < 8; ++corner)
        {
            const Math::Vec3 localCorner(
                (corner & 1u) ? localMax.x : localMin.x,
                (corner & 2u) ? localMax.y : localMin.y,
                (corner & 4u) ? localMax.z : localMin.z);
            const Math::Vec3 worldCorner =
                (transform * Math::Vec4(localCorner, 1.0f)).ToVec3();
            minPoint.x = Math::Min(minPoint.x, worldCorner.x);
            minPoint.y = Math::Min(minPoint.y, worldCorner.y);
            minPoint.z = Math::Min(minPoint.z, worldCorner.z);
            maxPoint.x = Math::Max(maxPoint.x, worldCorner.x);
            maxPoint.y = Math::Max(maxPoint.y, worldCorner.y);
            maxPoint.z = Math::Max(maxPoint.z, worldCorner.z);
        }
    }

    Scene::Scene(const std::string& name) : m_name(name) {}

    EntityId Scene::AllocateEntityId()
    {
        while (m_nextEntityId == 0 || Contains(EntityId(m_nextEntityId)))
        {
            ++m_nextEntityId;
        }
        return EntityId(m_nextEntityId++);
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithId(AllocateEntityId(), name);
    }

    Entity Scene::CreateEntityWithId(EntityId id, const std::string& name)
    {
        if (!id || Contains(id))
        {
            return {};
        }
        EntityRecord record;
        record.id = id;
        record.name = name;
        m_entities.emplace(id, std::move(record));
        m_entityOrder.push_back(id);
        if (id.GetValue() >= m_nextEntityId)
        {
            m_nextEntityId = id.GetValue() + 1;
        }
        return MakeEntity(id);
    }

    bool Scene::DestroyEntity(Entity entity)
    {
        return entity.GetScene() == this && DestroyEntity(entity.GetId());
    }

    bool Scene::DestroyEntity(EntityId id)
    {
        EntityRecord* record = FindRecord(id);
        if (!record)
        {
            return false;
        }
        const std::vector<EntityId> children = record->children;
        for (EntityId child : children)
        {
            DestroyEntity(child);
        }
        if (record->parent)
        {
            if (EntityRecord* parent = FindRecord(record->parent))
            {
                parent->children.erase(
                    std::remove(parent->children.begin(), parent->children.end(), id),
                    parent->children.end());
            }
        }
        if (m_primaryLight == id)
        {
            m_primaryLight = {};
        }
        m_entities.erase(id);
        m_entityOrder.erase(
            std::remove(m_entityOrder.begin(), m_entityOrder.end(), id),
            m_entityOrder.end());
        return true;
    }

    bool Scene::Contains(EntityId id) const
    {
        return id && m_entities.find(id) != m_entities.end();
    }

    Scene::EntityRecord* Scene::FindRecord(EntityId id)
    {
        const auto iterator = m_entities.find(id);
        return iterator == m_entities.end() ? nullptr : &iterator->second;
    }

    const Scene::EntityRecord* Scene::FindRecord(EntityId id) const
    {
        const auto iterator = m_entities.find(id);
        return iterator == m_entities.end() ? nullptr : &iterator->second;
    }

    Entity Scene::MakeEntity(EntityId id) const
    {
        return Contains(id) ? Entity(const_cast<Scene*>(this), id, m_lifetimeToken) : Entity{};
    }

    Entity Scene::FindEntity(EntityId id) { return MakeEntity(id); }
    Entity Scene::FindEntity(EntityId id) const { return MakeEntity(id); }

    Entity Scene::FindEntity(const std::string& name)
    {
        for (EntityId id : m_entityOrder)
        {
            const EntityRecord* record = FindRecord(id);
            if (record && record->name == name)
            {
                return MakeEntity(id);
            }
        }
        return {};
    }

    Entity Scene::FindEntity(const std::string& name) const
    {
        return const_cast<Scene*>(this)->FindEntity(name);
    }

    std::vector<Entity> Scene::GetEntities()
    {
        std::vector<Entity> entities;
        entities.reserve(m_entityOrder.size());
        for (EntityId id : m_entityOrder)
        {
            entities.push_back(Entity(this, id, m_lifetimeToken));
        }
        return entities;
    }

    std::vector<Entity> Scene::GetEntities() const
    {
        return const_cast<Scene*>(this)->GetEntities();
    }

    std::vector<Entity> Scene::GetRootEntities()
    {
        std::vector<Entity> entities;
        for (EntityId id : m_entityOrder)
        {
            const EntityRecord* record = FindRecord(id);
            if (record && !record->parent)
            {
                entities.push_back(Entity(this, id, m_lifetimeToken));
            }
        }
        return entities;
    }

    std::vector<Entity> Scene::GetRootEntities() const
    {
        return const_cast<Scene*>(this)->GetRootEntities();
    }

    bool Scene::IsDescendant(EntityId candidate, EntityId ancestor) const
    {
        EntityId current = candidate;
        while (current)
        {
            if (current == ancestor)
            {
                return true;
            }
            const EntityRecord* record = FindRecord(current);
            current = record ? record->parent : EntityId{};
        }
        return false;
    }

    bool Scene::SetParent(EntityId childId, EntityId parentId)
    {
        EntityRecord* child = FindRecord(childId);
        if (!child || childId == parentId || (parentId && !FindRecord(parentId)) ||
            (parentId && IsDescendant(parentId, childId)))
        {
            return false;
        }
        if (child->parent == parentId)
        {
            return true;
        }
        if (child->parent)
        {
            EntityRecord* oldParent = FindRecord(child->parent);
            if (oldParent)
            {
                oldParent->children.erase(
                    std::remove(oldParent->children.begin(), oldParent->children.end(), childId),
                    oldParent->children.end());
            }
        }
        child->parent = parentId;
        if (parentId)
        {
            FindRecord(parentId)->children.push_back(childId);
        }
        MarkWorldDirty(childId);
        return true;
    }

    bool Scene::SetLocalTransform(EntityId id, const TransformComponent& transform)
    {
        EntityRecord* record = FindRecord(id);
        if (!record)
        {
            return false;
        }
        record->transform = transform;
        record->transform.rotation = NormalizeRotation(record->transform.rotation);
        record->localDirty = true;
        MarkWorldDirty(id);
        return true;
    }

    void Scene::MarkWorldDirty(EntityId id)
    {
        EntityRecord* record = FindRecord(id);
        if (!record)
        {
            return;
        }
        record->worldDirty = true;
        for (EntityId child : record->children)
        {
            MarkWorldDirty(child);
        }
    }

    const Math::Mat4& Scene::GetLocalMatrix(EntityId id) const
    {
        EntityRecord* record = const_cast<EntityRecord*>(FindRecord(id));
        static const Math::Mat4 identity = Math::Mat4::Identity;
        if (!record)
        {
            return identity;
        }
        if (record->localDirty)
        {
            record->localMatrix =
                Math::Mat4::CreateTranslation(record->transform.position) *
                NormalizeRotation(record->transform.rotation).ToMatrix4() *
                Math::Mat4::CreateScale(record->transform.scale);
            record->localDirty = false;
        }
        return record->localMatrix;
    }

    const Math::Mat4& Scene::GetWorldMatrix(EntityId id) const
    {
        EntityRecord* record = const_cast<EntityRecord*>(FindRecord(id));
        static const Math::Mat4 identity = Math::Mat4::Identity;
        if (!record)
        {
            return identity;
        }
        if (record->worldDirty)
        {
            if (record->parent)
            {
                const EntityRecord* parent = FindRecord(record->parent);
                record->worldMatrix = GetWorldMatrix(record->parent) * GetLocalMatrix(id);
                record->worldRotation = NormalizeRotation(
                    (parent ? parent->worldRotation : Math::Quat::Identity) *
                    record->transform.rotation);
            }
            else
            {
                record->worldMatrix = GetLocalMatrix(id);
                record->worldRotation = NormalizeRotation(record->transform.rotation);
            }
            record->worldScale = ExtractWorldScale(record->worldMatrix);
            record->worldDirty = false;
        }
        return record->worldMatrix;
    }

    Math::Quat Scene::GetWorldRotation(EntityId id) const
    {
        GetWorldMatrix(id);
        const EntityRecord* record = FindRecord(id);
        return record ? record->worldRotation : Math::Quat::Identity;
    }

    Math::Vec3 Scene::GetWorldScale(EntityId id) const
    {
        GetWorldMatrix(id);
        const EntityRecord* record = FindRecord(id);
        return record ? record->worldScale : Math::Vec3::One;
    }

    bool Scene::IsEffectivelyVisible(EntityId id) const
    {
        const EntityRecord* record = FindRecord(id);
        if (!record || !record->visible)
        {
            return false;
        }
        return !record->parent || IsEffectivelyVisible(record->parent);
    }

    bool Scene::SetMeshRenderer(
        EntityId id,
        const MeshRendererComponent& input,
        const ResourceRegistry* registry)
    {
        EntityRecord* record = FindRecord(id);
        if (!record)
        {
            return false;
        }
        MeshRendererComponent component = input;
        CanonicalizeBounds(component.localBoundsMin, component.localBoundsMax);
        CanonicalizeBounds(component.meshBoundsMin, component.meshBoundsMax);
        if (registry)
        {
            if (component.mesh)
            {
                const auto mesh = registry->Resolve(component.mesh);
                if (!mesh || !mesh->IsValid())
                {
                    return false;
                }
                mesh->GetLocalBounds(component.meshBoundsMin, component.meshBoundsMax);
                component.hasMeshBounds = true;
            }
            if (component.material && !registry->Resolve(component.material))
            {
                return false;
            }
        }
        record->meshRenderer = component;
        if (!record->renderProxy)
        {
            record->renderProxy = std::make_shared<RenderObject>();
        }
        return true;
    }

    bool Scene::SetLight(EntityId id, const LightComponent& input)
    {
        EntityRecord* record = FindRecord(id);
        if (!record)
        {
            return false;
        }
        LightComponent component = input;
        component.localDirection = Math::IsZero(component.localDirection.LengthSquared())
            ? Math::Vec3(0.0f, -1.0f, 0.0f)
            : component.localDirection.Normalized();
        record->light = component;
        if (!record->lightProxy)
        {
            record->lightProxy = std::make_shared<Light>();
        }
        return true;
    }

    void Scene::SynchronizeRenderProxies() const
    {
        m_renderObjects.clear();
        for (EntityId id : m_entityOrder)
        {
            EntityRecord* record = const_cast<EntityRecord*>(FindRecord(id));
            if (!record || !record->meshRenderer)
            {
                continue;
            }
            if (!record->renderProxy)
            {
                record->renderProxy = std::make_shared<RenderObject>();
            }
            RenderObject& proxy = *record->renderProxy;
            if (record->legacyRenderProxyAuthoring)
            {
                record->transform.position = proxy.position;
                record->transform.rotation = NormalizeRotation(proxy.rotation);
                record->transform.scale = proxy.scale;
                record->localDirty = true;
                const_cast<Scene*>(this)->MarkWorldDirty(id);
                record->meshRenderer->mesh = proxy.meshHandle;
                record->meshRenderer->material = proxy.materialHandle;
                record->meshRenderer->visible = proxy.visible;
                record->meshRenderer->castShadows = proxy.castShadows;
                record->meshRenderer->receiveShadows = proxy.receiveShadows;
                record->meshRenderer->boundsMode = proxy.boundsMode;
                proxy.GetLocalBounds(
                    record->meshRenderer->localBoundsMin,
                    record->meshRenderer->localBoundsMax);
                if (proxy.TryGetGeometryBounds(
                        record->meshRenderer->meshBoundsMin,
                        record->meshRenderer->meshBoundsMax))
                {
                    record->meshRenderer->hasMeshBounds = true;
                }
            }
            const MeshRendererComponent& component = *record->meshRenderer;
            proxy.entityId = id;
            proxy.name = record->name;
            proxy.SetWorldTransform(GetWorldMatrix(id));
            proxy.rotation = GetWorldRotation(id);
            proxy.meshHandle = component.mesh;
            proxy.materialHandle = component.material;
            proxy.visible = IsEffectivelyVisible(id) && component.visible;
            proxy.castShadows = component.castShadows;
            proxy.receiveShadows = component.receiveShadows;
            proxy.boundsMode = component.boundsMode;
            proxy.localBoundsMin = component.localBoundsMin;
            proxy.localBoundsMax = component.localBoundsMax;
            proxy.handleBoundsMin = component.meshBoundsMin;
            proxy.handleBoundsMax = component.meshBoundsMax;
            proxy.hasHandleBounds = component.hasMeshBounds;
            m_renderObjects.push_back(record->renderProxy);
        }
    }

    void Scene::SynchronizeLightProxies() const
    {
        m_lights.clear();
        for (EntityId id : m_entityOrder)
        {
            EntityRecord* record = const_cast<EntityRecord*>(FindRecord(id));
            if (!record || !record->light)
            {
                continue;
            }
            if (!record->lightProxy)
            {
                record->lightProxy = std::make_shared<Light>();
            }
            Light& proxy = *record->lightProxy;
            const LightComponent& component = *record->light;
            proxy.entityId = id;
            proxy.name = record->name;
            proxy.type = component.type;
            proxy.position = ExtractTranslation(GetWorldMatrix(id));
            const Math::Vec3 direction =
                (GetWorldMatrix(id) * Math::Vec4(component.localDirection, 0.0f)).ToVec3();
            proxy.direction = Math::IsZero(direction.LengthSquared())
                ? component.localDirection
                : direction.Normalized();
            proxy.color = component.color;
            proxy.intensity = component.intensity;
            proxy.range = component.range;
            proxy.innerConeAngle = component.innerConeAngle;
            proxy.outerConeAngle = component.outerConeAngle;
            proxy.castShadows = component.castShadows;
            proxy.shadowBias = component.shadowBias;
            proxy.shadowMapSize = component.shadowMapSize;
            proxy.enabled = IsEffectivelyVisible(id) && component.enabled;
            m_lights.push_back(record->lightProxy);
        }
    }

    const std::vector<std::shared_ptr<RenderObject>>& Scene::GetRenderObjects() const
    {
        SynchronizeRenderProxies();
        return m_renderObjects;
    }

    const std::vector<std::shared_ptr<Light>>& Scene::GetLights() const
    {
        SynchronizeLightProxies();
        return m_lights;
    }

    Entity Scene::AddRenderObject(std::shared_ptr<RenderObject> object)
    {
        if (!object)
        {
            return {};
        }
        Entity entity = CreateEntity(object->name.empty() ? "RenderObject" : object->name);
        TransformComponent transform;
        transform.position = object->position;
        transform.rotation = object->rotation;
        transform.scale = object->scale;
        SetLocalTransform(entity.GetId(), transform);
        MeshRendererComponent component;
        component.mesh = object->meshHandle;
        component.material = object->materialHandle;
        component.visible = object->visible;
        component.castShadows = object->castShadows;
        component.receiveShadows = object->receiveShadows;
        component.boundsMode = object->boundsMode;
        object->GetLocalBounds(component.localBoundsMin, component.localBoundsMax);
        if (object->TryGetGeometryBounds(component.meshBoundsMin, component.meshBoundsMax))
        {
            component.hasMeshBounds = true;
        }
        SetMeshRenderer(entity.GetId(), component, nullptr);
        EntityRecord* record = FindRecord(entity.GetId());
        record->renderProxy = object;
        record->legacyRenderProxyAuthoring = true;
        object->entityId = entity.GetId();
        return entity;
    }

    void Scene::RemoveRenderObject(std::shared_ptr<RenderObject> object)
    {
        if (!object)
        {
            return;
        }
        for (EntityId id : m_entityOrder)
        {
            const EntityRecord* record = FindRecord(id);
            if (record && record->renderProxy == object)
            {
                DestroyEntity(id);
                return;
            }
        }
    }

    Entity Scene::AddLight(std::shared_ptr<Light> light)
    {
        if (!light)
        {
            return {};
        }
        Entity entity = CreateEntity(light->name.empty() ? "Light" : light->name);
        TransformComponent transform;
        transform.position = light->position;
        SetLocalTransform(entity.GetId(), transform);
        LightComponent component;
        component.type = light->type;
        component.localDirection = light->direction;
        component.color = light->color;
        component.intensity = light->intensity;
        component.range = light->range;
        component.innerConeAngle = light->innerConeAngle;
        component.outerConeAngle = light->outerConeAngle;
        component.castShadows = light->castShadows;
        component.shadowBias = light->shadowBias;
        component.shadowMapSize = light->shadowMapSize;
        component.enabled = light->enabled;
        SetLight(entity.GetId(), component);
        EntityRecord* record = FindRecord(entity.GetId());
        record->lightProxy = light;
        record->legacyLightProxyAuthoring = true;
        light->entityId = entity.GetId();
        return entity;
    }

    void Scene::RemoveLight(std::shared_ptr<Light> light)
    {
        if (!light)
        {
            return;
        }
        for (EntityId id : m_entityOrder)
        {
            const EntityRecord* record = FindRecord(id);
            if (record && record->lightProxy == light)
            {
                DestroyEntity(id);
                return;
            }
        }
    }

    void Scene::SetPrimaryLight(Entity entity)
    {
        m_primaryLight = entity.GetScene() == this && entity.HasLight()
            ? entity.GetId()
            : EntityId{};
    }

    void Scene::SetPrimaryLight(std::shared_ptr<Light> light)
    {
        if (!light)
        {
            m_primaryLight = {};
            return;
        }
        if (light->entityId && Contains(light->entityId))
        {
            m_primaryLight = light->entityId;
            return;
        }
        SetPrimaryLight(AddLight(light));
    }

    Entity Scene::GetPrimaryLightEntity() const
    {
        return MakeEntity(m_primaryLight);
    }

    std::shared_ptr<Light> Scene::GetPrimaryLight() const
    {
        if (!m_primaryLight)
        {
            return nullptr;
        }
        SynchronizeLightProxies();
        const EntityRecord* record = FindRecord(m_primaryLight);
        return record ? record->lightProxy : nullptr;
    }

    std::vector<std::shared_ptr<RenderObject>> Scene::GetVisibleObjects(const Camera& camera) const
    {
        std::vector<std::shared_ptr<RenderObject>> visible;
        for (const auto& object : GetRenderObjects())
        {
            if (!object || !object->visible)
            {
                continue;
            }
            Math::Vec3 minimum;
            Math::Vec3 maximum;
            object->GetWorldBounds(minimum, maximum);
            if (camera.IsAABBVisible(minimum, maximum))
            {
                visible.push_back(object);
            }
        }
        return visible;
    }

    std::vector<std::shared_ptr<Light>> Scene::GetVisibleLights(const Camera& camera) const
    {
        (void)camera;
        std::vector<std::shared_ptr<Light>> visible;
        for (const auto& light : GetLights())
        {
            if (light && light->enabled)
            {
                visible.push_back(light);
            }
        }
        return visible;
    }

    void Scene::Clear()
    {
        m_entities.clear();
        m_entityOrder.clear();
        m_renderObjects.clear();
        m_lights.clear();
        m_primaryLight = {};
        m_nextEntityId = 1;
    }

    bool Entity::IsValid() const
    {
        return m_scene && !m_lifetime.expired() && m_scene->Contains(m_id);
    }

    std::string Entity::GetName() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record ? record->name : std::string{};
    }

    bool Entity::SetName(const std::string& name)
    {
        auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        if (!record) return false;
        record->name = name;
        return true;
    }

    bool Entity::IsVisible() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record && record->visible;
    }

    bool Entity::IsEffectivelyVisible() const
    {
        return IsValid() && m_scene->IsEffectivelyVisible(m_id);
    }

    bool Entity::SetVisible(bool visible)
    {
        auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        if (!record) return false;
        record->visible = visible;
        return true;
    }

    const TransformComponent* Entity::GetTransform() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record ? &record->transform : nullptr;
    }

    bool Entity::SetLocalTransform(
        const Math::Vec3& position,
        const Math::Quat& rotation,
        const Math::Vec3& scale)
    {
        TransformComponent transform;
        transform.position = position;
        transform.rotation = rotation;
        transform.scale = scale;
        return IsValid() && m_scene->SetLocalTransform(m_id, transform);
    }

    bool Entity::SetLocalPosition(const Math::Vec3& position)
    {
        const TransformComponent* current = GetTransform();
        return current && SetLocalTransform(position, current->rotation, current->scale);
    }

    bool Entity::SetLocalRotation(const Math::Quat& rotation)
    {
        const TransformComponent* current = GetTransform();
        return current && SetLocalTransform(current->position, rotation, current->scale);
    }

    bool Entity::SetLocalScale(const Math::Vec3& scale)
    {
        const TransformComponent* current = GetTransform();
        return current && SetLocalTransform(current->position, current->rotation, scale);
    }

    Math::Vec3 Entity::GetWorldPosition() const
    {
        return IsValid() ? ExtractTranslation(m_scene->GetWorldMatrix(m_id)) : Math::Vec3::Zero;
    }

    Math::Quat Entity::GetWorldRotation() const
    {
        return IsValid() ? m_scene->GetWorldRotation(m_id) : Math::Quat::Identity;
    }

    Math::Vec3 Entity::GetWorldScale() const
    {
        return IsValid() ? m_scene->GetWorldScale(m_id) : Math::Vec3::One;
    }

    Math::Mat4 Entity::GetLocalMatrix() const
    {
        return IsValid() ? m_scene->GetLocalMatrix(m_id) : Math::Mat4::Identity;
    }

    Math::Mat4 Entity::GetWorldMatrix() const
    {
        return IsValid() ? m_scene->GetWorldMatrix(m_id) : Math::Mat4::Identity;
    }

    Entity Entity::GetParent() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record ? m_scene->MakeEntity(record->parent) : Entity{};
    }

    std::vector<Entity> Entity::GetChildren() const
    {
        std::vector<Entity> children;
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        if (!record) return children;
        children.reserve(record->children.size());
        for (EntityId id : record->children)
        {
            children.push_back(Entity(m_scene, id, m_lifetime));
        }
        return children;
    }

    bool Entity::SetParent(Entity parent)
    {
        if (!IsValid() || (parent && parent.GetScene() != m_scene))
        {
            return false;
        }
        return m_scene->SetParent(m_id, parent ? parent.GetId() : EntityId{});
    }

    bool Entity::ClearParent() { return IsValid() && m_scene->SetParent(m_id, {}); }

    bool Entity::HasMeshRenderer() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record && record->meshRenderer.has_value();
    }

    const MeshRendererComponent* Entity::GetMeshRenderer() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record && record->meshRenderer ? &*record->meshRenderer : nullptr;
    }

    bool Entity::SetMeshRenderer(
        const MeshRendererComponent& component,
        const ResourceRegistry* registry)
    {
        return IsValid() && m_scene->SetMeshRenderer(m_id, component, registry);
    }

    bool Entity::RemoveMeshRenderer()
    {
        auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        if (!record) return false;
        record->meshRenderer.reset();
        record->renderProxy.reset();
        return true;
    }

    bool Entity::HasLight() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record && record->light.has_value();
    }

    const LightComponent* Entity::GetLight() const
    {
        const auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        return record && record->light ? &*record->light : nullptr;
    }

    bool Entity::SetLight(const LightComponent& component)
    {
        return IsValid() && m_scene->SetLight(m_id, component);
    }

    bool Entity::RemoveLight()
    {
        auto* record = IsValid() ? m_scene->FindRecord(m_id) : nullptr;
        if (!record) return false;
        record->light.reset();
        record->lightProxy.reset();
        if (m_scene->m_primaryLight == m_id)
        {
            m_scene->m_primaryLight = {};
        }
        return true;
    }

    namespace SceneUtils
    {
        std::shared_ptr<Scene> CreateTestScene()
        {
            auto scene = std::make_shared<Scene>("Test Scene");
            Entity sun = scene->CreateEntity("Directional Light");
            LightComponent light;
            light.type = LightType::Directional;
            light.localDirection = Math::Vec3(0.5f, -1.0f, 0.5f).Normalized();
            sun.SetLight(light);
            scene->SetPrimaryLight(sun);
            scene->GetEnvironment().skyColor = Math::Vec3(0.5f, 0.7f, 1.0f);
            return scene;
        }

        std::shared_ptr<RenderObject> CreateCube(f32 size)
        {
            auto object = std::make_shared<RenderObject>();
            object->name = "Cube";
            const f32 half = size * 0.5f;
            object->SetLocalBounds(Math::Vec3(-half), Math::Vec3(half));
            return object;
        }

        std::shared_ptr<RenderObject> CreateSphere(f32 radius, u32 segments)
        {
            (void)segments;
            auto object = std::make_shared<RenderObject>();
            object->name = "Sphere";
            object->SetLocalBounds(Math::Vec3(-radius), Math::Vec3(radius));
            return object;
        }

        std::shared_ptr<RenderObject> CreatePlane(f32 width, f32 height)
        {
            auto object = std::make_shared<RenderObject>();
            object->name = "Plane";
            object->SetLocalBounds(
                Math::Vec3(-width * 0.5f, 0.0f, -height * 0.5f),
                Math::Vec3(width * 0.5f, 0.0f, height * 0.5f));
            return object;
        }

        std::shared_ptr<Light> CreateDirectionalLight(
            const Math::Vec3& direction,
            const Math::Vec3& color,
            f32 intensity)
        {
            auto light = std::make_shared<Light>();
            light->type = LightType::Directional;
            light->direction = Math::IsZero(direction.LengthSquared())
                ? Math::Vec3(0.0f, -1.0f, 0.0f)
                : direction.Normalized();
            light->color = color;
            light->intensity = intensity;
            light->name = "Directional Light";
            return light;
        }
    }
}
