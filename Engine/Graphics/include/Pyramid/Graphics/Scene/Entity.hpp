#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Graphics/Resources/ResourceHandle.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Pyramid
{
    class ResourceRegistry;
    class Scene;

    /** Stable scene-local identifier persisted by the versioned scene format. */
    class EntityId final
    {
    public:
        constexpr EntityId() = default;
        explicit constexpr EntityId(u64 value) : m_value(value) {}

        bool IsValid() const { return m_value != 0; }
        explicit operator bool() const { return IsValid(); }
        u64 GetValue() const { return m_value; }

        std::string ToString() const;
        static bool TryParse(std::string_view text, EntityId& output);

        bool operator==(EntityId other) const { return m_value == other.m_value; }
        bool operator!=(EntityId other) const { return !(*this == other); }
        bool operator<(EntityId other) const { return m_value < other.m_value; }

    private:
        u64 m_value = 0;
    };

    struct EntityIdHash
    {
        std::size_t operator()(EntityId id) const noexcept
        {
            return static_cast<std::size_t>(id.GetValue() ^ (id.GetValue() >> 32U));
        }
    };

    enum class RenderBoundsMode : u8
    {
        Automatic,
        Manual
    };

    enum class LightType : u8
    {
        Directional,
        Point,
        Spot,
        Area
    };

    /** Every entity owns exactly one transform component. */
    struct TransformComponent
    {
        Math::Vec3 position = Math::Vec3::Zero;
        Math::Quat rotation = Math::Quat::Identity;
        Math::Vec3 scale = Math::Vec3::One;
    };

    /** Optional renderable state owned by one entity. */
    struct MeshRendererComponent
    {
        MeshHandle mesh;
        MaterialHandle material;
        bool visible = true;
        bool castShadows = true;
        bool receiveShadows = true;
        RenderBoundsMode boundsMode = RenderBoundsMode::Automatic;
        Math::Vec3 localBoundsMin = Math::Vec3(-0.5f);
        Math::Vec3 localBoundsMax = Math::Vec3(0.5f);
        Math::Vec3 meshBoundsMin = Math::Vec3(-0.5f);
        Math::Vec3 meshBoundsMax = Math::Vec3(0.5f);
        bool hasMeshBounds = false;
    };

    /** Optional light data. Position and orientation come from TransformComponent. */
    struct LightComponent
    {
        LightType type = LightType::Directional;
        Math::Vec3 localDirection = Math::Vec3(0.0f, -1.0f, 0.0f);
        Math::Vec3 color = Math::Vec3::One;
        f32 intensity = 1.0f;
        f32 range = 10.0f;
        f32 innerConeAngle = 30.0f;
        f32 outerConeAngle = 45.0f;
        bool castShadows = true;
        f32 shadowBias = 0.005f;
        u32 shadowMapSize = 1024;
        bool enabled = true;
    };

    /** Lightweight, non-owning scene entity facade. */
    class Entity final
    {
    public:
        Entity() = default;

        bool IsValid() const;
        explicit operator bool() const { return IsValid(); }
        EntityId GetId() const { return m_id; }
        Scene* GetScene() const { return m_lifetime.expired() ? nullptr : m_scene; }

        std::string GetName() const;
        bool SetName(const std::string& name);

        bool IsVisible() const;
        bool IsEffectivelyVisible() const;
        bool SetVisible(bool visible);

        const TransformComponent* GetTransform() const;
        bool SetLocalTransform(
            const Math::Vec3& position,
            const Math::Quat& rotation,
            const Math::Vec3& scale);
        bool SetLocalPosition(const Math::Vec3& position);
        bool SetLocalRotation(const Math::Quat& rotation);
        bool SetLocalScale(const Math::Vec3& scale);

        Math::Vec3 GetWorldPosition() const;
        Math::Quat GetWorldRotation() const;
        Math::Vec3 GetWorldScale() const;
        Math::Mat4 GetLocalMatrix() const;
        Math::Mat4 GetWorldMatrix() const;

        Entity GetParent() const;
        std::vector<Entity> GetChildren() const;
        bool SetParent(Entity parent);
        bool ClearParent();

        bool HasMeshRenderer() const;
        const MeshRendererComponent* GetMeshRenderer() const;
        bool SetMeshRenderer(
            const MeshRendererComponent& component,
            const ResourceRegistry* registry = nullptr);
        bool RemoveMeshRenderer();

        bool HasLight() const;
        const LightComponent* GetLight() const;
        bool SetLight(const LightComponent& component);
        bool RemoveLight();

        bool operator==(const Entity& other) const
        {
            return m_scene == other.m_scene && m_id == other.m_id;
        }

        bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        friend class Scene;
        Entity(Scene* scene, EntityId id, std::weak_ptr<u8> lifetime)
            : m_scene(scene), m_id(id), m_lifetime(std::move(lifetime)) {}

        Scene* m_scene = nullptr;
        EntityId m_id;
        std::weak_ptr<u8> m_lifetime;
    };
}
