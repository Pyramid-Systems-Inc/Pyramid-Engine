# Scene Management System - Usage Examples

## Overview

This document provides comprehensive examples of using the Scene Management Core Architecture in the Pyramid Game Engine. The examples demonstrate real-world usage patterns for game development scenarios.

## Basic Setup

### Creating a Scene Manager

```cpp
#include <Pyramid/Graphics/Scene/SceneManager.hpp>
#include <Pyramid/Graphics/Scene/Octree.hpp>
#include <Pyramid/Util/Log.hpp>

using namespace Pyramid::SceneManagement;

class GameWorld {
private:
    std::unique_ptr<SceneManager> m_sceneManager;
    std::shared_ptr<Pyramid::Scene> m_currentScene;

public:
    void Initialize() {
        // Create scene manager with default settings
        m_sceneManager = SceneUtils::CreateSceneManager();
        
        // Configure spatial partitioning for your world size
        m_sceneManager->SetOctreeDepth(8);  // Good for medium-large worlds
        m_sceneManager->SetOctreeSize(Math::Vec3(2000.0f, 500.0f, 2000.0f));
        
        // Create main game scene
        m_currentScene = m_sceneManager->CreateScene("GameWorld");
        m_sceneManager->SetActiveScene(m_currentScene);
        
        PYRAMID_LOG_INFO("Scene management system initialized");
        PYRAMID_LOG_INFO("Octree bounds: 2000x500x2000 units, depth: 8");
    }
};
```

### Scene Lifecycle Management

```cpp
class SceneController {
private:
    std::unique_ptr<SceneManager> m_sceneManager;
    std::unordered_map<std::string, std::shared_ptr<Pyramid::Scene>> m_scenes;

public:
    void LoadLevel(const std::string& levelName) {
        // Create new scene for the level
        auto scene = m_sceneManager->CreateScene(levelName);
        m_scenes[levelName] = scene;
        
        // Load level-specific objects
        LoadLevelObjects(scene, levelName);
        
        // Switch to new scene
        m_sceneManager->SetActiveScene(scene);
        
        // Force rebuild spatial partition for new objects
        m_sceneManager->RebuildSpatialPartition();
        
        PYRAMID_LOG_INFO("Loaded level: ", levelName);
    }
    
    void UnloadLevel(const std::string& levelName) {
        auto it = m_scenes.find(levelName);
        if (it != m_scenes.end()) {
            // Scene will be automatically cleaned up when shared_ptr is destroyed
            m_scenes.erase(it);
            PYRAMID_LOG_INFO("Unloaded level: ", levelName);
        }
    }
    
private:
    void LoadLevelObjects(std::shared_ptr<Pyramid::Scene> scene, const std::string& levelName) {
        // Example: Load objects from level data
        // This would typically load from a file or database
        
        // Add some example objects
        for (int i = 0; i < 100; ++i) {
            auto object = std::make_shared<RenderObject>();
            object->position = Math::Vec3(
                Math::Random(-500.0f, 500.0f),
                0.0f,
                Math::Random(-500.0f, 500.0f)
            );
            // Add object to scene (implementation depends on your Scene class)
        }
    }
};
```

## Spatial Queries for Game Logic

### Player Interaction System

```cpp
class InteractionSystem {
private:
    SceneManager* m_sceneManager;
    f32 m_interactionRange = 3.0f;

public:
    InteractionSystem(SceneManager* sceneManager) : m_sceneManager(sceneManager) {}
    
    void UpdatePlayerInteraction(const Math::Vec3& playerPosition) {
        // Find all objects within interaction range
        auto nearbyObjects = m_sceneManager->GetObjectsInRadius(playerPosition, m_interactionRange);
        
        // Filter for interactive objects
        std::vector<std::shared_ptr<RenderObject>> interactables;
        for (const auto& obj : nearbyObjects) {
            if (IsInteractable(obj)) {
                interactables.push_back(obj);
            }
        }
        
        if (!interactables.empty()) {
            // Find the closest interactive object
            auto closest = m_sceneManager->GetNearestObject(playerPosition);
            if (closest && IsInteractable(closest)) {
                ShowInteractionPrompt(closest);
            }
        } else {
            HideInteractionPrompt();
        }
    }
    
private:
    bool IsInteractable(const std::shared_ptr<RenderObject>& object) {
        // Check if object has interaction component
        // This is game-specific logic
        return object && object->hasComponent<InteractionComponent>();
    }
    
    void ShowInteractionPrompt(const std::shared_ptr<RenderObject>& object) {
        // Display UI prompt for interaction
        PYRAMID_LOG_DEBUG("Interaction available with object at: ", 
                         object->position.x, ", ", object->position.y, ", ", object->position.z);
    }
    
    void HideInteractionPrompt() {
        // Hide UI prompt
    }
};
```

### AI Awareness System

```cpp
class AIAwarenessSystem {
private:
    SceneManager* m_sceneManager;
    
public:
    AIAwarenessSystem(SceneManager* sceneManager) : m_sceneManager(sceneManager) {}
    
    void UpdateAIAwareness(const std::vector<AIAgent>& agents) {
        for (auto& agent : agents) {
            UpdateAgentAwareness(agent);
        }
    }
    
private:
    void UpdateAgentAwareness(AIAgent& agent) {
        const f32 awarenessRadius = agent.GetAwarenessRadius();
        const Math::Vec3& agentPos = agent.GetPosition();
        
        // Find all objects within awareness radius
        auto nearbyObjects = m_sceneManager->GetObjectsInRadius(agentPos, awarenessRadius);
        
        // Process each nearby object
        for (const auto& obj : nearbyObjects) {
            if (IsPlayer(obj)) {
                HandlePlayerDetection(agent, obj);
            } else if (IsEnemy(obj)) {
                HandleEnemyDetection(agent, obj);
            } else if (IsItem(obj)) {
                HandleItemDetection(agent, obj);
            }
        }
        
        // Update agent's knowledge of surroundings
        agent.UpdateKnownObjects(nearbyObjects);
    }
    
    void HandlePlayerDetection(AIAgent& agent, const std::shared_ptr<RenderObject>& player) {
        f32 distance = (agent.GetPosition() - player->position).Length();
        
        // Check line of sight using ray query
        QueryParams rayParams;
        rayParams.type = QueryType::Ray;
        rayParams.position = agent.GetPosition();
        rayParams.direction = (player->position - agent.GetPosition()).Normalized();
        rayParams.maxDistance = distance;
        
        auto rayResult = m_sceneManager->QueryScene(rayParams);
        
        // Check if player is the first object hit (no obstacles)
        bool hasLineOfSight = !rayResult.objects.empty() && rayResult.objects[0] == player;
        
        if (hasLineOfSight) {
            agent.OnPlayerSighted(player, distance);
            PYRAMID_LOG_DEBUG("AI agent detected player at distance: ", distance);
        }
    }
    
    void HandleEnemyDetection(AIAgent& agent, const std::shared_ptr<RenderObject>& enemy) {
        // Handle enemy-to-enemy interactions
        agent.OnEnemySighted(enemy);
    }
    
    void HandleItemDetection(AIAgent& agent, const std::shared_ptr<RenderObject>& item) {
        // Handle item detection for AI collection behavior
        if (agent.NeedsItem(item)) {
            agent.SetTarget(item);
        }
    }
    
    bool IsPlayer(const std::shared_ptr<RenderObject>& obj) {
        return obj && obj->hasComponent<PlayerComponent>();
    }
    
    bool IsEnemy(const std::shared_ptr<RenderObject>& obj) {
        return obj && obj->hasComponent<EnemyComponent>();
    }
    
    bool IsItem(const std::shared_ptr<RenderObject>& obj) {
        return obj && obj->hasComponent<ItemComponent>();
    }
};
```

### Combat System with Area Effects

```cpp
class CombatSystem {
private:
    SceneManager* m_sceneManager;
    
public:
    CombatSystem(SceneManager* sceneManager) : m_sceneManager(sceneManager) {}
    
    void ProcessExplosion(const Math::Vec3& center, f32 radius, f32 damage) {
        PYRAMID_LOG_INFO("Processing explosion at: ", center.x, ", ", center.y, ", ", center.z);
        PYRAMID_LOG_INFO("Explosion radius: ", radius, ", damage: ", damage);
        
        // Find all objects within explosion radius
        auto affectedObjects = m_sceneManager->GetObjectsInRadius(center, radius);
        
        PYRAMID_LOG_INFO("Explosion affects ", affectedObjects.size(), " objects");
        
        for (const auto& obj : affectedObjects) {
            if (HasHealthComponent(obj)) {
                f32 distance = (obj->position - center).Length();
                f32 damageMultiplier = 1.0f - (distance / radius); // Linear falloff
                f32 actualDamage = damage * damageMultiplier;
                
                ApplyDamage(obj, actualDamage);
                
                PYRAMID_LOG_DEBUG("Applied ", actualDamage, " damage to object at distance ", distance);
            }
        }
    }
    
    void ProcessAreaOfEffectSpell(const Math::Vec3& center, const Math::Vec3& size, 
                                 const std::string& effectType) {
        // Use box query for rectangular area effects
        QueryParams boxParams;
        boxParams.type = QueryType::Box;
        boxParams.position = center;
        boxParams.size = size;
        
        auto result = m_sceneManager->QueryScene(boxParams);
        
        PYRAMID_LOG_INFO("Area effect '", effectType, "' affects ", result.totalFound, " objects");
        
        for (const auto& obj : result.objects) {
            ApplyMagicEffect(obj, effectType);
        }
    }
    
private:
    bool HasHealthComponent(const std::shared_ptr<RenderObject>& obj) {
        return obj && obj->hasComponent<HealthComponent>();
    }
    
    void ApplyDamage(const std::shared_ptr<RenderObject>& obj, f32 damage) {
        auto health = obj->getComponent<HealthComponent>();
        if (health) {
            health->TakeDamage(damage);
        }
    }
    
    void ApplyMagicEffect(const std::shared_ptr<RenderObject>& obj, const std::string& effectType) {
        // Apply magical effects based on type
        if (effectType == "heal") {
            auto health = obj->getComponent<HealthComponent>();
            if (health) {
                health->Heal(50.0f);
            }
        } else if (effectType == "slow") {
            auto movement = obj->getComponent<MovementComponent>();
            if (movement) {
                movement->ApplySlowEffect(5.0f); // 5 second slow
            }
        }
    }
};
```

## Performance Monitoring and Optimization

### Performance Monitoring System

```cpp
class PerformanceMonitor {
private:
    SceneManager* m_sceneManager;
    f32 m_updateInterval = 1.0f; // Log stats every second
    f32 m_timeSinceLastUpdate = 0.0f;
    
public:
    PerformanceMonitor(SceneManager* sceneManager) : m_sceneManager(sceneManager) {}
    
    void Update(f32 deltaTime) {
        m_timeSinceLastUpdate += deltaTime;
        
        if (m_timeSinceLastUpdate >= m_updateInterval) {
            LogPerformanceStats();
            m_timeSinceLastUpdate = 0.0f;
        }
    }
    
private:
    void LogPerformanceStats() {
        const auto& stats = m_sceneManager->GetStats();
        
        PYRAMID_LOG_INFO("=== Scene Performance Statistics ===");
        PYRAMID_LOG_INFO("Total Objects: ", stats.totalObjects);
        PYRAMID_LOG_INFO("Visible Objects: ", stats.visibleObjects);
        PYRAMID_LOG_INFO("Culled Objects: ", stats.culledObjects);
        PYRAMID_LOG_INFO("Octree Nodes: ", stats.octreeNodes);
        PYRAMID_LOG_INFO("Octree Depth: ", stats.octreeDepth);
        PYRAMID_LOG_INFO("Last Query Time: ", stats.lastQueryTime, " ms");
        PYRAMID_LOG_INFO("Last Update Time: ", stats.lastUpdateTime, " ms");
        
        // Calculate performance metrics
        f32 cullingEfficiency = stats.totalObjects > 0 ? 
            (f32)stats.culledObjects / stats.totalObjects * 100.0f : 0.0f;
        
        PYRAMID_LOG_INFO("Culling Efficiency: ", cullingEfficiency, "%");
        
        // Warn about performance issues
        if (stats.lastQueryTime > 5.0f) {
            PYRAMID_LOG_WARN("High query time detected: ", stats.lastQueryTime, " ms");
        }
        
        if (stats.octreeNodes > 10000) {
            PYRAMID_LOG_WARN("High octree node count: ", stats.octreeNodes);
        }
        
        PYRAMID_LOG_INFO("=====================================");
    }
};
```

### Dynamic Optimization

```cpp
class SceneOptimizer {
private:
    SceneManager* m_sceneManager;
    
public:
    SceneOptimizer(SceneManager* sceneManager) : m_sceneManager(sceneManager) {}
    
    void OptimizeScene() {
        const auto& stats = m_sceneManager->GetStats();
        
        // Check if optimization is needed
        if (ShouldOptimize(stats)) {
            PYRAMID_LOG_INFO("Starting scene optimization...");
            
            // Adjust octree parameters based on current performance
            AdjustOctreeParameters(stats);
            
            // Rebuild spatial partition with new parameters
            m_sceneManager->RebuildSpatialPartition();
            
            PYRAMID_LOG_INFO("Scene optimization completed");
        }
    }
    
private:
    bool ShouldOptimize(const SceneStats& stats) {
        // Optimize if query times are too high
        if (stats.lastQueryTime > 10.0f) return true;
        
        // Optimize if octree is too deep or too shallow
        if (stats.octreeDepth > 12 || stats.octreeDepth < 4) return true;
        
        // Optimize if too many nodes for the object count
        f32 nodesPerObject = stats.totalObjects > 0 ? 
            (f32)stats.octreeNodes / stats.totalObjects : 0.0f;
        if (nodesPerObject > 5.0f) return true;
        
        return false;
    }
    
    void AdjustOctreeParameters(const SceneStats& stats) {
        // Adjust depth based on object count
        u32 newDepth = 6; // Default
        
        if (stats.totalObjects < 100) {
            newDepth = 4;
        } else if (stats.totalObjects < 500) {
            newDepth = 6;
        } else if (stats.totalObjects < 2000) {
            newDepth = 8;
        } else {
            newDepth = 10;
        }
        
        m_sceneManager->SetOctreeDepth(newDepth);
        
        PYRAMID_LOG_INFO("Adjusted octree depth to: ", newDepth);
    }
};
```

## Integration with BasicGame Example

### Enhanced BasicGame with Scene Management

```cpp
class EnhancedBasicGame : public Pyramid::Game {
private:
    std::unique_ptr<SceneManager> m_sceneManager;
    std::unique_ptr<InteractionSystem> m_interactionSystem;
    std::unique_ptr<PerformanceMonitor> m_performanceMonitor;
    
    Math::Vec3 m_playerPosition = Math::Vec3::Zero;
    
public:
    void onCreate() override {
        Game::onCreate();
        
        // Initialize scene management
        m_sceneManager = SceneUtils::CreateSceneManager();
        m_sceneManager->SetOctreeDepth(6); // Smaller depth for demo
        m_sceneManager->SetOctreeSize(Math::Vec3(100.0f, 50.0f, 100.0f));
        
        // Create demo scene
        auto demoScene = m_sceneManager->CreateScene("DemoScene");
        m_sceneManager->SetActiveScene(demoScene);
        
        // Initialize systems
        m_interactionSystem = std::make_unique<InteractionSystem>(m_sceneManager.get());
        m_performanceMonitor = std::make_unique<PerformanceMonitor>(m_sceneManager.get());
        
        // Create demo objects
        CreateDemoObjects();
        
        PYRAMID_LOG_INFO("Enhanced BasicGame with Scene Management initialized");
    }
    
    void onUpdate(f32 deltaTime) override {
        Game::onUpdate(deltaTime);
        
        // Update player position (simple movement for demo)
        UpdatePlayerPosition(deltaTime);
        
        // Update game systems
        m_interactionSystem->UpdatePlayerInteraction(m_playerPosition);
        m_performanceMonitor->Update(deltaTime);
        
        // Update scene management
        m_sceneManager->Update(deltaTime);
        
        // Log visible objects count periodically
        static f32 logTimer = 0.0f;
        logTimer += deltaTime;
        if (logTimer >= 2.0f) {
            auto camera = GetCamera(); // Assuming you have a camera
            if (camera) {
                auto visibleObjects = m_sceneManager->GetVisibleObjects(*camera);
                PYRAMID_LOG_INFO("Visible objects: ", visibleObjects.size());
            }
            logTimer = 0.0f;
        }
    }
    
private:
    void CreateDemoObjects() {
        // Create a grid of demo objects
        for (int x = -10; x <= 10; x += 2) {
            for (int z = -10; z <= 10; z += 2) {
                auto object = std::make_shared<RenderObject>();
                object->position = Math::Vec3((f32)x, 0.0f, (f32)z);
                
                // Add object to scene (implementation depends on your Scene class)
                // scene->AddObject(object);
            }
        }
        
        PYRAMID_LOG_INFO("Created demo objects in grid pattern");
    }
    
    void UpdatePlayerPosition(f32 deltaTime) {
        // Simple circular movement for demo
        static f32 angle = 0.0f;
        angle += deltaTime * 0.5f; // Rotate slowly
        
        f32 radius = 5.0f;
        m_playerPosition.x = cos(angle) * radius;
        m_playerPosition.z = sin(angle) * radius;
        m_playerPosition.y = 1.0f; // Keep above ground
    }
};
```

This comprehensive set of examples demonstrates how to effectively use the Scene Management Core Architecture in real game scenarios, from basic setup to advanced optimization techniques.
