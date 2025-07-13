#include <Pyramid/Graphics/Scene/Octree.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Utils/Logger.hpp>
#include <algorithm>
#include <limits>

namespace Pyramid
{
    namespace Scene
    {

        // AABB Implementation
        bool AABB::IntersectsSphere(const Math::Vec3 &center, f32 radius) const
        {
            f32 sqDist = 0.0f;

            // Calculate squared distance from sphere center to AABB
            for (int i = 0; i < 3; ++i)
            {
                f32 v = center[i];
                f32 minVal = min[i];
                f32 maxVal = max[i];

                if (v < minVal)
                    sqDist += (minVal - v) * (minVal - v);
                if (v > maxVal)
                    sqDist += (v - maxVal) * (v - maxVal);
            }

            return sqDist <= radius * radius;
        }

        bool AABB::IntersectsRay(const Math::Vec3 &origin, const Math::Vec3 &direction, f32 &distance) const
        {
            f32 tmin = 0.0f;
            f32 tmax = std::numeric_limits<f32>::max();

            for (int i = 0; i < 3; ++i)
            {
                if (Math::Abs(direction[i]) < 1e-6f)
                {
                    // Ray is parallel to slab
                    if (origin[i] < min[i] || origin[i] > max[i])
                        return false;
                }
                else
                {
                    f32 ood = 1.0f / direction[i];
                    f32 t1 = (min[i] - origin[i]) * ood;
                    f32 t2 = (max[i] - origin[i]) * ood;

                    if (t1 > t2)
                        std::swap(t1, t2);

                    tmin = Math::Max(tmin, t1);
                    tmax = Math::Min(tmax, t2);

                    if (tmin > tmax)
                        return false;
                }
            }

            distance = tmin;
            return true;
        }

        void AABB::Expand(const AABB &other)
        {
            min.x = Math::Min(min.x, other.min.x);
            min.y = Math::Min(min.y, other.min.y);
            min.z = Math::Min(min.z, other.min.z);
            max.x = Math::Max(max.x, other.max.x);
            max.y = Math::Max(max.y, other.max.y);
            max.z = Math::Max(max.z, other.max.z);
        }

        AABB AABB::GetExpanded(f32 amount) const
        {
            Math::Vec3 expansion(amount, amount, amount);
            return AABB(min - expansion, max + expansion);
        }

        // OctreeNode Implementation
        OctreeNode::OctreeNode(const AABB &bounds, u32 depth, u32 maxDepth, u32 maxObjects)
            : m_bounds(bounds), m_depth(depth), m_maxDepth(maxDepth), m_maxObjects(maxObjects), m_needsRebuild(false)
        {
            // Initialize children to nullptr
            for (auto &child : m_children)
            {
                child = nullptr;
            }
        }

        OctreeNode::~OctreeNode() = default;

        void OctreeNode::Insert(std::shared_ptr<RenderObject> object)
        {
            if (!object)
                return;

            // If we're at max depth or under object limit, store here
            if (m_depth >= m_maxDepth || (IsLeaf() && m_objects.size() < m_maxObjects))
            {
                m_objects.push_back(object);
                return;
            }

            // If we're a leaf and over the limit, subdivide
            if (IsLeaf() && m_objects.size() >= m_maxObjects)
            {
                Subdivide();

                // Redistribute existing objects
                auto objectsCopy = m_objects;
                m_objects.clear();

                for (const auto &obj : objectsCopy)
                {
                    Insert(obj);
                }
            }

            // Try to insert into appropriate child
            u32 childIndex = GetChildIndex(object->position);
            if (childIndex < 8 && m_children[childIndex])
            {
                m_children[childIndex]->Insert(object);
            }
            else
            {
                // Fallback: store in this node
                m_objects.push_back(object);
            }
        }

        void OctreeNode::Remove(std::shared_ptr<RenderObject> object)
        {
            // Remove from this node
            auto it = std::find(m_objects.begin(), m_objects.end(), object);
            if (it != m_objects.end())
            {
                m_objects.erase(it);
                return;
            }

            // Try to remove from children
            if (!IsLeaf())
            {
                for (auto &child : m_children)
                {
                    if (child)
                    {
                        child->Remove(object);
                    }
                }
            }
        }

        void OctreeNode::Clear()
        {
            m_objects.clear();
            for (auto &child : m_children)
            {
                child.reset();
            }
        }

        void OctreeNode::Query(const Math::Vec3 &point, std::vector<std::shared_ptr<RenderObject>> &results) const
        {
            if (!m_bounds.Contains(point))
                return;

            // Add objects from this node
            for (const auto &obj : m_objects)
            {
                if (obj)
                {
                    results.push_back(obj);
                }
            }

            // Query children
            if (!IsLeaf())
            {
                for (const auto &child : m_children)
                {
                    if (child)
                    {
                        child->Query(point, results);
                    }
                }
            }
        }

        void OctreeNode::Query(const AABB &bounds, std::vector<std::shared_ptr<RenderObject>> &results) const
        {
            if (!m_bounds.Intersects(bounds))
                return;

            // Add objects from this node
            for (const auto &obj : m_objects)
            {
                if (obj)
                {
                    // Simple point-in-bounds check for now
                    if (bounds.Contains(obj->position))
                    {
                        results.push_back(obj);
                    }
                }
            }

            // Query children
            if (!IsLeaf())
            {
                for (const auto &child : m_children)
                {
                    if (child)
                    {
                        child->Query(bounds, results);
                    }
                }
            }
        }

        void OctreeNode::Query(const Math::Vec3 &center, f32 radius, std::vector<std::shared_ptr<RenderObject>> &results) const
        {
            if (!m_bounds.IntersectsSphere(center, radius))
                return;

            // Add objects from this node
            for (const auto &obj : m_objects)
            {
                if (obj)
                {
                    f32 distance = (obj->position - center).Length();
                    if (distance <= radius)
                    {
                        results.push_back(obj);
                    }
                }
            }

            // Query children
            if (!IsLeaf())
            {
                for (const auto &child : m_children)
                {
                    if (child)
                    {
                        child->Query(center, radius, results);
                    }
                }
            }
        }

        u32 OctreeNode::GetTotalObjectCount() const
        {
            u32 count = static_cast<u32>(m_objects.size());

            if (!IsLeaf())
            {
                for (const auto &child : m_children)
                {
                    if (child)
                    {
                        count += child->GetTotalObjectCount();
                    }
                }
            }

            return count;
        }

        u32 OctreeNode::GetNodeCount() const
        {
            u32 count = 1; // This node

            if (!IsLeaf())
            {
                for (const auto &child : m_children)
                {
                    if (child)
                    {
                        count += child->GetNodeCount();
                    }
                }
            }

            return count;
        }

        void OctreeNode::QueryFrustum(const std::array<Math::Vec4, 6> &frustumPlanes,
                                      std::vector<std::shared_ptr<RenderObject>> &results) const
        {
            // Simple frustum culling - for now just add all objects
            for (const auto &obj : m_objects)
            {
                if (obj)
                {
                    results.push_back(obj);
                }
            }

            // Query children
            if (!IsLeaf())
            {
                for (const auto &child : m_children)
                {
                    if (child)
                    {
                        child->QueryFrustum(frustumPlanes, results);
                    }
                }
            }
        }

        void OctreeNode::Subdivide()
        {
            if (m_depth >= m_maxDepth)
                return;

            Math::Vec3 center = m_bounds.GetCenter();
            Math::Vec3 size = m_bounds.GetSize() * 0.5f;

            // Create 8 child nodes
            for (u32 i = 0; i < 8; ++i)
            {
                AABB childBounds = GetChildBounds(i);
                m_children[i] = std::make_unique<OctreeNode>(childBounds, m_depth + 1, m_maxDepth, m_maxObjects);
            }
        }

        u32 OctreeNode::GetChildIndex(const Math::Vec3 &position) const
        {
            Math::Vec3 center = m_bounds.GetCenter();
            u32 index = 0;

            if (position.x >= center.x)
                index |= 1;
            if (position.y >= center.y)
                index |= 2;
            if (position.z >= center.z)
                index |= 4;

            return index;
        }

        AABB OctreeNode::GetChildBounds(u32 index) const
        {
            Math::Vec3 center = m_bounds.GetCenter();
            Math::Vec3 size = m_bounds.GetSize() * 0.5f;

            Math::Vec3 childMin = center;
            Math::Vec3 childMax = center;

            if (index & 1)
            {
                childMin.x = center.x;
                childMax.x = m_bounds.max.x;
            }
            else
            {
                childMin.x = m_bounds.min.x;
                childMax.x = center.x;
            }

            if (index & 2)
            {
                childMin.y = center.y;
                childMax.y = m_bounds.max.y;
            }
            else
            {
                childMin.y = m_bounds.min.y;
                childMax.y = center.y;
            }

            if (index & 4)
            {
                childMin.z = center.z;
                childMax.z = m_bounds.max.z;
            }
            else
            {
                childMin.z = m_bounds.min.z;
                childMax.z = center.z;
            }

            return AABB(childMin, childMax);
        }

        // Octree Implementation
        Octree::Octree(const Math::Vec3 &center, const Math::Vec3 &size, u32 maxDepth, u32 maxObjectsPerNode)
            : m_maxDepth(maxDepth), m_maxObjectsPerNode(maxObjectsPerNode), m_lastQueryCount(0), m_lastQueryTime(0.0f)
        {
            Math::Vec3 halfSize = size * 0.5f;
            AABB rootBounds(center - halfSize, center + halfSize);
            m_root = std::make_unique<OctreeNode>(rootBounds, 0, maxDepth, maxObjectsPerNode);

            PYRAMID_LOG_INFO("Octree created with max depth: ", maxDepth, ", max objects per node: ", maxObjectsPerNode);
        }

        Octree::~Octree() = default;

        void Octree::Insert(std::shared_ptr<RenderObject> object)
        {
            if (m_root && object)
            {
                m_root->Insert(object);

                // Track object bounds for updates
                AABB objBounds(object->position - Math::Vec3(0.5f), object->position + Math::Vec3(0.5f));
                m_objectBounds[object] = objBounds;
            }
        }

        void Octree::Clear()
        {
            if (m_root)
            {
                m_root->Clear();
            }
            m_objectBounds.clear();
        }

        std::vector<std::shared_ptr<RenderObject>> Octree::QueryPoint(const Math::Vec3 &point) const
        {
            std::vector<std::shared_ptr<RenderObject>> results;
            if (m_root)
            {
                m_root->Query(point, results);
            }
            return results;
        }

        std::vector<std::shared_ptr<RenderObject>> Octree::QuerySphere(const Math::Vec3 &center, f32 radius) const
        {
            std::vector<std::shared_ptr<RenderObject>> results;
            if (m_root)
            {
                m_root->Query(center, radius, results);
            }
            return results;
        }

        std::vector<std::shared_ptr<RenderObject>> Octree::QueryBox(const AABB &bounds) const
        {
            std::vector<std::shared_ptr<RenderObject>> results;
            if (m_root)
            {
                m_root->Query(bounds, results);
            }
            return results;
        }

        std::vector<std::shared_ptr<RenderObject>> Octree::QueryRay(const Math::Vec3 &origin,
                                                                    const Math::Vec3 &direction,
                                                                    f32 maxDistance) const
        {
            std::vector<std::shared_ptr<RenderObject>> results;
            // Simple ray query implementation
            if (m_root)
            {
                // For now, use sphere query along the ray
                Math::Vec3 endPoint = origin + direction * maxDistance;
                Math::Vec3 center = (origin + endPoint) * 0.5f;
                f32 radius = maxDistance * 0.5f;
                m_root->Query(center, radius, results);
            }
            return results;
        }

        std::vector<std::shared_ptr<RenderObject>> Octree::QueryFrustum(const std::array<Math::Vec4, 6> &frustumPlanes) const
        {
            std::vector<std::shared_ptr<RenderObject>> results;
            if (m_root)
            {
                m_root->QueryFrustum(frustumPlanes, results);
            }
            return results;
        }

        std::shared_ptr<RenderObject> Octree::FindNearest(const Math::Vec3 &position) const
        {
            std::shared_ptr<RenderObject> nearest = nullptr;
            f32 nearestDistance = std::numeric_limits<f32>::max();

            if (m_root)
            {
                FindNearestRecursive(position, nearest, nearestDistance, m_root.get());
            }

            return nearest;
        }

        void Octree::Rebuild()
        {
            if (m_root)
            {
                // Simple rebuild - clear and re-insert all objects
                std::vector<std::shared_ptr<RenderObject>> allObjects;

                // Collect all objects
                for (const auto &pair : m_objectBounds)
                {
                    allObjects.push_back(pair.first);
                }

                // Clear and re-insert
                Clear();
                for (const auto &obj : allObjects)
                {
                    Insert(obj);
                }
            }
        }

        Octree::OctreeStats Octree::GetStats() const
        {
            OctreeStats stats;
            if (m_root)
            {
                stats.totalNodes = m_root->GetNodeCount();
                stats.totalObjects = m_root->GetTotalObjectCount();
                stats.maxDepth = m_maxDepth;
            }
            return stats;
        }

        const AABB &Octree::GetBounds() const
        {
            return m_root->GetBounds();
        }

        void Octree::FindNearestRecursive(const Math::Vec3 &position,
                                          std::shared_ptr<RenderObject> &nearest,
                                          f32 &nearestDistance,
                                          const OctreeNode *node) const
        {
            if (!node)
                return;

            // Check objects in this node
            for (const auto &obj : node->m_objects)
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

            // Recursively check children
            if (!node->IsLeaf())
            {
                for (const auto &child : node->m_children)
                {
                    if (child)
                    {
                        FindNearestRecursive(position, nearest, nearestDistance, child.get());
                    }
                }
            }
        }

        // Spatial utility functions
        namespace SpatialUtils
        {
            AABB CalculateAABB(const std::shared_ptr<RenderObject> &object)
            {
                if (!object)
                    return AABB();

                // Simple AABB calculation - in real implementation this would consider mesh bounds
                Math::Vec3 halfSize(0.5f, 0.5f, 0.5f);
                return AABB(object->position - halfSize, object->position + halfSize);
            }

            std::array<Math::Vec4, 6> CalculateFrustumPlanes(const Camera &camera)
            {
                // Simplified frustum plane calculation
                // In real implementation, this would extract planes from view-projection matrix
                std::array<Math::Vec4, 6> planes;

                // For now, return dummy planes
                for (auto &plane : planes)
                {
                    plane = Math::Vec4(0, 1, 0, -1000); // Ground plane far below
                }

                return planes;
            }

            bool AABBInFrustum(const AABB &aabb, const std::array<Math::Vec4, 6> &frustumPlanes)
            {
                // Simplified frustum culling - always return true for now
                return true;
            }

            f32 DistanceToAABB(const Math::Vec3 &point, const AABB &aabb)
            {
                f32 sqDist = 0.0f;

                for (int i = 0; i < 3; ++i)
                {
                    f32 v = point[i];
                    if (v < aabb.min[i])
                        sqDist += (aabb.min[i] - v) * (aabb.min[i] - v);
                    if (v > aabb.max[i])
                        sqDist += (v - aabb.max[i]) * (v - aabb.max[i]);
                }

                return Math::Sqrt(sqDist);
            }
        }

    } // namespace Scene
} // namespace Pyramid
