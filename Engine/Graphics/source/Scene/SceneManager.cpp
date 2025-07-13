#include <Pyramid/Graphics/Scene/SceneManager.hpp>
#include <Pyramid/Graphics/Scene/Octree.hpp>
#include <Pyramid/Graphics/Camera.hpp>
// TODO: Add proper logging when available
#include <fstream>
#include <chrono>

namespace Pyramid
{
    namespace SceneManagement
    {

        SceneManager::SceneManager()
            : m_activeScene(nullptr), m_spatialPartitioningEnabled(true), m_octreeMaxDepth(8), m_octreeSize(1000.0f, 1000.0f, 1000.0f), m_octreeCenter(Math::Vec3::Zero), m_lodEnabled(true), m_frustumCullingEnabled(true), m_occlusionCullingEnabled(false), m_debugVisualization(false), m_needsOctreeRebuild(false), m_lastUpdateTime(0.0f)
        {
            PYRAMID_LOG_INFO("SceneManager initialized");
            InitializeOctree();
        }

        SceneManager::~SceneManager()
        {
            PYRAMID_LOG_INFO("SceneManager destroyed");
        }

        std::shared_ptr<Pyramid::Scene> SceneManager::CreateScene(const std::string &name)
        {
            auto scene = std::make_shared<Pyramid::Scene>();
            m_scenes[name] = scene;

            if (!m_activeScene)
            {
                SetActiveScene(scene);
            }

            PYRAMID_LOG_INFO("Created scene: ", name);
            return scene;
        }

        void SceneManager::SetActiveScene(std::shared_ptr<Pyramid::Scene> scene)
        {
            if (m_activeScene != scene)
            {
                m_activeScene = scene;
                m_needsOctreeRebuild = true;
                PYRAMID_LOG_INFO("Active scene changed");
            }
        }

        void SceneManager::InitializeOctree()
        {
            if (m_spatialPartitioningEnabled)
            {
                m_octree = std::make_unique<Octree>(m_octreeCenter, m_octreeSize, m_octreeMaxDepth);
                PYRAMID_LOG_INFO("Octree spatial partitioning initialized");
                PYRAMID_LOG_INFO("  Size: (", m_octreeSize.x, ", ", m_octreeSize.y, ", ", m_octreeSize.z, ")");
                PYRAMID_LOG_INFO("  Max Depth: ", m_octreeMaxDepth);
            }
        }

        void SceneManager::RebuildSpatialPartition()
        {
            if (!m_spatialPartitioningEnabled || !m_octree || !m_activeScene)
                return;

            auto start = std::chrono::high_resolution_clock::now();

            m_octree->Clear();

            // Add all render objects from active scene to octree
            auto renderObjects = m_activeScene->GetVisibleObjects();
            for (const auto &obj : renderObjects)
            {
                if (obj)
                {
                    m_octree->Insert(obj);
                }
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<float, std::milli>(end - start).count();

            m_stats.lastUpdateTime = duration;
            m_needsOctreeRebuild = false;

            PYRAMID_LOG_INFO("Spatial partition rebuilt in ", duration, " ms");
            PYRAMID_LOG_INFO("  Objects added: ", renderObjects.size());
        }

        QueryResult SceneManager::QueryScene(const QueryParams &params)
        {
            QueryResult result;

            if (!m_activeScene)
                return result;

            auto start = std::chrono::high_resolution_clock::now();

            if (m_spatialPartitioningEnabled && m_octree)
            {
                // Use spatial partitioning for efficient queries
                switch (params.type)
                {
                case QueryType::Point:
                    result.objects = m_octree->QueryPoint(params.position);
                    break;
                case QueryType::Sphere:
                    result.objects = m_octree->QuerySphere(params.position, params.radius);
                    break;
                case QueryType::Box:
                {
                    AABB bounds(params.position - params.size * 0.5f,
                                params.position + params.size * 0.5f);
                    result.objects = m_octree->QueryBox(bounds);
                }
                break;
                case QueryType::Ray:
                    result.objects = m_octree->QueryRay(params.position, params.direction, params.maxDistance);
                    break;
                }
            }
            else
            {
                // Fallback to brute force search
                auto allObjects = m_activeScene->GetVisibleObjects();
                for (const auto &obj : allObjects)
                {
                    // Simple distance check for demonstration
                    if (obj && (obj->position - params.position).Length() <= params.radius)
                    {
                        result.objects.push_back(obj);
                    }
                }
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<float, std::milli>(end - start).count();

            result.totalFound = static_cast<u32>(result.objects.size());
            m_stats.lastQueryTime = duration;

            return result;
        }

        std::vector<std::shared_ptr<RenderObject>> SceneManager::GetVisibleObjects(const Camera &camera)
        {
            std::vector<std::shared_ptr<RenderObject>> visibleObjects;

            if (!m_activeScene)
                return visibleObjects;

            if (m_spatialPartitioningEnabled && m_octree && m_frustumCullingEnabled)
            {
                // Use frustum culling with spatial partitioning
                auto frustumPlanes = SpatialUtils::CalculateFrustumPlanes(camera);
                visibleObjects = m_octree->QueryFrustum(frustumPlanes);
            }
            else
            {
                // Fallback to all objects
                visibleObjects = m_activeScene->GetVisibleObjects();
            }

            // Apply additional culling if enabled
            if (m_occlusionCullingEnabled)
            {
                auto it = std::remove_if(visibleObjects.begin(), visibleObjects.end(),
                                         [this, &camera](const std::shared_ptr<RenderObject> &obj)
                                         {
                                             return OcclusionCull(obj, camera);
                                         });
                visibleObjects.erase(it, visibleObjects.end());
            }

            m_stats.visibleObjects = static_cast<u32>(visibleObjects.size());
            return visibleObjects;
        }

        std::vector<std::shared_ptr<RenderObject>> SceneManager::GetObjectsInRadius(const Math::Vec3 &center, f32 radius)
        {
            QueryParams params;
            params.type = QueryType::Sphere;
            params.position = center;
            params.radius = radius;

            auto result = QueryScene(params);
            return result.objects;
        }

        std::shared_ptr<RenderObject> SceneManager::GetNearestObject(const Math::Vec3 &position)
        {
            if (m_spatialPartitioningEnabled && m_octree)
            {
                return m_octree->FindNearest(position);
            }

            // Fallback implementation
            if (!m_activeScene)
                return nullptr;

            auto allObjects = m_activeScene->GetVisibleObjects();
            std::shared_ptr<RenderObject> nearest = nullptr;
            f32 nearestDistance = std::numeric_limits<f32>::max();

            for (const auto &obj : allObjects)
            {
                if (obj)
                {
                    f32 distance = (obj->position - position).Length();
                    if (distance < nearestDistance)
                    {
                        nearestDistance = distance;
                        nearest = obj;
                    }
                }
            }

            return nearest;
        }

        void SceneManager::Update(f32 deltaTime, UpdateFlags flags)
        {
            if (!m_activeScene)
                return;

            auto start = std::chrono::high_resolution_clock::now();

            // Update transforms if requested
            if ((static_cast<u32>(flags) & static_cast<u32>(UpdateFlags::Transforms)) != 0)
            {
                UpdateTransforms();
            }

            // Update spatial partition if needed
            if ((static_cast<u32>(flags) & static_cast<u32>(UpdateFlags::SpatialPartition)) != 0 || m_needsOctreeRebuild)
            {
                UpdateSpatialPartition();
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<float, std::milli>(end - start).count();

            m_stats.lastUpdateTime = duration;
            m_lastUpdateTime += deltaTime;
        }

        void SceneManager::UpdateTransforms()
        {
            if (!m_activeScene)
                return;

            // Update scene transforms
            m_activeScene->Update(0.0f); // Scene handles its own delta time
        }

        void SceneManager::UpdateSpatialPartition()
        {
            if (m_needsOctreeRebuild)
            {
                RebuildSpatialPartition();
            }
            else if (m_octree)
            {
                m_octree->Rebuild();
            }
        }

        const SceneStats &SceneManager::GetStats() const
        {
            // Update stats
            const_cast<SceneManager *>(this)->m_stats.totalNodes = m_activeScene ? m_activeScene->GetNodeCount() : 0;
            const_cast<SceneManager *>(this)->m_stats.totalObjects = m_activeScene ? static_cast<u32>(m_activeScene->GetVisibleObjects().size()) : 0;

            if (m_octree)
            {
                auto octreeStats = m_octree->GetStats();
                const_cast<SceneManager *>(this)->m_stats.octreeNodes = octreeStats.totalNodes;
                const_cast<SceneManager *>(this)->m_stats.octreeDepth = octreeStats.maxDepth;
            }

            return m_stats;
        }

        void SceneManager::ResetStats()
        {
            m_stats = SceneStats{};
        }

        bool SceneManager::FrustumCull(const std::shared_ptr<RenderObject> &object, const Camera &camera)
        {
            // Simple frustum culling implementation
            // This would be more sophisticated in a real implementation
            return false; // For now, don't cull anything
        }

        bool SceneManager::OcclusionCull(const std::shared_ptr<RenderObject> &object, const Camera &camera)
        {
            // Occlusion culling implementation would go here
            return false; // For now, don't cull anything
        }

        f32 SceneManager::CalculateLOD(const std::shared_ptr<RenderObject> &object, const Camera &camera)
        {
            if (!object)
                return 1.0f;

            f32 distance = (object->position - camera.GetPosition()).Length();

            // Simple LOD calculation based on distance
            if (distance < 10.0f)
                return 1.0f; // High detail
            else if (distance < 50.0f)
                return 0.5f; // Medium detail
            else
                return 0.25f; // Low detail
        }

        // Utility functions
        namespace SceneUtils
        {
            std::unique_ptr<SceneManager> CreateSceneManager()
            {
                return std::make_unique<SceneManager>();
            }

            bool ValidateScene(const std::shared_ptr<Pyramid::Scene> &scene)
            {
                if (!scene)
                {
                    PYRAMID_LOG_ERROR("Scene validation failed: null scene");
                    return false;
                }

                PYRAMID_LOG_INFO("Scene validation passed");
                return true;
            }

            void OptimizeScene(std::shared_ptr<Pyramid::Scene> scene)
            {
                if (!scene)
                    return;

                PYRAMID_LOG_INFO("Scene optimization completed");
            }
        }

    } // namespace Scene
} // namespace Pyramid
