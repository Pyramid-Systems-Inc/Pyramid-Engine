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

    int TestEntityRecreation(bool clearScene)
    {
        using namespace Pyramid;

        Scene scene("Entity recreation");
        Entity original = scene.CreateEntity("Original");
        Entity originalChild = scene.CreateEntity("Original child");
        if (!originalChild.SetParent(original))
        {
            return Fail("entity recreation hierarchy setup failed");
        }
        const Entity originalCopy = original;
        const auto listed = scene.GetEntities();
        const auto roots = scene.GetRootEntities();
        const auto children = original.GetChildren();
        const Entity found = scene.FindEntity(original.GetId());
        const Entity parentFacade = originalChild.GetParent();
        const Scene& constScene = scene;
        const auto constListed = constScene.GetEntities();
        const auto constRoots = constScene.GetRootEntities();
        const Entity constFound = constScene.FindEntity(original.GetId());
        if (listed.size() != 2 || constListed.size() != 2 || roots.size() != 1 ||
            constRoots.size() != 1 || children.size() != 1)
        {
            return Fail("entity recreation enumeration returned unexpected counts");
        }
        if (found != original || parentFacade != original || constFound != original ||
            scene.FindEntity("Original") != original || constScene.FindEntity("Original") != original ||
            listed[0] != original || constListed[0] != original ||
            roots[0] != original || constRoots[0] != original || children[0] != originalChild)
        {
            return Fail("entity lookup and enumeration returned different facade identities");
        }

        if (clearScene)
        {
            scene.Clear();
        }
        else if (!scene.DestroyEntity(original))
        {
            return Fail("entity recreation destruction failed");
        }

        Entity replacement = clearScene ? scene.CreateEntity("Replacement")
            : scene.CreateEntityWithId(original.GetId(), "Replacement");
        Entity replacementChild = scene.CreateEntityWithId(originalChild.GetId(), "Replacement child");
        if (!replacement || !replacementChild || replacement.GetId() != original.GetId() ||
            !replacementChild.SetParent(replacement))
        {
            return Fail("entity recreation did not reuse the original IDs");
        }
        if (original || originalCopy || originalChild || found || listed[0] || listed[1] ||
            roots[0] || children[0] || parentFacade || constFound ||
            constListed[0] || constListed[1] || constRoots[0] || original.GetScene() != nullptr)
        {
            return Fail(clearScene ? "scene clear revived retained entity facades"
                : "explicit ID recreation revived destroyed entity facades");
        }
        if (original != originalCopy || original == replacement ||
            replacement != scene.FindEntity(replacement.GetId()) || Entity{} != Entity{})
        {
            return Fail("entity equality did not distinguish entity lifetimes");
        }

        MeshRendererComponent renderer;
        LightComponent light;
        if (!replacement.SetMeshRenderer(renderer) || !replacement.SetLight(light))
        {
            return Fail("replacement component setup failed");
        }
        if (!original.GetName().empty() || original.GetTransform() ||
            original.GetMeshRenderer() || original.GetLight() || original.GetParent() ||
            !original.GetChildren().empty() || original.IsVisible() ||
            original.IsEffectivelyVisible() || original.HasMeshRenderer() || original.HasLight())
        {
            return Fail("stale facade accessed replacement state");
        }
        if (original.SetName("Stale") || original.SetVisible(false) ||
            original.SetLocalPosition(Math::Vec3(99.0f)) ||
            original.SetMeshRenderer(renderer) || original.RemoveMeshRenderer() ||
            original.SetLight(light) || original.RemoveLight() ||
            original.SetParent(replacementChild) || original.ClearParent() ||
            scene.DestroyEntity(original))
        {
            return Fail("stale facade mutated or destroyed replacement state");
        }
        if (replacementChild.SetParent(original) || replacementChild.GetParent() != replacement)
        {
            return Fail("stale parent changed the replacement hierarchy");
        }
        scene.SetPrimaryLight(replacement);
        scene.SetPrimaryLight(original);
        if (scene.GetPrimaryLightEntity() != replacement)
        {
            return Fail("stale primary-light facade changed the replacement primary");
        }

        Scene foreignScene("Foreign scene");
        Entity foreign = foreignScene.CreateEntity("Foreign");
        foreign.SetLight(light);
        scene.SetPrimaryLight(foreign);
        if (scene.DestroyEntity(foreign) || replacementChild.SetParent(foreign) ||
            replacementChild.GetParent() != replacement ||
            scene.GetPrimaryLightEntity() != replacement)
        {
            return Fail("foreign facade changed the current scene");
        }
        if (!replacementChild.SetParent(Entity{}) || replacementChild.GetParent())
        {
            return Fail("default entity no longer detaches a parent");
        }
        scene.SetPrimaryLight(Entity{});
        if (scene.GetPrimaryLightEntity() || replacement.GetName() != "Replacement" ||
            !replacement.IsVisible() || !replacement.HasMeshRenderer() || !replacement.HasLight() ||
            !NearlyEqual(replacement.GetWorldPosition(), Math::Vec3::Zero))
        {
            return Fail("default primary-light clearing or replacement state was corrupted");
        }

        scene.Clear();
        scene.Clear();
        Entity final = scene.CreateEntityWithId(original.GetId(), "Final");
        if (!final || original || replacement || replacementChild || final == replacement)
        {
            return Fail("repeated scene clearing revived a previous entity lifetime");
        }
        return EXIT_SUCCESS;
    }

    int TestChildFacadeLifetime()
    {
        using namespace Pyramid;

        Scene scene("Child lifetime");
        Entity parent = scene.CreateEntity("Parent");
        Entity child = scene.CreateEntity("Child");
        Entity newParent = scene.CreateEntity("New parent");
        if (!child.SetParent(parent))
        {
            return Fail("child lifetime hierarchy setup failed");
        }
        const auto children = parent.GetChildren();
        if (children.size() != 1)
        {
            return Fail("child lifetime enumeration returned an unexpected count");
        }
        const Entity retainedChild = children[0];
        if (!child.SetParent(newParent) || !scene.DestroyEntity(parent) ||
            !retainedChild || retainedChild != child ||
            retainedChild.GetParent() != newParent || retainedChild != scene.FindEntity(child.GetId()))
        {
            return Fail("GetChildren facade expired with its former parent");
        }
        if (!scene.DestroyEntity(child))
        {
            return Fail("child lifetime destruction failed");
        }
        Entity replacement = scene.CreateEntityWithId(child.GetId(), "Replacement child");
        if (!newParent || !replacement || child || retainedChild ||
            retainedChild.GetScene() != nullptr || retainedChild == replacement)
        {
            return Fail("GetChildren facade did not expire with its child record");
        }
        return EXIT_SUCCESS;
    }
}

int main()
{
    using namespace Pyramid;

    if (TestEntityRecreation(true) != EXIT_SUCCESS ||
        TestEntityRecreation(false) != EXIT_SUCCESS ||
        TestChildFacadeLifetime() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

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
