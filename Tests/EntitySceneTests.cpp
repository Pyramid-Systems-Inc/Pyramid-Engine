#include <Pyramid/Graphics/Scene.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
    int Fail(const char* message)
    {
        std::cerr << "EntitySceneTests failure: " << message << '\n';
        return EXIT_FAILURE;
    }

    bool NearlyEqual(float left, float right, float epsilon = 0.0001f)
    {
        return std::fabs(left - right) <= epsilon;
    }

    bool NearlyEqual(const Pyramid::Math::Vec3& left, const Pyramid::Math::Vec3& right)
    {
        return NearlyEqual(left.x, right.x) && NearlyEqual(left.y, right.y) &&
            NearlyEqual(left.z, right.z);
    }
}

int main()
{
    using namespace Pyramid;

    Scene scene("Entity Scene");
    Entity root = scene.CreateEntityWithId(EntityId(0x10), "Root");
    Entity child = scene.CreateEntityWithId(EntityId(0x20), "Child");
    Entity grandchild = scene.CreateEntityWithId(EntityId(0x30), "Grandchild");
    if (!root || !child || !grandchild || scene.GetEntityCount() != 3)
    {
        return Fail("stable entity creation failed");
    }

    EntityId parsed;
    if (root.GetId().ToString() != "0000000000000010" ||
        !EntityId::TryParse(root.GetId().ToString(), parsed) || parsed != root.GetId() ||
        scene.CreateEntityWithId(root.GetId(), "Duplicate"))
    {
        return Fail("entity ID serialization or duplicate rejection failed");
    }

    root.SetLocalTransform(
        Math::Vec3(10.0f, 0.0f, 0.0f),
        Math::Quat::FromAxisAngle(Math::Vec3::Up, Math::Radians(90.0f)),
        Math::Vec3(2.0f));
    child.SetLocalPosition(Math::Vec3(1.0f, 0.0f, 0.0f));
    grandchild.SetLocalPosition(Math::Vec3(0.0f, 0.0f, 1.0f));
    if (!child.SetParent(root) || !grandchild.SetParent(child))
    {
        return Fail("entity parenting failed");
    }

    if (!NearlyEqual(child.GetWorldPosition(), Math::Vec3(10.0f, 0.0f, -2.0f)) ||
        !NearlyEqual(grandchild.GetWorldPosition(), Math::Vec3(12.0f, 0.0f, -2.0f)))
    {
        return Fail("hierarchical world transform composition failed");
    }

    root.SetLocalPosition(Math::Vec3(20.0f, 0.0f, 0.0f));
    if (!NearlyEqual(grandchild.GetWorldPosition(), Math::Vec3(22.0f, 0.0f, -2.0f)))
    {
        return Fail("descendant world-cache invalidation failed");
    }

    if (root.SetParent(grandchild) || child.SetParent(child) ||
        child.GetParent() != root || root.GetChildren().size() != 1)
    {
        return Fail("cycle, self-parent, or hierarchy enumeration validation failed");
    }

    root.SetVisible(false);
    if (grandchild.IsEffectivelyVisible())
    {
        return Fail("ancestor visibility was not inherited");
    }
    root.SetVisible(true);

    MeshRendererComponent renderer;
    renderer.boundsMode = RenderBoundsMode::Manual;
    renderer.localBoundsMin = Math::Vec3(-2.0f, -1.0f, -3.0f);
    renderer.localBoundsMax = Math::Vec3(2.0f, 1.0f, 3.0f);
    if (!grandchild.SetMeshRenderer(renderer) || !grandchild.HasMeshRenderer())
    {
        return Fail("mesh renderer component attachment failed");
    }

    LightComponent light;
    light.type = LightType::Point;
    light.color = Math::Vec3(1.0f, 0.5f, 0.25f);
    light.range = 50.0f;
    if (!child.SetLight(light) || !child.HasLight())
    {
        return Fail("light component attachment failed");
    }
    scene.SetPrimaryLight(child);

    const auto& renderObjects = scene.GetRenderObjects();
    const auto& lights = scene.GetLights();
    if (renderObjects.size() != 1 || !renderObjects[0] ||
        renderObjects[0]->entityId != grandchild.GetId() ||
        !renderObjects[0]->hasWorldTransform ||
        !NearlyEqual(renderObjects[0]->GetWorldPosition(), grandchild.GetWorldPosition()) ||
        lights.size() != 1 || lights[0]->entityId != child.GetId() ||
        scene.GetPrimaryLightEntity() != child)
    {
        return Fail("renderer/light proxy synchronization failed");
    }

    if (!grandchild.ClearParent() || grandchild.GetParent() ||
        !NearlyEqual(grandchild.GetWorldPosition(), Math::Vec3(0.0f, 0.0f, 1.0f)))
    {
        return Fail("detachment did not preserve local transform semantics");
    }

    if (!scene.DestroyEntity(root) || root || child || scene.GetEntityCount() != 1 ||
        !grandchild)
    {
        return Fail("recursive destruction or entity invalidation failed");
    }

    scene.Clear();
    if (scene.GetEntityCount() != 0 || scene.GetObjectCount() != 0 ||
        scene.GetLightCount() != 0)
    {
        return Fail("scene clear failed");
    }

    Entity expired;
    {
        Scene temporary("Temporary");
        expired = temporary.CreateEntity("Ephemeral");
        if (!expired)
        {
            return Fail("temporary entity creation failed");
        }
    }
    if (expired || expired.GetScene() != nullptr)
    {
        return Fail("entity facade did not expire with its scene");
    }

    return EXIT_SUCCESS;
}
