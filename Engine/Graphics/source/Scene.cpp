#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <algorithm>

namespace Pyramid
{

    // RenderObject Implementation
    Math::Mat4 RenderObject::GetTransformMatrix() const
    {
        Math::Mat4 scaleMatrix = Math::Mat4::CreateScale(scale);
        Math::Mat4 rotationMatrix = rotation.ToMatrix4();
        Math::Mat4 translationMatrix = Math::Mat4::CreateTranslation(position);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    // SceneNode Implementation
    SceneNode::SceneNode(const std::string &name)
        : m_name(name)
    {
    }

    void SceneNode::AddChild(std::shared_ptr<SceneNode> child)
    {
        if (!child || child.get() == this)
            return;

        // Remove from previous parent
        if (auto oldParent = child->m_parent.lock())
        {
            oldParent->RemoveChild(child);
        }

        child->m_parent = shared_from_this();
        m_children.push_back(child);
        child->m_worldTransformDirty = true;
    }

    void SceneNode::RemoveChild(std::shared_ptr<SceneNode> child)
    {
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end())
        {
            (*it)->m_parent.reset();
            m_children.erase(it);
        }
    }

    void SceneNode::SetParent(std::shared_ptr<SceneNode> parent)
    {
        if (parent)
        {
            parent->AddChild(shared_from_this());
        }
        else if (auto oldParent = m_parent.lock())
        {
            oldParent->RemoveChild(shared_from_this());
        }
    }

    void SceneNode::SetLocalTransform(const Math::Vec3 &position, const Math::Quat &rotation, const Math::Vec3 &scale)
    {
        m_localPosition = position;
        m_localRotation = rotation;
        m_localScale = scale;
        m_transformDirty = true;
        m_worldTransformDirty = true;
    }

    Math::Vec3 SceneNode::GetWorldPosition() const
    {
        const Math::Mat4 &worldTransform = GetWorldTransform();
        return Math::Vec3(worldTransform.m[12], worldTransform.m[13], worldTransform.m[14]);
    }

    Math::Quat SceneNode::GetWorldRotation() const
    {
        if (auto parent = m_parent.lock())
        {
            return parent->GetWorldRotation() * m_localRotation;
        }
        return m_localRotation;
    }

    Math::Vec3 SceneNode::GetWorldScale() const
    {
        if (auto parent = m_parent.lock())
        {
            Math::Vec3 parentScale = parent->GetWorldScale();
            return Math::Vec3(parentScale.x * m_localScale.x,
                              parentScale.y * m_localScale.y,
                              parentScale.z * m_localScale.z);
        }
        return m_localScale;
    }

    const Math::Mat4 &SceneNode::GetLocalTransform() const
    {
        if (m_transformDirty)
        {
            UpdateLocalTransform();
        }
        return m_localTransform;
    }

    const Math::Mat4 &SceneNode::GetWorldTransform() const
    {
        if (m_worldTransformDirty)
        {
            UpdateWorldTransform();
        }
        return m_worldTransform;
    }

    void SceneNode::UpdateLocalTransform() const
    {
        Math::Mat4 scaleMatrix = Math::Mat4::CreateScale(m_localScale);
        Math::Mat4 rotationMatrix = m_localRotation.ToMatrix4();
        Math::Mat4 translationMatrix = Math::Mat4::CreateTranslation(m_localPosition);

        m_localTransform = translationMatrix * rotationMatrix * scaleMatrix;
        m_transformDirty = false;
    }

    void SceneNode::UpdateWorldTransform() const
    {
        if (auto parent = m_parent.lock())
        {
            m_worldTransform = parent->GetWorldTransform() * GetLocalTransform();
        }
        else
        {
            m_worldTransform = GetLocalTransform();
        }
        m_worldTransformDirty = false;
    }

    // Scene Implementation
    Scene::Scene(const std::string &name)
        : m_name(name)
    {
        m_rootNode = std::make_shared<SceneNode>("Root");
    }

    std::shared_ptr<SceneNode> Scene::CreateNode(const std::string &name)
    {
        auto node = std::make_shared<SceneNode>(name);
        m_rootNode->AddChild(node);
        return node;
    }

    void Scene::RemoveNode(std::shared_ptr<SceneNode> node)
    {
        if (node && node != m_rootNode)
        {
            if (auto parent = node->m_parent.lock())
            {
                parent->RemoveChild(node);
            }
        }
    }

    std::shared_ptr<SceneNode> Scene::FindNode(const std::string &name) const
    {
        // Simple recursive search - could be optimized with a hash map
        std::function<std::shared_ptr<SceneNode>(std::shared_ptr<SceneNode>)> search;
        search = [&](std::shared_ptr<SceneNode> node) -> std::shared_ptr<SceneNode>
        {
            if (node->GetName() == name)
            {
                return node;
            }
            for (auto &child : node->m_children)
            {
                if (auto found = search(child))
                {
                    return found;
                }
            }
            return nullptr;
        };

        return search(m_rootNode);
    }

    void Scene::AddRenderObject(std::shared_ptr<RenderObject> object)
    {
        if (object)
        {
            m_renderObjects.push_back(object);
        }
    }

    void Scene::RemoveRenderObject(std::shared_ptr<RenderObject> object)
    {
        auto it = std::find(m_renderObjects.begin(), m_renderObjects.end(), object);
        if (it != m_renderObjects.end())
        {
            m_renderObjects.erase(it);
        }
    }

    void Scene::AddLight(std::shared_ptr<Light> light)
    {
        if (light)
        {
            m_lights.push_back(light);
        }
    }

    void Scene::RemoveLight(std::shared_ptr<Light> light)
    {
        auto it = std::find(m_lights.begin(), m_lights.end(), light);
        if (it != m_lights.end())
        {
            m_lights.erase(it);
        }
    }

    std::vector<std::shared_ptr<RenderObject>> Scene::GetVisibleObjects(const Camera &camera) const
    {
        std::vector<std::shared_ptr<RenderObject>> visibleObjects;

        for (const auto &object : m_renderObjects)
        {
            if (object && object->visible)
            {
                // Simple visibility test - could be enhanced with proper frustum culling
                if (camera.IsPointVisible(object->position))
                {
                    visibleObjects.push_back(object);
                }
            }
        }

        return visibleObjects;
    }

    std::vector<std::shared_ptr<Light>> Scene::GetVisibleLights(const Camera &camera) const
    {
        std::vector<std::shared_ptr<Light>> visibleLights;

        for (const auto &light : m_lights)
        {
            if (light && light->enabled)
            {
                // For now, include all lights - could add light culling later
                visibleLights.push_back(light);
            }
        }

        return visibleLights;
    }

    void Scene::Clear()
    {
        m_renderObjects.clear();
        m_lights.clear();
        m_primaryLight.reset();
        m_rootNode = std::make_shared<SceneNode>("Root");
    }

    // SceneUtils Implementation
    namespace SceneUtils
    {
        std::shared_ptr<Scene> CreateTestScene()
        {
            auto scene = std::make_shared<Scene>("Test Scene");

            // Create a simple directional light
            auto light = CreateDirectionalLight(Math::Vec3(0.5f, -1.0f, 0.5f));
            scene->AddLight(light);
            scene->SetPrimaryLight(light);

            // Set up basic environment
            auto &env = scene->GetEnvironment();
            env.skyColor = Math::Vec3(0.5f, 0.7f, 1.0f);
            env.ambientColor = Math::Vec3(0.1f, 0.1f, 0.1f);

            return scene;
        }

        std::shared_ptr<RenderObject> CreateCube(f32 size)
        {
            auto object = std::make_shared<RenderObject>();
            object->name = "Cube";
            object->scale = Math::Vec3(size, size, size);

            // TODO: Create actual cube geometry when vertex buffer system is ready
            // For now, this is a placeholder

            return object;
        }

        std::shared_ptr<RenderObject> CreateSphere(f32 radius, u32 segments)
        {
            auto object = std::make_shared<RenderObject>();
            object->name = "Sphere";
            object->scale = Math::Vec3(radius, radius, radius);

            // TODO: Create actual sphere geometry

            return object;
        }

        std::shared_ptr<RenderObject> CreatePlane(f32 width, f32 height)
        {
            auto object = std::make_shared<RenderObject>();
            object->name = "Plane";
            object->scale = Math::Vec3(width, 1.0f, height);

            // TODO: Create actual plane geometry

            return object;
        }

        std::shared_ptr<Light> CreateDirectionalLight(const Math::Vec3 &direction,
                                                      const Math::Vec3 &color,
                                                      f32 intensity)
        {
            auto light = std::make_shared<Light>();
            light->type = LightType::Directional;
            light->direction = direction.Normalized();
            light->color = color;
            light->intensity = intensity;
            light->name = "Directional Light";

            return light;
        }
    }

} // namespace Pyramid
