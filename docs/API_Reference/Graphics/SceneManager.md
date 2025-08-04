# Pyramid SceneManager API Reference

## Overview

The Pyramid SceneManager provides advanced scene management capabilities with spatial partitioning, efficient querying, and comprehensive serialization support. It integrates seamlessly with the Pyramid Engine's rendering pipeline and offers powerful tools for organizing, querying, and optimizing 3D scenes.

## Key Features

- **Spatial Partitioning**: Octree-based spatial partitioning for efficient culling and queries
- **Scene Queries**: Multiple query types (point, sphere, box, frustum, ray) for spatial operations
- **Scene Serialization**: Support for binary, JSON, and XML scene formats
- **Performance Monitoring**: Built-in statistics and performance tracking
- **Event System**: Callback-based event handling for scene changes
- **Advanced Culling**: Frustum and occlusion culling with LOD support
- **Debug Visualization**: Visual debugging tools for scene analysis

## Architecture

The SceneManager follows a modular architecture:

```
Application Layer
├── Game Logic
├── Resource Management
└── User Interface

Scene Management Layer
├── SceneManager (Main Interface)
├── Scene (Scene Graph)
├── SceneNode (Scene Elements)
└── RenderObject (Renderable Entities)

Spatial Partitioning Layer
├── Octree (Spatial Structure)
├── Query System (Spatial Queries)
└── Culling System (Visibility Determination)

Serialization Layer
├── Binary Format (Fast Loading)
├── JSON Format (Human Readable)
└── XML Format (Structured Data)
```

## Quick Start

```cpp
#include <Pyramid/Graphics/Scene/SceneManager.hpp>
#include <Pyramid/Graphics/Camera.hpp>

using namespace Pyramid;
using namespace Pyramid::SceneManagement;

// Create scene manager
auto sceneManager = SceneUtils::CreateSceneManager();

// Create and configure scene
auto scene = sceneManager->CreateScene("MainScene");
sceneManager->SetActiveScene(scene);

// Configure spatial partitioning
sceneManager->SetOctreeDepth(8);
sceneManager->SetOctreeSize(Math::Vec3(1000.0f, 1000.0f, 1000.0f));
sceneManager->RebuildSpatialPartition();

// Spatial queries
Math::Vec3 playerPos = GetPlayerPosition();
auto nearbyObjects = sceneManager->GetObjectsInRadius(playerPos, 10.0f);
auto visibleObjects = sceneManager->GetVisibleObjects(camera);

// Update scene
sceneManager->Update(deltaTime);
```

## Query Types

The SceneManager supports various spatial query types:

```cpp
enum class QueryType
{
    Point,    // Point-in-space queries
    Sphere,   // Sphere intersection queries
    Box,      // Axis-aligned bounding box queries
    Frustum,  // Camera frustum queries
    Ray       // Ray intersection queries
};
```

## Constructor and Initialization

### Constructor
```cpp
SceneManager();
```

Creates a new SceneManager instance with default settings:
- Spatial partitioning enabled
- Octree depth: 8 levels
- Octree size: 1000x1000x1000 units
- Frustum culling enabled
- LOD enabled

### Factory Method
```cpp
std::unique_ptr<SceneManager> SceneUtils::CreateSceneManager();
```

Creates a SceneManager instance with default settings.

**Example:**
```cpp
// Create scene manager using constructor
SceneManager sceneManager;

// Create scene manager using factory method
auto sceneManager = SceneUtils::CreateSceneManager();
```

## Scene Management

### Create Scene
```cpp
std::shared_ptr<Pyramid::Scene> CreateScene(const std::string &name);
```

Creates a new scene with the specified name.

**Parameters:**
- `name`: Name of the scene

**Returns:** Shared pointer to the created scene

**Example:**
```cpp
// Create scenes
auto mainScene = sceneManager->CreateScene("MainScene");
auto uiScene = sceneManager->CreateScene("UIScene");
auto loadingScene = sceneManager->CreateScene("LoadingScene");
```

### Load and Save Scenes
```cpp
bool LoadScene(const std::string &filePath, SerializationFormat format = SerializationFormat::JSON);
bool SaveScene(const std::string &filePath, SerializationFormat format = SerializationFormat::JSON);
```

Loads or saves scenes from/to files in various formats.

**Parameters:**
- `filePath`: Path to the scene file
- `format`: Serialization format (Binary, JSON, or XML)

**Returns:** `true` if successful, `false` otherwise

**Example:**
```cpp
// Save scene in different formats
sceneManager->SaveScene("scenes/main_scene.json", SerializationFormat::JSON);
sceneManager->SaveScene("scenes/main_scene.bin", SerializationFormat::Binary);
sceneManager->SaveScene("scenes/main_scene.xml", SerializationFormat::XML);

// Load scene
if (!sceneManager->LoadScene("scenes/main_scene.json")) {
    PYRAMID_LOG_ERROR("Failed to load scene");
    return false;
}
```

### Active Scene Management
```cpp
void SetActiveScene(std::shared_ptr<Pyramid::Scene> scene);
std::shared_ptr<Pyramid::Scene> GetActiveScene() const;
```

Manages the currently active scene.

**Example:**
```cpp
// Set active scene
sceneManager->SetActiveScene(mainScene);

// Get active scene
auto currentScene = sceneManager->GetActiveScene();
if (currentScene) {
    PYRAMID_LOG_INFO("Active scene: ", currentScene->GetName());
}
```

## Spatial Partitioning

### Configuration
```cpp
void EnableSpatialPartitioning(bool enable);
bool IsSpatialPartitioningEnabled() const;
void SetOctreeDepth(u32 depth);
void SetOctreeSize(const Math::Vec3 &size);
void RebuildSpatialPartition();
```

Configures and manages the octree spatial partitioning system.

**Parameters:**
- `enable`: Enable or disable spatial partitioning
- `depth`: Maximum octree depth (affects performance and memory usage)
- `size`: Size of the octree bounding box

**Example:**
```cpp
// Configure spatial partitioning
sceneManager->EnableSpatialPartitioning(true);
sceneManager->SetOctreeDepth(10);           // Deeper tree for more precision
sceneManager->SetOctreeSize(Math::Vec3(2000.0f, 2000.0f, 2000.0f));  // Larger world size

// Rebuild spatial partitioning (call after adding many objects)
sceneManager->RebuildSpatialPartition();
```

### Octree Optimization
```cpp
// Optimize octree for different scenarios
void OptimizeForLargeWorld() {
    sceneManager->SetOctreeDepth(6);   // Shallower tree for large worlds
    sceneManager->SetOctreeSize(Math::Vec3(10000.0f, 10000.0f, 10000.0f));
    sceneManager->RebuildSpatialPartition();
}

void OptimizeForDetailedScene() {
    sceneManager->SetOctreeDepth(12);  // Deeper tree for detailed scenes
    sceneManager->SetOctreeSize(Math::Vec3(500.0f, 500.0f, 500.0f));
    sceneManager->RebuildSpatialPartition();
}
```

## Scene Queries

### General Query
```cpp
QueryResult QueryScene(const QueryParams &params);
```

Performs a general spatial query with custom parameters.

**Parameters:**
- `params`: Query parameters including type, position, size, etc.

**Returns:** QueryResult containing found objects and statistics

**Example:**
```cpp
// Point query
QueryParams pointQuery;
pointQuery.type = QueryType::Point;
pointQuery.position = Math::Vec3(10.0f, 5.0f, 0.0f);
auto pointResult = sceneManager->QueryScene(pointQuery);

// Sphere query
QueryParams sphereQuery;
sphereQuery.type = QueryType::Sphere;
sphereQuery.position = playerPosition;
sphereQuery.radius = 20.0f;
auto sphereResult = sceneManager->QueryScene(sphereQuery);

// Process results
for (const auto& object : sphereResult.objects) {
    PYRAMID_LOG_INFO("Found object at distance: ", object->GetPosition().Distance(playerPosition));
}
```

### Specialized Queries
```cpp
std::vector<std::shared_ptr<RenderObject>> GetVisibleObjects(const Camera &camera);
std::vector<std::shared_ptr<RenderObject>> GetObjectsInRadius(const Math::Vec3 &center, f32 radius);
std::vector<std::shared_ptr<RenderObject>> GetObjectsInBox(const Math::Vec3 &min, const Math::Vec3 &max);
std::shared_ptr<RenderObject> GetNearestObject(const Math::Vec3 &position);
```

Specialized query methods for common use cases.

**Example:**
```cpp
// Get visible objects for rendering
auto visibleObjects = sceneManager->GetVisibleObjects(camera);
for (const auto& object : visibleObjects) {
    object->Render();
}

// Get objects in radius for gameplay logic
auto nearbyObjects = sceneManager->GetObjectsInRadius(playerPosition, 15.0f);
for (const auto& object : nearbyObjects) {
    if (object->GetType() == "Pickup") {
        player->CollectPickup(object);
    }
}

// Get objects in bounding box for area effects
Math::Vec3 boxMin = explosionPosition - Math::Vec3(10.0f, 10.0f, 10.0f);
Math::Vec3 boxMax = explosionPosition + Math::Vec3(10.0f, 10.0f, 10.0f);
auto objectsInBlast = sceneManager->GetObjectsInBox(boxMin, boxMax);
for (const auto& object : objectsInBlast) {
    object->ApplyExplosionForce(explosionPosition, 50.0f);
}

// Get nearest object for targeting
auto target = sceneManager->GetNearestObject(playerPosition);
if (target) {
    player->SetTarget(target);
}
```

### Ray Casting
```cpp
// Custom ray casting query
std::vector<std::shared_ptr<RenderObject>> RayCast(const Math::Vec3 &origin, const Math::Vec3 &direction, f32 maxDistance = 1000.0f) {
    QueryParams rayQuery;
    rayQuery.type = QueryType::Ray;
    rayQuery.position = origin;
    rayQuery.direction = direction.Normalized();
    rayQuery.maxDistance = maxDistance;
    
    auto result = sceneManager->QueryScene(rayQuery);
    return result.objects;
}

// Usage for shooting mechanics
auto camera = player->GetCamera();
Math::Vec3 rayOrigin, rayDirection;
camera->ScreenToWorldRay(0.5f, 0.5f, rayOrigin, rayDirection);  // Center of screen

auto hitObjects = RayCast(rayOrigin, rayDirection, 100.0f);
if (!hitObjects.empty()) {
    auto firstHit = hitObjects[0];
    firstHit->TakeDamage(10.0f);
}
```

## Scene Updates

### General Update
```cpp
void Update(f32 deltaTime, UpdateFlags flags = UpdateFlags::All);
```

Updates the scene with specified update flags.

**Parameters:**
- `deltaTime`: Time since last update in seconds
- `flags`: Update flags to control which systems to update

**Example:**
```cpp
// Update all systems
sceneManager->Update(deltaTime);

// Update only transforms and visibility
sceneManager->Update(deltaTime, UpdateFlags::Transforms | UpdateFlags::Visibility);

// Update only spatial partitioning
sceneManager->Update(deltaTime, UpdateFlags::SpatialPartition);
```

### Specific Updates
```cpp
void UpdateTransforms();
void UpdateVisibility(const Camera &camera);
void UpdateSpatialPartition();
```

Updates specific systems individually.

**Example:**
```cpp
// Update transforms when objects move
sceneManager->UpdateTransforms();

// Update visibility when camera moves
sceneManager->UpdateVisibility(camera);

// Update spatial partitioning when many objects move
sceneManager->UpdateSpatialPartition();
```

## Performance Monitoring

### Statistics
```cpp
const SceneStats &GetStats() const;
void ResetStats();
```

Accesses and manages scene performance statistics.

**Example:**
```cpp
// Get and display statistics
const SceneStats& stats = sceneManager->GetStats();
PYRAMID_LOG_INFO("Scene Statistics:");
PYRAMID_LOG_INFO("  Total Nodes: ", stats.totalNodes);
PYRAMID_LOG_INFO("  Total Objects: ", stats.totalObjects);
PYRAMID_LOG_INFO("  Visible Objects: ", stats.visibleObjects);
PYRAMID_LOG_INFO("  Culled Objects: ", stats.culledObjects);
PYRAMID_LOG_INFO("  Octree Nodes: ", stats.octreeNodes);
PYRAMID_LOG_INFO("  Octree Depth: ", stats.octreeDepth);
PYRAMID_LOG_INFO("  Last Query Time: ", stats.lastQueryTime, "ms");
PYRAMID_LOG_INFO("  Last Update Time: ", stats.lastUpdateTime, "ms");

// Reset statistics (typically once per frame)
sceneManager->ResetStats();
```

### Performance Optimization
```cpp
// Optimize scene based on statistics
void OptimizeScenePerformance() {
    const SceneStats& stats = sceneManager->GetStats();
    
    // Adjust octree depth based on object count
    if (stats.totalObjects > 10000 && stats.octreeDepth < 12) {
        sceneManager->SetOctreeDepth(stats.octreeDepth + 1);
        sceneManager->RebuildSpatialPartition();
        PYRAMID_LOG_INFO("Increased octree depth to ", stats.octreeDepth + 1);
    }
    
    // Monitor query performance
    if (stats.lastQueryTime > 5.0f) {
        PYRAMID_LOG_WARNING("Scene query performance degraded: ", stats.lastQueryTime, "ms");
    }
}
```

## Event System

### Event Management
```cpp
using SceneEventCallback = std::function<void(const std::string &, std::shared_ptr<SceneNode>)>;
void RegisterEventCallback(const std::string &eventType, SceneEventCallback callback);
void TriggerEvent(const std::string &eventType, std::shared_ptr<SceneNode> node);
```

Manages scene events and callbacks.

**Example:**
```cpp
// Register event callbacks
sceneManager->RegisterEventCallback("ObjectCreated", [](const std::string& type, std::shared_ptr<SceneNode> node) {
    PYRAMID_LOG_INFO("Object created: ", type, " at ", node->GetPosition());
});

sceneManager->RegisterEventCallback("ObjectDestroyed", [](const std::string& type, std::shared_ptr<SceneNode> node) {
    PYRAMID_LOG_INFO("Object destroyed: ", type);
});

sceneManager->RegisterEventCallback("ObjectMoved", [](const std::string& type, std::shared_ptr<SceneNode> node) {
    // Update spatial partitioning when objects move significantly
    if (node->HasMovedSignificantly()) {
        sceneManager->UpdateSpatialPartition();
    }
});

// Trigger events
auto newNode = std::make_shared<SceneNode>();
sceneManager->TriggerEvent("ObjectCreated", newNode);
```

## Advanced Features

### Culling Configuration
```cpp
void SetLODEnabled(bool enabled);
void SetFrustumCullingEnabled(bool enabled);
void SetOcclusionCullingEnabled(bool enabled);
```

Configures advanced culling features.

**Example:**
```cpp
// Configure culling for different scenarios
void ConfigureForHighEnd() {
    sceneManager->SetLODEnabled(true);
    sceneManager->SetFrustumCullingEnabled(true);
    sceneManager->SetOcclusionCullingEnabled(true);
}

void ConfigureForLowEnd() {
    sceneManager->SetLODEnabled(false);
    sceneManager->SetFrustumCullingEnabled(true);
    sceneManager->SetOcclusionCullingEnabled(false);
}

void ConfigureForVR() {
    sceneManager->SetLODEnabled(true);
    sceneManager->SetFrustumCullingEnabled(true);
    sceneManager->SetOcclusionCullingEnabled(false);  // VR typically avoids occlusion culling
}
```

### Debug Visualization
```cpp
void SetDebugVisualization(bool enabled);
void DrawDebugInfo();
```

Enables and manages debug visualization.

**Example:**
```cpp
// Toggle debug visualization
void ToggleDebugVisualization() {
    static bool debugEnabled = false;
    debugEnabled = !debugEnabled;
    sceneManager->SetDebugVisualization(debugEnabled);
    
    if (debugEnabled) {
        PYRAMID_LOG_INFO("Debug visualization enabled");
    } else {
        PYRAMID_LOG_INFO("Debug visualization disabled");
    }
}

// Render debug info
void RenderDebugInfo() {
    if (sceneManager->IsDebugVisualizationEnabled()) {
        sceneManager->DrawDebugInfo();
        
        // Draw additional debug info
        const SceneStats& stats = sceneManager->GetStats();
        DrawText("Objects: " + std::to_string(stats.totalObjects), 10, 10);
        DrawText("Visible: " + std::to_string(stats.visibleObjects), 10, 30);
        DrawText("Culled: " + std::to_string(stats.culledObjects), 10, 50);
    }
}
```

## Update Flags

### Update Flags Definition
```cpp
enum class UpdateFlags : u32
{
    None = 0,
    Transforms = 1 << 0,
    Visibility = 1 << 1,
    Lighting = 1 << 2,
    Materials = 1 << 3,
    SpatialPartition = 1 << 4,
    All = 0xFFFFFFFF
};
```

### Update Flag Usage
```cpp
// Combine update flags using bitwise operations
UpdateFlags GetUpdateFlagsForGameState(GameState state) {
    switch (state) {
        case GameState::Playing:
            return UpdateFlags::All;
        case GameState::Paused:
            return UpdateFlags::Visibility;  // Only update visibility for camera movement
        case GameState::Loading:
            return UpdateFlags::None;       // No updates during loading
        case GameState::Cinematic:
            return UpdateFlags::Transforms | UpdateFlags::Visibility;  // Only transforms and visibility
        default:
            return UpdateFlags::All;
    }
}

// Usage
UpdateFlags flags = GetUpdateFlagsForGameState(currentGameState);
sceneManager->Update(deltaTime, flags);
```

## Serialization Formats

### Serialization Format Definition
```cpp
enum class SerializationFormat
{
    Binary, // Fast binary format
    JSON,   // Human-readable JSON
    XML     // Structured XML format
};
```

### Format Comparison
```cpp
// Choose serialization format based on use case
SerializationFormat ChooseFormatForPurpose(SerializationPurpose purpose) {
    switch (purpose) {
        case SerializationPurpose::RuntimeSave:
            return SerializationFormat::Binary;  // Fastest for runtime saves
        case SerializationPurpose::UserSave:
            return SerializationFormat::JSON;    // Human-readable for user saves
        case SerializationPurpose::EditorExport:
            return SerializationFormat::XML;     // Structured for editor tools
        case SerializationPurpose::NetworkTransfer:
            return SerializationFormat::Binary;  // Compact for network transfer
        default:
            return SerializationFormat::JSON;
    }
}

// Usage
auto format = ChooseFormatForPurpose(SerializationPurpose::UserSave);
sceneManager->SaveScene("savegames/user_save_001.json", format);
```

## Usage Examples

### Basic Scene Setup
```cpp
class SceneSetup {
private:
    std::unique_ptr<SceneManager> sceneManager;
    
public:
    SceneSetup() {
        sceneManager = SceneUtils::CreateSceneManager();
        
        // Create main scene
        auto scene = sceneManager->CreateScene("MainScene");
        sceneManager->SetActiveScene(scene);
        
        // Configure spatial partitioning
        sceneManager->SetOctreeDepth(8);
        sceneManager->SetOctreeSize(Math::Vec3(1000.0f, 1000.0f, 1000.0f));
        sceneManager->RebuildSpatialPartition();
        
        // Configure culling
        sceneManager->SetFrustumCullingEnabled(true);
        sceneManager->SetLODEnabled(true);
        sceneManager->SetOcclusionCullingEnabled(false);
    }
    
    void AddObjectToScene(std::shared_ptr<RenderObject> object) {
        auto scene = sceneManager->GetActiveScene();
        if (scene) {
            scene->AddObject(object);
        }
    }
    
    void Update(f32 deltaTime) {
        sceneManager->Update(deltaTime);
    }
    
    SceneManager* GetSceneManager() { return sceneManager.get(); }
};
```

### Advanced Scene Management
```cpp
class AdvancedSceneManager {
private:
    std::unique_ptr<SceneManager> sceneManager;
    std::unordered_map<std::string, std::shared_ptr<Pyramid::Scene>> scenes;
    
public:
    AdvancedSceneManager() {
        sceneManager = SceneUtils::CreateSceneManager();
        SetupEventHandlers();
    }
    
    void SetupEventHandlers() {
        // Handle object creation
        sceneManager->RegisterEventCallback("ObjectCreated", [this](const std::string& type, std::shared_ptr<SceneNode> node) {
            OnObjectCreated(type, node);
        });
        
        // Handle object destruction
        sceneManager->RegisterEventCallback("ObjectDestroyed", [this](const std::string& type, std::shared_ptr<SceneNode> node) {
            OnObjectDestroyed(type, node);
        });
    }
    
    void LoadLevel(const std::string& levelName) {
        // Load level scene
        std::string scenePath = "levels/" + levelName + ".json";
        if (sceneManager->LoadScene(scenePath)) {
            PYRAMID_LOG_INFO("Level loaded: ", levelName);
            
            // Post-load setup
            OnLevelLoaded();
        } else {
            PYRAMID_LOG_ERROR("Failed to load level: ", levelName);
        }
    }
    
    void SaveLevel(const std::string& levelName) {
        // Save current scene
        std::string scenePath = "levels/" + levelName + ".json";
        if (sceneManager->SaveScene(scenePath)) {
            PYRAMID_LOG_INFO("Level saved: ", levelName);
        } else {
            PYRAMID_LOG_ERROR("Failed to save level: ", levelName);
        }
    }
    
    void OptimizeForCurrentScene() {
        const SceneStats& stats = sceneManager->GetStats();
        
        // Adjust octree settings based on scene complexity
        if (stats.totalObjects > 5000) {
            sceneManager->SetOctreeDepth(10);
            sceneManager->SetOctreeSize(Math::Vec3(2000.0f, 2000.0f, 2000.0f));
        } else if (stats.totalObjects > 1000) {
            sceneManager->SetOctreeDepth(8);
            sceneManager->SetOctreeSize(Math::Vec3(1000.0f, 1000.0f, 1000.0f));
        } else {
            sceneManager->SetOctreeDepth(6);
            sceneManager->SetOctreeSize(Math::Vec3(500.0f, 500.0f, 500.0f));
        }
        
        sceneManager->RebuildSpatialPartition();
    }
    
    std::vector<std::shared_ptr<RenderObject>> GetObjectsInRange(const Math::Vec3& position, f32 radius) {
        return sceneManager->GetObjectsInRadius(position, radius);
    }
    
    std::vector<std::shared_ptr<RenderObject>> GetVisibleObjectsForCamera(const Camera& camera) {
        return sceneManager->GetVisibleObjects(camera);
    }
    
private:
    void OnObjectCreated(const std::string& type, std::shared_ptr<SceneNode> node) {
        // Handle object creation
        if (type == "Player") {
            SetupPlayerObject(node);
        } else if (type == "Enemy") {
            SetupEnemyObject(node);
        }
    }
    
    void OnObjectDestroyed(const std::string& type, std::shared_ptr<SceneNode> node) {
        // Handle object destruction
        PYRAMID_LOG_INFO("Object destroyed: ", type);
    }
    
    void OnLevelLoaded() {
        // Post-load optimization
        OptimizeForCurrentScene();
        
        // Set up level-specific configurations
        sceneManager->SetLODEnabled(true);
        sceneManager->SetFrustumCullingEnabled(true);
    }
};
```

### Scene Query System
```cpp
class SceneQuerySystem {
private:
    SceneManager* sceneManager;
    
public:
    SceneQuerySystem(SceneManager* manager) : sceneManager(manager) {}
    
    // Find all objects of a specific type
    std::vector<std::shared_ptr<RenderObject>> FindObjectsByType(const std::string& type) {
        QueryParams params;
        params.type = QueryType::Sphere;
        params.position = Math::Vec3::Zero;
        params.radius = FLT_MAX;  // Search entire scene
        
        auto result = sceneManager->QueryScene(params);
        
        std::vector<std::shared_ptr<RenderObject>> filtered;
        for (const auto& object : result.objects) {
            if (object->GetType() == type) {
                filtered.push_back(object);
            }
        }
        
        return filtered;
    }
    
    // Find objects within a cone (for spotlights, cone attacks, etc.)
    std::vector<std::shared_ptr<RenderObject>> FindObjectsInCone(const Math::Vec3& origin, const Math::Vec3& direction, f32 angle, f32 maxDistance) {
        // First get objects in sphere
        auto objectsInSphere = sceneManager->GetObjectsInRadius(origin, maxDistance);
        
        std::vector<std::shared_ptr<RenderObject>> objectsInCone;
        f32 cosAngle = cos(angle * 0.5f);
        
        for (const auto& object : objectsInSphere) {
            Math::Vec3 toObject = (object->GetPosition() - origin).Normalized();
            f32 dotProduct = direction.Dot(toObject);
            
            if (dotProduct >= cosAngle) {
                objectsInCone.push_back(object);
            }
        }
        
        return objectsInCone;
    }
    
    // Perform line of sight check
    bool HasLineOfSight(const Math::Vec3& start, const Math::Vec3& end) {
        Math::Vec3 direction = (end - start).Normalized();
        f32 distance = (end - start).Length();
        
        QueryParams params;
        params.type = QueryType::Ray;
        params.position = start;
        params.direction = direction;
        params.maxDistance = distance;
        
        auto result = sceneManager->QueryScene(params);
        
        // If no objects hit, we have line of sight
        return result.objects.empty();
    }
    
    // Find path between two points (simplified)
    std::vector<Math::Vec3> FindPath(const Math::Vec3& start, const Math::Vec3& end) {
        std::vector<Math::Vec3> path;
        
        // Simple direct path with line of sight checks
        if (HasLineOfSight(start, end)) {
            path.push_back(end);
        } else {
            // Simplified: try intermediate points
            Math::Vec3 midpoint = (start + end) * 0.5f;
            midpoint.y += 5.0f;  // Go over obstacles
            
            if (HasLineOfSight(start, midpoint) && HasLineOfSight(midpoint, end)) {
                path.push_back(midpoint);
                path.push_back(end);
            } else {
                // Fallback: direct path (would need proper pathfinding in real implementation)
                path.push_back(end);
            }
        }
        
        return path;
    }
};
```

### Integration with Rendering Pipeline
```cpp
class SceneRenderer {
private:
    SceneManager* sceneManager;
    GraphicsDevice* graphicsDevice;
    
public:
    SceneRenderer(SceneManager* manager, GraphicsDevice* device) 
        : sceneManager(manager), graphicsDevice(device) {}
    
    void RenderScene(const Camera& camera) {
        // Begin frame
        graphicsDevice->Clear(Color(0.1f, 0.1f, 0.1f, 1.0f));
        
        // Get visible objects
        auto visibleObjects = sceneManager->GetVisibleObjects(camera);
        
        // Sort objects by material/texture for optimal rendering
        std::sort(visibleObjects.begin(), visibleObjects.end(), 
            [](const auto& a, const auto& b) {
                if (a->GetMaterial() != b->GetMaterial()) 
                    return a->GetMaterial() < b->GetMaterial();
                return a->GetTexture() < b->GetTexture();
            });
        
        // Render objects
        for (const auto& object : visibleObjects) {
            RenderObject(object, camera);
        }
        
        // Render debug info if enabled
        if (sceneManager->IsDebugVisualizationEnabled()) {
            RenderDebugInfo();
        }
        
        // End frame
        graphicsDevice->Present(true);
    }
    
    void RenderObject(std::shared_ptr<RenderObject> object, const Camera& camera) {
        // Set up rendering state
        auto shader = object->GetShader();
        auto texture = object->GetTexture();
        auto vertexArray = object->GetVertexArray();
        
        shader->bind();
        texture->bind(0);
        vertexArray->bind();
        
        // Set matrices
        shader->setUniform("u_ViewProjection", camera.GetViewProjectionMatrix());
        shader->setUniform("u_WorldMatrix", object->GetWorldMatrix());
        shader->setUniform("u_CameraPosition", camera.GetPosition());
        
        // Set object-specific uniforms
        shader->setUniform("u_Color", object->GetColor());
        shader->setUniform("u_Texture", 0);
        
        // Draw
        graphicsDevice->DrawIndexed(vertexArray->GetIndexBuffer()->getCount());
    }
    
    void RenderDebugInfo() {
        sceneManager->DrawDebugInfo();
        
        // Render additional debug information
        const SceneStats& stats = sceneManager->GetStats();
        RenderDebugText("Objects: " + std::to_string(stats.totalObjects), 10, 10);
        RenderDebugText("Visible: " + std::to_string(stats.visibleObjects), 10, 30);
        RenderDebugText("Culled: " + std::to_string(stats.culledObjects), 10, 50);
        RenderDebugText("Query Time: " + std::to_string(stats.lastQueryTime) + "ms", 10, 70);
    }
};
```

## Best Practices

### Scene Organization
1. **Logical Grouping**: Organize scenes logically (main, UI, loading, etc.)
2. **Object Naming**: Use consistent naming conventions for scene objects
3. **Hierarchy**: Utilize scene node hierarchy for complex objects
4. **References**: Minimize cross-references between scenes

### Performance Optimization
1. **Spatial Partitioning**: Always enable spatial partitioning for complex scenes
2. **LOD System**: Use Level of Detail for distant objects
3. **Culling**: Enable appropriate culling based on scene complexity
4. **Batch Updates**: Group similar operations to minimize update calls

### Memory Management
1. **Shared Pointers**: Use shared pointers for scene objects and nodes
2. **Resource Cleanup**: Properly clean up unused resources
3. **Scene Unloading**: Unload unused scenes to free memory
4. **Object Pooling**: Consider object pooling for frequently created/destroyed objects

### Serialization
1. **Format Selection**: Choose appropriate serialization format for use case
2. **Version Control**: Implement version control for serialized scenes
3. **Error Handling**: Handle serialization errors gracefully
4. **Compression**: Consider compression for large scene files

## Integration with Other Systems

### Integration with Camera System
```cpp
// SceneManager works seamlessly with Camera system
void UpdateSceneVisibility(SceneManager& sceneManager, const Camera& camera) {
    // Update visibility based on camera
    sceneManager->UpdateVisibility(camera);
    
    // Get visible objects
    auto visibleObjects = sceneManager->GetVisibleObjects(camera);
    
    // Process visible objects
    for (const auto& object : visibleObjects) {
        object->SetVisible(true);
        object->UpdateLOD(camera.GetPosition());
    }
}
```

### Integration with Math Library
```cpp
// SceneManager integrates with Math library for spatial operations
void PerformSpatialQueries(SceneManager& sceneManager) {
    // Use Math library types for queries
    Math::Vec3 position = Math::Vec3(10.0f, 5.0f, 0.0f);
    Math::Vec3 direction = Math::Vec3::Forward;
    f32 radius = 20.0f;
    
    // Perform queries
    auto nearbyObjects = sceneManager->GetObjectsInRadius(position, radius);
    
    // Use math operations for calculations
    for (const auto& object : nearbyObjects) {
        f32 distance = Math::Vec3::Distance(position, object->GetPosition());
        Math::Vec3 directionToObject = (object->GetPosition() - position).Normalized();
        
        // Process based on distance and direction
        if (distance < radius && direction.Dot(directionToObject) > 0.5f) {
            object->Activate();
        }
    }
}
```

## Conclusion

The Pyramid SceneManager provides a comprehensive scene management system with advanced spatial partitioning, efficient querying, and robust serialization support. Its integration with the Pyramid Engine's rendering pipeline, camera system, and math library makes it an essential component for managing complex 3D scenes. The system is designed to be both powerful and flexible, supporting everything from simple scenes to large, complex worlds with thousands of objects.

By leveraging the SceneManager's spatial partitioning, query system, and performance monitoring features, developers can create efficient, scalable 3D applications that maintain high performance even with complex scenes and large numbers of objects.