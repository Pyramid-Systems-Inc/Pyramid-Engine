# Scene Management Core Architecture API Documentation

## Overview

The Scene Management Core Architecture provides a comprehensive system for managing 3D scenes with advanced spatial partitioning capabilities. The system is designed for high-performance games requiring efficient object queries and visibility culling.

## Namespace

All Scene Management components are located in the `Pyramid::SceneManagement` namespace to avoid conflicts with the existing `Pyramid::Scene` class.

```cpp
#include <Pyramid/Graphics/Scene/SceneManager.hpp>
#include <Pyramid/Graphics/Scene/Octree.hpp>

using namespace Pyramid::SceneManagement;
```

## Core Classes

### SceneManager

The `SceneManager` class is the primary interface for scene lifecycle management and spatial queries.

#### Constructor

```cpp
SceneManager();
```

Creates a new scene manager with default settings:
- Spatial partitioning enabled
- Octree max depth: 8 levels
- Octree size: 1000x1000x1000 units
- LOD and frustum culling enabled

#### Scene Management

```cpp
// Create a new scene
std::shared_ptr<Pyramid::Scene> CreateScene(const std::string& name);

// Set the active scene for queries and updates
void SetActiveScene(std::shared_ptr<Pyramid::Scene> scene);

// Get the currently active scene
std::shared_ptr<Pyramid::Scene> GetActiveScene() const;
```

#### Spatial Partitioning Configuration

```cpp
// Enable or disable spatial partitioning
void EnableSpatialPartitioning(bool enable);

// Check if spatial partitioning is enabled
bool IsSpatialPartitioningEnabled() const;

// Configure octree parameters
void SetOctreeDepth(u32 depth);           // Default: 8
void SetOctreeSize(const Math::Vec3& size); // Default: (1000, 1000, 1000)

// Force rebuild of spatial partition
void RebuildSpatialPartition();
```

#### Spatial Queries

```cpp
// Generic query interface
QueryResult QueryScene(const QueryParams& params);

// Convenience methods for common queries
std::vector<std::shared_ptr<RenderObject>> GetVisibleObjects(const Camera& camera);
std::vector<std::shared_ptr<RenderObject>> GetObjectsInRadius(const Math::Vec3& center, f32 radius);
std::vector<std::shared_ptr<RenderObject>> GetObjectsInBox(const Math::Vec3& min, const Math::Vec3& max);
std::shared_ptr<RenderObject> GetNearestObject(const Math::Vec3& position);
```

#### Performance Monitoring

```cpp
// Get real-time performance statistics
const SceneStats& GetStats() const;

// Reset performance counters
void ResetStats();
```

#### Update System

```cpp
// Update scene with specific flags
void Update(f32 deltaTime, UpdateFlags flags = UpdateFlags::All);

// Individual update methods
void UpdateTransforms();
void UpdateVisibility(const Camera& camera);
void UpdateSpatialPartition();
```

### Octree

The `Octree` class provides hierarchical spatial partitioning for efficient object queries.

#### Constructor

```cpp
Octree(const Math::Vec3& center, const Math::Vec3& size, u32 maxDepth = 8, u32 maxObjectsPerNode = 10);
```

#### Object Management

```cpp
// Insert object into octree
void Insert(std::shared_ptr<RenderObject> object);

// Remove object from octree
void Remove(std::shared_ptr<RenderObject> object);

// Update object position (remove and re-insert)
void Update(std::shared_ptr<RenderObject> object);

// Clear all objects
void Clear();

// Rebuild entire octree structure
void Rebuild();
```

#### Spatial Queries

```cpp
// Point-based queries
std::vector<std::shared_ptr<RenderObject>> QueryPoint(const Math::Vec3& point) const;

// Sphere-based queries
std::vector<std::shared_ptr<RenderObject>> QuerySphere(const Math::Vec3& center, f32 radius) const;

// Box-based queries
std::vector<std::shared_ptr<RenderObject>> QueryBox(const AABB& bounds) const;

// Ray-based queries
std::vector<std::shared_ptr<RenderObject>> QueryRay(const Math::Vec3& origin, 
                                                   const Math::Vec3& direction, 
                                                   f32 maxDistance = 1000.0f) const;

// Frustum culling
std::vector<std::shared_ptr<RenderObject>> QueryFrustum(const std::array<Math::Vec4, 6>& frustumPlanes) const;

// Nearest neighbor queries
std::shared_ptr<RenderObject> FindNearest(const Math::Vec3& position) const;
std::vector<std::shared_ptr<RenderObject>> FindKNearest(const Math::Vec3& position, u32 k) const;
```

#### Configuration

```cpp
// Runtime configuration
void SetMaxDepth(u32 maxDepth);
void SetMaxObjectsPerNode(u32 maxObjects);
void SetBounds(const Math::Vec3& center, const Math::Vec3& size);

// Accessors
const AABB& GetBounds() const;
u32 GetMaxDepth() const;
u32 GetMaxObjectsPerNode() const;
```

#### Statistics

```cpp
struct OctreeStats {
    u32 totalNodes;
    u32 leafNodes;
    u32 totalObjects;
    u32 maxDepth;
    u32 averageObjectsPerLeaf;
    f32 averageDepth;
    f32 memoryUsage; // In MB
};

OctreeStats GetStats() const;
```

### AABB (Axis-Aligned Bounding Box)

The `AABB` struct provides spatial bounds representation and intersection testing.

#### Constructor

```cpp
AABB();
AABB(const Math::Vec3& minPoint, const Math::Vec3& maxPoint);
```

#### Properties

```cpp
Math::Vec3 min;  // Minimum corner
Math::Vec3 max;  // Maximum corner
```

#### Utility Methods

```cpp
// Basic properties
Math::Vec3 GetCenter() const;
Math::Vec3 GetSize() const;
f32 GetVolume() const;

// Intersection testing
bool Contains(const Math::Vec3& point) const;
bool Intersects(const AABB& other) const;
bool IntersectsSphere(const Math::Vec3& center, f32 radius) const;
bool IntersectsRay(const Math::Vec3& origin, const Math::Vec3& direction, f32& distance) const;

// Expansion operations
void Expand(const Math::Vec3& point);
void Expand(const AABB& other);
AABB GetExpanded(f32 amount) const;
```

## Data Structures

### QueryParams

```cpp
struct QueryParams {
    QueryType type = QueryType::Point;
    Math::Vec3 position = Math::Vec3::Zero;
    Math::Vec3 direction = Math::Vec3::Forward;
    Math::Vec3 size = Math::Vec3::One;
    f32 radius = 1.0f;
    f32 maxDistance = 1000.0f;
};
```

### QueryResult

```cpp
struct QueryResult {
    std::vector<std::shared_ptr<RenderObject>> objects;
    std::vector<std::shared_ptr<SceneNode>> nodes;
    std::vector<f32> distances;
    u32 totalChecked = 0;
    u32 totalFound = 0;
};
```

### SceneStats

```cpp
struct SceneStats {
    u32 totalNodes = 0;
    u32 totalObjects = 0;
    u32 visibleObjects = 0;
    u32 culledObjects = 0;
    u32 octreeNodes = 0;
    u32 octreeDepth = 0;
    f32 lastQueryTime = 0.0f;
    f32 lastUpdateTime = 0.0f;
};
```

## Enumerations

### QueryType

```cpp
enum class QueryType {
    Point,    // Point-in-space queries
    Sphere,   // Sphere intersection queries
    Box,      // Axis-aligned bounding box queries
    Frustum,  // Camera frustum queries
    Ray       // Ray intersection queries
};
```

### UpdateFlags

```cpp
enum class UpdateFlags : u32 {
    None = 0,
    Transforms = 1 << 0,
    Visibility = 1 << 1,
    Lighting = 1 << 2,
    Materials = 1 << 3,
    SpatialPartition = 1 << 4,
    All = 0xFFFFFFFF
};
```

## Utility Functions

### SceneUtils Namespace

```cpp
namespace SceneUtils {
    // Factory function for creating scene managers
    std::unique_ptr<SceneManager> CreateSceneManager();
    
    // Create test scenes for development
    std::shared_ptr<Pyramid::Scene> CreateSpatialTestScene(u32 objectCount = 1000);
    
    // Scene validation and optimization
    bool ValidateScene(const std::shared_ptr<Pyramid::Scene>& scene);
    void OptimizeScene(std::shared_ptr<Pyramid::Scene> scene);
}
```

### SpatialUtils Namespace

```cpp
namespace SpatialUtils {
    // AABB calculation for render objects
    AABB CalculateAABB(const std::shared_ptr<RenderObject>& object);
    
    // Frustum plane calculation from camera
    std::array<Math::Vec4, 6> CalculateFrustumPlanes(const Camera& camera);
    
    // Intersection testing utilities
    bool AABBInFrustum(const AABB& aabb, const std::array<Math::Vec4, 6>& frustumPlanes);
    f32 DistanceToAABB(const Math::Vec3& point, const AABB& aabb);
    bool RayAABBIntersection(const Math::Vec3& origin, const Math::Vec3& direction, 
                           const AABB& aabb, f32& distance);
    bool SphereAABBIntersection(const Math::Vec3& center, f32 radius, const AABB& aabb);
}
```

## Performance Characteristics

### Complexity Analysis

- **Octree Insertion**: O(log n) average, O(n) worst case
- **Octree Query**: O(log n + k) where k is result count
- **Brute Force Query**: O(n) linear search
- **Spatial Partition Rebuild**: O(n log n)

### Memory Usage

- **Octree Node**: ~64 bytes per node
- **AABB**: 24 bytes (6 floats)
- **SceneManager**: ~200 bytes + octree overhead
- **Typical Scene**: 1000 objects ≈ 100KB octree memory

### Recommended Settings

- **Small Scenes** (< 100 objects): Octree depth 4-6
- **Medium Scenes** (100-1000 objects): Octree depth 6-8
- **Large Scenes** (1000+ objects): Octree depth 8-10
- **Objects per Node**: 5-15 depending on query patterns

## Integration Notes

### Thread Safety

The Scene Management system is **not thread-safe** by design for performance reasons. External synchronization is required for multi-threaded access.

### Memory Management

All objects use smart pointers (`std::shared_ptr`) for automatic memory management. The system follows RAII principles for resource cleanup.

### Future Enhancements

- Scene serialization (JSON/Binary formats)
- Advanced occlusion culling
- Level-of-detail (LOD) system integration
- Multi-threaded spatial queries
- GPU-accelerated culling
