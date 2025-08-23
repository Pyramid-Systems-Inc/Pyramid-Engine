# Physics System API Reference

## Overview

The Pyramid Engine's Physics System provides realistic physics simulation for games and interactive applications. The system is designed to be modular, high-performance, and easy to integrate with the game world.

**Note**: The Physics System is currently in development. This documentation represents the planned API and architecture.

## Planned Features

### Core Physics Features
- **Rigid Body Dynamics**: 3D rigid body simulation with realistic physics
- **Collision Detection**: Efficient collision detection with multiple primitive types
- **Collision Response**: Realistic collision response and physics integration
- **Constraints and Joints**: Springs, hinges, sliders, and custom constraints
- **Force System**: Gravity, friction, drag, and custom force applications
- **Particle System**: Large-scale particle simulation for effects

### Advanced Features
- **Soft Body Physics**: Cloth, fluid, and deformable object simulation
- **Character Controller**: Specialized controller for character movement
- **Trigger Volumes**: Non-physical volumes for game logic triggers
- **Raycasting**: High-performance ray queries for gameplay mechanics
- **Multi-Threading**: Parallel physics simulation for performance

## Planned API Structure

### PhysicsWorld Class

The main physics simulation world:

```cpp
class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();
    
    // World management
    bool Initialize(const PhysicsConfig& config);
    void Shutdown();
    void Step(float deltaTime);
    
    // Rigid body management
    RigidBodyID CreateRigidBody(const RigidBodyDesc& desc);
    void DestroyRigidBody(RigidBodyID bodyId);
    RigidBody* GetRigidBody(RigidBodyID bodyId);
    
    // Collision shape management
    CollisionShapeID CreateBoxShape(const Math::Vec3& halfExtents);
    CollisionShapeID CreateSphereShape(float radius);
    CollisionShapeID CreateCapsuleShape(float radius, float height);
    CollisionShapeID CreateMeshShape(const Mesh& mesh);
    void DestroyCollisionShape(CollisionShapeID shapeId);
    
    // World properties
    void SetGravity(const Math::Vec3& gravity);
    Math::Vec3 GetGravity() const;
    
    // Queries
    bool Raycast(const Math::Vec3& origin, const Math::Vec3& direction, 
                float maxDistance, RaycastHit& hit);
    std::vector<RigidBodyID> QuerySphere(const Math::Vec3& center, float radius);
    std::vector<RigidBodyID> QueryBox(const Math::Vec3& center, const Math::Vec3& halfExtents);
    
    // Force application
    void ApplyForce(RigidBodyID bodyId, const Math::Vec3& force);
    void ApplyForceAtPosition(RigidBodyID bodyId, const Math::Vec3& force, const Math::Vec3& position);
    void ApplyTorque(RigidBodyID bodyId, const Math::Vec3& torque);
    void ApplyImpulse(RigidBodyID bodyId, const Math::Vec3& impulse);
    
    // Constraints and joints
    ConstraintID CreateConstraint(const ConstraintDesc& desc);
    void DestroyConstraint(ConstraintID constraintId);
    
    // Collision callbacks
    void SetCollisionCallback(CollisionCallback callback);
    void SetTriggerCallback(TriggerCallback callback);
    
private:
    std::unique_ptr<PhysicsWorldImpl> m_impl;
};
```

### RigidBody Class

Individual physics rigid body:

```cpp
class RigidBody {
public:
    // Transform
    void SetPosition(const Math::Vec3& position);
    Math::Vec3 GetPosition() const;
    void SetRotation(const Math::Quat& rotation);
    Math::Quat GetRotation() const;
    void SetTransform(const Math::Vec3& position, const Math::Quat& rotation);
    
    // Physics properties
    void SetMass(float mass);
    float GetMass() const;
    void SetInertia(const Math::Vec3& inertia);
    Math::Vec3 GetInertia() const;
    
    // Motion
    void SetLinearVelocity(const Math::Vec3& velocity);
    Math::Vec3 GetLinearVelocity() const;
    void SetAngularVelocity(const Math::Vec3& velocity);
    Math::Vec3 GetAngularVelocity() const;
    
    // Material properties
    void SetFriction(float friction);
    float GetFriction() const;
    void SetRestitution(float restitution);
    float GetRestitution() const;
    void SetDrag(float linearDrag, float angularDrag);
    
    // State control
    void SetKinematic(bool kinematic);
    bool IsKinematic() const;
    void SetActive(bool active);
    bool IsActive() const;
    void SetSleeping(bool sleeping);
    bool IsSleeping() const;
    
    // Force application
    void ApplyForce(const Math::Vec3& force);
    void ApplyForceAtPosition(const Math::Vec3& force, const Math::Vec3& position);
    void ApplyTorque(const Math::Vec3& torque);
    void ApplyImpulse(const Math::Vec3& impulse);
    void ApplyImpulseAtPosition(const Math::Vec3& impulse, const Math::Vec3& position);
    
    // Collision shape
    void SetCollisionShape(CollisionShapeID shapeId);
    CollisionShapeID GetCollisionShape() const;
    
    // User data
    void SetUserData(void* userData);
    void* GetUserData() const;
    
private:
    RigidBodyID m_bodyId;
    PhysicsWorld* m_world;
};
```

### CollisionShape Classes

Various collision shape types:

```cpp
// Base collision shape
class CollisionShape {
public:
    virtual ~CollisionShape() = default;
    virtual CollisionShapeType GetType() const = 0;
    virtual Math::Vec3 CalculateLocalInertia(float mass) const = 0;
};

// Box collision shape
class BoxShape : public CollisionShape {
public:
    BoxShape(const Math::Vec3& halfExtents);
    CollisionShapeType GetType() const override { return CollisionShapeType::Box; }
    Math::Vec3 CalculateLocalInertia(float mass) const override;
    
    Math::Vec3 GetHalfExtents() const { return m_halfExtents; }
    
private:
    Math::Vec3 m_halfExtents;
};

// Sphere collision shape
class SphereShape : public CollisionShape {
public:
    SphereShape(float radius);
    CollisionShapeType GetType() const override { return CollisionShapeType::Sphere; }
    Math::Vec3 CalculateLocalInertia(float mass) const override;
    
    float GetRadius() const { return m_radius; }
    
private:
    float m_radius;
};

// Capsule collision shape
class CapsuleShape : public CollisionShape {
public:
    CapsuleShape(float radius, float height);
    CollisionShapeType GetType() const override { return CollisionShapeType::Capsule; }
    Math::Vec3 CalculateLocalInertia(float mass) const override;
    
    float GetRadius() const { return m_radius; }
    float GetHeight() const { return m_height; }
    
private:
    float m_radius;
    float m_height;
};

// Mesh collision shape
class MeshShape : public CollisionShape {
public:
    MeshShape(const std::vector<Math::Vec3>& vertices, const std::vector<u32>& indices);
    CollisionShapeType GetType() const override { return CollisionShapeType::Mesh; }
    Math::Vec3 CalculateLocalInertia(float mass) const override;
    
private:
    std::vector<Math::Vec3> m_vertices;
    std::vector<u32> m_indices;
};
```

### Constraints and Joints

```cpp
// Base constraint class
class Constraint {
public:
    virtual ~Constraint() = default;
    virtual ConstraintType GetType() const = 0;
    
    void SetBodyA(RigidBodyID bodyId) { m_bodyA = bodyId; }
    void SetBodyB(RigidBodyID bodyId) { m_bodyB = bodyId; }
    
protected:
    RigidBodyID m_bodyA = InvalidRigidBodyID;
    RigidBodyID m_bodyB = InvalidRigidBodyID;
};

// Point-to-point constraint
class PointConstraint : public Constraint {
public:
    PointConstraint(const Math::Vec3& anchorA, const Math::Vec3& anchorB);
    ConstraintType GetType() const override { return ConstraintType::Point; }
    
    void SetAnchorA(const Math::Vec3& anchor) { m_anchorA = anchor; }
    void SetAnchorB(const Math::Vec3& anchor) { m_anchorB = anchor; }
    
private:
    Math::Vec3 m_anchorA;
    Math::Vec3 m_anchorB;
};

// Hinge constraint
class HingeConstraint : public Constraint {
public:
    HingeConstraint(const Math::Vec3& pivotA, const Math::Vec3& pivotB,
                   const Math::Vec3& axisA, const Math::Vec3& axisB);
    ConstraintType GetType() const override { return ConstraintType::Hinge; }
    
    void SetLimits(float lowerLimit, float upperLimit);
    void SetMotor(float targetVelocity, float maxMotorImpulse);
    
private:
    Math::Vec3 m_pivotA, m_pivotB;
    Math::Vec3 m_axisA, m_axisB;
    float m_lowerLimit = -Math::Pi;
    float m_upperLimit = Math::Pi;
};

// Spring constraint
class SpringConstraint : public Constraint {
public:
    SpringConstraint(float restLength, float stiffness, float damping);
    ConstraintType GetType() const override { return ConstraintType::Spring; }
    
    void SetStiffness(float stiffness) { m_stiffness = stiffness; }
    void SetDamping(float damping) { m_damping = damping; }
    void SetRestLength(float length) { m_restLength = length; }
    
private:
    float m_restLength;
    float m_stiffness;
    float m_damping;
};
```

## Planned Configuration

### PhysicsConfig Structure

```cpp
struct PhysicsConfig {
    // Simulation settings
    Math::Vec3 gravity = Math::Vec3(0.0f, -9.81f, 0.0f);
    float timeStep = 1.0f / 60.0f;  // 60 FPS
    u32 maxSubSteps = 10;
    u32 velocityIterations = 6;
    u32 positionIterations = 2;
    
    // World settings
    Math::Vec3 worldMin = Math::Vec3(-1000.0f);
    Math::Vec3 worldMax = Math::Vec3(1000.0f);
    u32 maxRigidBodies = 10000;
    u32 maxConstraints = 5000;
    
    // Performance settings
    bool enableMultiThreading = true;
    u32 threadCount = 0;  // 0 = auto-detect
    bool enableContinuousCollisionDetection = true;
    bool enableSleeping = true;
    float sleepThreshold = 0.5f;
    
    // Collision settings
    u32 collisionMargin = 0.01f;
    bool enableContactCache = true;
    float contactBreakingThreshold = 0.02f;
};
```

## Planned Usage Examples

### Basic Rigid Body Physics

```cpp
#include <Pyramid/Physics/PhysicsWorld.hpp>

// Initialize physics world
PhysicsConfig config;
config.gravity = Math::Vec3(0.0f, -9.81f, 0.0f);
config.enableMultiThreading = true;

PhysicsWorld physicsWorld;
if (!physicsWorld.Initialize(config)) {
    PYRAMID_LOG_ERROR("Failed to initialize physics world");
    return false;
}

// Create ground plane
RigidBodyDesc groundDesc;
groundDesc.type = RigidBodyType::Static;
groundDesc.position = Math::Vec3(0.0f, -1.0f, 0.0f);
auto groundShapeId = physicsWorld.CreateBoxShape(Math::Vec3(50.0f, 1.0f, 50.0f));
groundDesc.collisionShape = groundShapeId;

auto groundBodyId = physicsWorld.CreateRigidBody(groundDesc);

// Create falling box
RigidBodyDesc boxDesc;
boxDesc.type = RigidBodyType::Dynamic;
boxDesc.position = Math::Vec3(0.0f, 10.0f, 0.0f);
boxDesc.mass = 1.0f;
auto boxShapeId = physicsWorld.CreateBoxShape(Math::Vec3(0.5f, 0.5f, 0.5f));
boxDesc.collisionShape = boxShapeId;

auto boxBodyId = physicsWorld.CreateRigidBody(boxDesc);
auto* boxBody = physicsWorld.GetRigidBody(boxBodyId);

// Simulation loop
while (running) {
    float deltaTime = GetDeltaTime();
    
    // Step physics simulation
    physicsWorld.Step(deltaTime);
    
    // Get updated transform for rendering
    Math::Vec3 position = boxBody->GetPosition();
    Math::Quat rotation = boxBody->GetRotation();
    
    // Update render transform
    UpdateRenderObject(boxBodyId, position, rotation);
}
```

### Character Controller

```cpp
class PhysicsCharacterController {
public:
    bool Initialize(PhysicsWorld* world, const Math::Vec3& position) {
        m_world = world;
        
        // Create character capsule
        auto capsuleShape = world->CreateCapsuleShape(0.4f, 1.8f);
        
        RigidBodyDesc desc;
        desc.type = RigidBodyType::Dynamic;
        desc.position = position;
        desc.mass = 70.0f;  // 70kg character
        desc.collisionShape = capsuleShape;
        desc.lockRotation = true;  // Prevent character from falling over
        
        m_bodyId = world->CreateRigidBody(desc);
        m_body = world->GetRigidBody(m_bodyId);
        
        return m_body != nullptr;
    }
    
    void Move(const Math::Vec3& moveDirection, float speed, float deltaTime) {
        if (!m_body) return;
        
        // Apply movement force
        Math::Vec3 force = moveDirection * speed * m_body->GetMass();
        m_body->ApplyForce(force);
        
        // Limit horizontal velocity to prevent infinite acceleration
        Math::Vec3 velocity = m_body->GetLinearVelocity();
        Math::Vec3 horizontalVel(velocity.x, 0.0f, velocity.z);
        
        if (horizontalVel.Length() > speed) {
            horizontalVel = horizontalVel.Normalized() * speed;
            m_body->SetLinearVelocity(Math::Vec3(horizontalVel.x, velocity.y, horizontalVel.z));
        }
    }
    
    void Jump(float jumpForce) {
        if (!m_body || !IsGrounded()) return;
        
        // Apply upward impulse
        m_body->ApplyImpulse(Math::Vec3(0.0f, jumpForce, 0.0f));
    }
    
    bool IsGrounded() {
        if (!m_world || !m_body) return false;
        
        // Raycast downward to check for ground
        Math::Vec3 position = m_body->GetPosition();
        Math::Vec3 rayStart = position;
        Math::Vec3 rayDirection = Math::Vec3(0.0f, -1.0f, 0.0f);
        float rayDistance = 1.1f;  // Slightly more than half character height
        
        RaycastHit hit;
        return m_world->Raycast(rayStart, rayDirection, rayDistance, hit);
    }
    
    Math::Vec3 GetPosition() const {
        return m_body ? m_body->GetPosition() : Math::Vec3::Zero;
    }
    
private:
    PhysicsWorld* m_world = nullptr;
    RigidBodyID m_bodyId = InvalidRigidBodyID;
    RigidBody* m_body = nullptr;
};
```

### Vehicle Physics

```cpp
class PhysicsVehicle {
public:
    bool Initialize(PhysicsWorld* world, const Math::Vec3& position) {
        m_world = world;
        
        // Create vehicle chassis
        auto chassisShape = world->CreateBoxShape(Math::Vec3(2.0f, 0.5f, 4.0f));
        
        RigidBodyDesc chassisDesc;
        chassisDesc.type = RigidBodyType::Dynamic;
        chassisDesc.position = position;
        chassisDesc.mass = 1500.0f;  // 1.5 ton vehicle
        chassisDesc.collisionShape = chassisShape;
        
        m_chassisId = world->CreateRigidBody(chassisDesc);
        m_chassis = world->GetRigidBody(m_chassisId);
        
        // Create wheels
        CreateWheels();
        
        return m_chassis != nullptr;
    }
    
    void SetThrottle(float throttle) {
        m_throttle = Math::Clamp(throttle, -1.0f, 1.0f);
    }
    
    void SetSteering(float steering) {
        m_steering = Math::Clamp(steering, -1.0f, 1.0f);
    }
    
    void Update(float deltaTime) {
        if (!m_chassis) return;
        
        // Apply engine force
        Math::Vec3 forward = m_chassis->GetRotation() * Math::Vec3::Forward;
        float engineForce = m_throttle * m_maxEngineForce;
        m_chassis->ApplyForce(forward * engineForce);
        
        // Apply steering torque
        Math::Vec3 up = Math::Vec3::Up;
        float steeringTorque = m_steering * m_maxSteeringTorque;
        m_chassis->ApplyTorque(up * steeringTorque);
        
        // Apply drag
        Math::Vec3 velocity = m_chassis->GetLinearVelocity();
        Math::Vec3 dragForce = -velocity * velocity.Length() * m_dragCoefficient;
        m_chassis->ApplyForce(dragForce);
    }
    
private:
    void CreateWheels() {
        // Wheel creation and constraints would be implemented here
        // This would involve creating wheel rigid bodies and connecting them
        // to the chassis with appropriate constraints
    }
    
    PhysicsWorld* m_world = nullptr;
    RigidBodyID m_chassisId = InvalidRigidBodyID;
    RigidBody* m_chassis = nullptr;
    
    std::vector<RigidBodyID> m_wheelIds;
    std::vector<ConstraintID> m_wheelConstraints;
    
    float m_throttle = 0.0f;
    float m_steering = 0.0f;
    float m_maxEngineForce = 5000.0f;
    float m_maxSteeringTorque = 1000.0f;
    float m_dragCoefficient = 0.3f;
};
```

### Collision Detection and Response

```cpp
class CollisionManager {
public:
    void Initialize(PhysicsWorld* world) {
        m_world = world;
        
        // Set collision callbacks
        world->SetCollisionCallback([this](const CollisionInfo& info) {
            OnCollision(info);
        });
        
        world->SetTriggerCallback([this](const TriggerInfo& info) {
            OnTrigger(info);
        });
    }
    
private:
    void OnCollision(const CollisionInfo& info) {
        // Get user data to identify game objects
        void* userDataA = info.bodyA->GetUserData();
        void* userDataB = info.bodyB->GetUserData();
        
        GameObject* objA = static_cast<GameObject*>(userDataA);
        GameObject* objB = static_cast<GameObject*>(userDataB);
        
        if (objA && objB) {
            // Handle collision between game objects
            HandleCollision(objA, objB, info);
        }
    }
    
    void OnTrigger(const TriggerInfo& info) {
        void* userData = info.triggerBody->GetUserData();
        void* otherData = info.otherBody->GetUserData();
        
        TriggerVolume* trigger = static_cast<TriggerVolume*>(userData);
        GameObject* other = static_cast<GameObject*>(otherData);
        
        if (trigger && other) {
            if (info.entering) {
                trigger->OnEnter(other);
            } else {
                trigger->OnExit(other);
            }
        }
    }
    
    void HandleCollision(GameObject* objA, GameObject* objB, const CollisionInfo& info) {
        // Example collision handling
        if (objA->GetType() == GameObjectType::Player && objB->GetType() == GameObjectType::Enemy) {
            // Player hit enemy
            Player* player = static_cast<Player*>(objA);
            Enemy* enemy = static_cast<Enemy*>(objB);
            
            player->TakeDamage(enemy->GetContactDamage());
            
            // Play collision sound
            AudioManager::PlaySound("collision_sound.wav", info.contactPoint);
            
            // Create particle effect
            ParticleManager::CreateEffect("hit_sparks", info.contactPoint);
        }
    }
    
    PhysicsWorld* m_world = nullptr;
};
```

## Implementation Status

**Current Status**: Planning and Architecture Phase

### Completed
- ✅ Architecture design
- ✅ API specification
- ✅ Integration planning with existing systems

### In Progress
- 🔄 Core physics engine research (Bullet Physics vs custom implementation)
- 🔄 Collision detection system design
- 🔄 Rigid body dynamics implementation

### Planned
- ⏳ Collision shape implementations
- ⏳ Constraint and joint system
- ⏳ Character controller
- ⏳ Vehicle physics
- ⏳ Soft body physics
- ⏳ Multi-threading optimization
- ⏳ Performance profiling and optimization

## Performance Considerations

### Broad Phase Collision Detection
- **Spatial Partitioning**: Octree or AABB tree for efficient collision culling
- **Continuous Collision Detection**: Prevent fast-moving objects from tunneling
- **Contact Caching**: Reuse collision information between frames

### Solver Optimization
- **Iterative Solvers**: Configurable iteration counts for quality vs performance
- **Island Solving**: Solve disconnected object groups independently
- **Sleeping**: Disable simulation for stationary objects

### Memory Management
- **Object Pooling**: Reuse physics objects to minimize allocations
- **SOA Layout**: Structure-of-Arrays for cache-friendly data access
- **Memory Mapping**: Efficient memory layout for SIMD operations

## See Also

- [Core Game Class](../Core/Game.md) - Main game loop integration
- [Math Library](../Math/MathLibrary.md) - Vector and matrix operations
- [Scene Management](../Graphics/SceneManager.md) - Spatial organization
- [Performance Profiling](../Utils/Profiling.md) - Physics performance monitoring

## Future Extensions

- **Fluid Simulation**: Water and gas dynamics
- **Destruction Physics**: Breakable objects and fracturing
- **Cloth Simulation**: Realistic fabric and rope physics
- **Physics Scripting**: Lua/C# scripting for complex physics behaviors
- **VR Physics**: Specialized physics for virtual reality applications
