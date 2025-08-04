# Pyramid Camera API Reference

## Overview

The Pyramid Camera class provides a comprehensive camera system for 3D rendering applications. It supports both perspective and orthographic projections, includes frustum culling capabilities, and offers various camera movement and orientation utilities. The camera system is designed to integrate seamlessly with the Pyramid Engine's rendering pipeline.

## Key Features

- **Dual Projection Support**: Both perspective and orthographic projection types
- **Frustum Culling**: Efficient visibility testing for points and spheres
- **Transform Operations**: Full position and rotation control
- **Movement Utilities**: Forward, right, and up movement with local rotation
- **Matrix Management**: Cached view, projection, and combined matrices
- **Coordinate Conversion**: Screen-to-world and world-to-screen coordinate transformations

## Camera Types

The Camera class supports two projection types:

```cpp
enum class ProjectionType
{
    Perspective,   // Perspective projection for 3D scenes
    Orthographic   // Orthographic projection for 2D or isometric views
};
```

## Quick Start

```cpp
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Math/Math.hpp>

using namespace Pyramid;

// Create a perspective camera
Camera camera(Math::Radians(60.0f),  // 60 degree FOV
             16.0f / 9.0f,          // 16:9 aspect ratio
             0.1f,                  // Near plane
             1000.0f);              // Far plane

// Position and orient the camera
camera.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
camera.LookAt(Math::Vec3::Zero);  // Look at origin

// Get matrices for rendering
const Math::Mat4& viewMatrix = camera.GetViewMatrix();
const Math::Mat4& projectionMatrix = camera.GetProjectionMatrix();
const Math::Mat4& viewProjectionMatrix = camera.GetViewProjectionMatrix();
```

## Constructor and Initialization

### Default Constructor
```cpp
Camera();
```
Creates a perspective camera with default parameters:
- FOV: 60 degrees
- Aspect ratio: 16:9
- Near plane: 0.1 units
- Far plane: 1000.0 units

### Parameterized Constructor
```cpp
Camera(f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane, ProjectionType type = ProjectionType::Perspective);
```

**Parameters:**
- `fov`: Field of view in radians (perspective projection only)
- `aspectRatio`: Aspect ratio (width/height)
- `nearPlane`: Near clipping plane distance
- `farPlane`: Far clipping plane distance
- `type`: Projection type (defaults to Perspective)

**Example:**
```cpp
// Create perspective camera
Camera perspectiveCam(Math::Radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

// Create orthographic camera
Camera orthoCam(Math::Radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f, ProjectionType::Orthographic);
```

## Transform Operations

### Position Control
```cpp
void SetPosition(const Math::Vec3 &position);
const Math::Vec3 &GetPosition() const;
```

**Example:**
```cpp
// Set camera position
camera.SetPosition(Math::Vec3(10.0f, 5.0f, -20.0f));

// Get current position
Math::Vec3 currentPos = camera.GetPosition();
```

### Rotation Control
```cpp
// Set rotation using Euler angles
void SetRotation(f32 pitch, f32 yaw, f32 roll = 0.0f);

// Set rotation using quaternion
void SetRotation(const Math::Quat &rotation);

// Get rotation quaternion
const Math::Quat &GetRotation() const;
```

**Example:**
```cpp
// Set rotation using Euler angles (in radians)
camera.SetRotation(Math::Radians(30.0f), Math::Radians(-45.0f), Math::Radians(0.0f));

// Set rotation using quaternion
Math::Quat rotation = Math::Quat::FromAxisAngle(Math::Vec3::Up, Math::Radians(45.0f));
camera.SetRotation(rotation);
```

### Look At
```cpp
void LookAt(const Math::Vec3 &target, const Math::Vec3 &up = Math::Vec3::Up);
```

Orients the camera to look at a specific target position.

**Parameters:**
- `target`: Target position to look at
- `up`: Up vector (defaults to world up)

**Example:**
```cpp
// Look at origin
camera.LookAt(Math::Vec3::Zero);

// Look at a specific point with custom up vector
camera.LookAt(Math::Vec3(10.0f, 5.0f, 0.0f), Math::Vec3(0.0f, 1.0f, 0.5f).Normalized());
```

## Movement Operations

### Directional Movement
```cpp
void MoveForward(f32 distance);
void MoveRight(f32 distance);
void MoveUp(f32 distance);
```

Moves the camera along its local axes.

**Parameters:**
- `distance`: Distance to move (positive values move in the forward/right/up direction)

**Example:**
```cpp
// Move camera forward 5 units
camera.MoveForward(5.0f);

// Move camera right 2 units
camera.MoveRight(2.0f);

// Move camera up 1 unit
camera.MoveUp(1.0f);

// Move backward (negative distance)
camera.MoveForward(-3.0f);
```

### Rotation
```cpp
void Rotate(f32 pitchDelta, f32 yawDelta, f32 rollDelta = 0.0f);
```

Rotates the camera around its local axes.

**Parameters:**
- `pitchDelta`: Pitch rotation delta in radians
- `yawDelta`: Yaw rotation delta in radians
- `rollDelta`: Roll rotation delta in radians (optional)

**Example:**
```cpp
// Rotate camera 10 degrees up and 5 degrees right
camera.Rotate(Math::Radians(10.0f), Math::Radians(5.0f));

// Complex rotation with roll
camera.Rotate(Math::Radians(-5.0f), Math::Radians(15.0f), Math::Radians(2.0f));
```

## Direction Vectors

```cpp
Math::Vec3 GetForward() const;
Math::Vec3 GetRight() const;
Math::Vec3 GetUp() const;
```

Returns normalized direction vectors representing the camera's local axes.

**Example:**
```cpp
// Get camera directions
Math::Vec3 forward = camera.GetForward();
Math::Vec3 right = camera.GetRight();
Math::Vec3 up = camera.GetUp();

// Use for calculations
Math::Vec3 targetPosition = camera.GetPosition() + forward * 10.0f;
```

## Projection Settings

### Perspective Projection
```cpp
void SetPerspective(f32 fov, f32 aspectRatio, f32 nearPlane, f32 farPlane);
```

Configures the camera for perspective projection.

**Parameters:**
- `fov`: Field of view in radians
- `aspectRatio`: Aspect ratio (width/height)
- `nearPlane`: Near clipping plane distance
- `farPlane`: Far clipping plane distance

**Example:**
```cpp
// Set up perspective projection
camera.SetPerspective(Math::Radians(60.0f),  // 60 degree FOV
                     1920.0f / 1080.0f,     // 16:9 aspect ratio
                     0.1f,                  // Near plane
                     1000.0f);              // Far plane
```

### Orthographic Projection
```cpp
// Set orthographic with explicit planes
void SetOrthographic(f32 left, f32 right, f32 bottom, f32 top, f32 nearPlane, f32 farPlane);

// Set orthographic with width/height
void SetOrthographic(f32 width, f32 height, f32 nearPlane, f32 farPlane);
```

Configures the camera for orthographic projection.

**Parameters:**
- `left`, `right`: Left and right clipping planes
- `bottom`, `top`: Bottom and top clipping planes
- `width`, `height`: Orthographic view width and height
- `nearPlane`: Near clipping plane distance
- `farPlane`: Far clipping plane distance

**Example:**
```cpp
// Set up orthographic projection with explicit planes
camera.SetOrthographic(-10.0f, 10.0f,   // Left, right
                       -10.0f, 10.0f,   // Bottom, top
                       0.1f, 100.0f);   // Near, far

// Set up orthographic projection with width/height
camera.SetOrthographic(20.0f, 20.0f,    // Width, height
                       0.1f, 100.0f);   // Near, far
```

## Matrix Accessors

```cpp
const Math::Mat4 &GetViewMatrix() const;
const Math::Mat4 &GetProjectionMatrix() const;
const Math::Mat4 &GetViewProjectionMatrix() const;
const Math::Mat4 &GetInverseViewMatrix() const;
```

Returns cached transformation matrices. These matrices are automatically updated when camera properties change.

**Example:**
```cpp
// Get matrices for rendering
const Math::Mat4& viewMatrix = camera.GetViewMatrix();
const Math::Mat4& projectionMatrix = camera.GetProjectionMatrix();
const Math::Mat4& viewProjectionMatrix = camera.GetViewProjectionMatrix();
const Math::Mat4& inverseViewMatrix = camera.GetInverseViewMatrix();

// Use in shader
shader->setUniform("u_ViewMatrix", viewMatrix);
shader->setUniform("u_ProjectionMatrix", projectionMatrix);
shader->setUniform("u_ViewProjectionMatrix", viewProjectionMatrix);
```

## Projection Accessors

```cpp
ProjectionType GetProjectionType() const;
f32 GetFOV() const;
f32 GetAspectRatio() const;
f32 GetNearPlane() const;
f32 GetFarPlane() const;
```

Returns current projection parameters.

**Example:**
```cpp
// Get projection information
ProjectionType type = camera.GetProjectionType();
f32 fov = camera.GetFOV();
f32 aspectRatio = camera.GetAspectRatio();
f32 nearPlane = camera.GetNearPlane();
f32 farPlane = camera.GetFarPlane();

// Log camera info
PYRAMID_LOG_INFO("Camera type: ", (type == ProjectionType::Perspective) ? "Perspective" : "Orthographic");
PYRAMID_LOG_INFO("FOV: ", Math::Degrees(fov), " degrees");
PYRAMID_LOG_INFO("Aspect ratio: ", aspectRatio);
PYRAMID_LOG_INFO("Near/Far planes: ", nearPlane, "/", farPlane);
```

## Coordinate Conversion

### Screen to World Ray
```cpp
void ScreenToWorldRay(f32 screenX, f32 screenY, Math::Vec3 &rayOrigin, Math::Vec3 &rayDirection) const;
```

Converts screen coordinates to a world-space ray originating from the camera.

**Parameters:**
- `screenX`: Screen X coordinate (0.0 to 1.0, where 0.0 is left, 1.0 is right)
- `screenY`: Screen Y coordinate (0.0 to 1.0, where 0.0 is top, 1.0 is bottom)
- `rayOrigin`: Output ray origin (camera position)
- `rayDirection`: Output ray direction (normalized)

**Example:**
```cpp
// Get ray from center of screen
Math::Vec3 rayOrigin, rayDirection;
camera.ScreenToWorldRay(0.5f, 0.5f, rayOrigin, rayDirection);

// Get ray from mouse position (assuming mouse coordinates are normalized)
f32 mouseX = static_cast<f32>(mouseX) / windowWidth;
f32 mouseY = static_cast<f32>(mouseY) / windowHeight;
camera.ScreenToWorldRay(mouseX, mouseY, rayOrigin, rayDirection);

// Use for picking or ray casting
Math::Vec3 endPoint = rayOrigin + rayDirection * 100.0f;
```

### World to Screen
```cpp
bool WorldToScreen(const Math::Vec3 &worldPos, f32 &screenX, f32 &screenY) const;
```

Converts world coordinates to screen coordinates.

**Parameters:**
- `worldPos`: World position to convert
- `screenX`: Output screen X coordinate (0.0 to 1.0)
- `screenY`: Output screen Y coordinate (0.0 to 1.0)

**Returns:** `true` if the position is in front of the camera, `false` otherwise

**Example:**
```cpp
// Convert world position to screen coordinates
Math::Vec3 worldPos(10.0f, 5.0f, -20.0f);
f32 screenX, screenY;

if (camera.WorldToScreen(worldPos, screenX, screenY)) {
    // Object is in front of camera
    f32 pixelX = screenX * windowWidth;
    f32 pixelY = screenY * windowHeight;
    PYRAMID_LOG_INFO("Object at pixel coordinates: ", pixelX, ", ", pixelY);
} else {
    // Object is behind camera
    PYRAMID_LOG_INFO("Object is behind camera");
}
```

## Frustum Culling

The Camera class provides efficient frustum culling for visibility testing. This is essential for optimizing rendering performance by skipping objects that are not visible.

### Point Visibility
```cpp
bool IsPointVisible(const Math::Vec3 &point) const;
```

Checks if a point is inside the camera frustum.

**Parameters:**
- `point`: World position to test

**Returns:** `true` if the point is visible, `false` otherwise

**Example:**
```cpp
// Check if a point is visible
Math::Vec3 point(10.0f, 5.0f, -20.0f);
if (camera.IsPointVisible(point)) {
    // Point is visible, render or process it
    RenderObject(point);
}
```

### Sphere Visibility
```cpp
bool IsSphereVisible(const Math::Vec3 &center, f32 radius) const;
```

Checks if a sphere is inside or intersects the camera frustum.

**Parameters:**
- `center`: Sphere center position
- `radius`: Sphere radius

**Returns:** `true` if the sphere is visible (fully or partially), `false` otherwise

**Example:**
```cpp
// Check if an object (represented as a sphere) is visible
Math::Vec3 objectCenter = object->GetPosition();
f32 objectRadius = object->GetBoundingRadius();

if (camera.IsSphereVisible(objectCenter, objectRadius)) {
    // Object is visible, add to render queue
    renderQueue.push_back(object);
} else {
    // Object is not visible, skip rendering
    culledObjects++;
}
```

## Usage Examples

### First-Person Camera Controller
```cpp
class FirstPersonController {
private:
    Camera camera;
    f32 moveSpeed = 5.0f;
    f32 lookSpeed = 0.1f;
    
public:
    FirstPersonController() {
        camera.SetPosition(Math::Vec3(0.0f, 1.8f, 0.0f));
        camera.SetPerspective(Math::Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f);
    }
    
    void Update(f32 deltaTime, f32 mouseDeltaX, f32 mouseDeltaY, bool forward, bool backward, bool left, bool right) {
        // Mouse look
        camera.Rotate(-mouseDeltaY * lookSpeed, -mouseDeltaX * lookSpeed);
        
        // Movement
        f32 moveDistance = moveSpeed * deltaTime;
        if (forward) camera.MoveForward(moveDistance);
        if (backward) camera.MoveForward(-moveDistance);
        if (left) camera.MoveRight(-moveDistance);
        if (right) camera.MoveRight(moveDistance);
    }
    
    const Camera& GetCamera() const { return camera; }
};
```

### Third-Person Camera Controller
```cpp
class ThirdPersonController {
private:
    Camera camera;
    Math::Vec3 targetPosition = Math::Vec3::Zero;
    f32 distance = 10.0f;
    f32 height = 5.0f;
    f32 angle = 0.0f;
    
public:
    ThirdPersonController() {
        camera.SetPerspective(Math::Radians(45.0f), 16.0f/9.0f, 0.1f, 1000.0f);
        UpdateCameraPosition();
    }
    
    void Update(f32 deltaTime, f32 rotationInput) {
        // Rotate around target
        angle += rotationInput * deltaTime * 2.0f;
        UpdateCameraPosition();
    }
    
    void UpdateCameraPosition() {
        // Calculate camera position based on spherical coordinates
        Math::Vec3 cameraPos;
        cameraPos.x = targetPosition.x + distance * cos(angle);
        cameraPos.y = targetPosition.y + height;
        cameraPos.z = targetPosition.z + distance * sin(angle);
        
        camera.SetPosition(cameraPos);
        camera.LookAt(targetPosition);
    }
    
    const Camera& GetCamera() const { return camera; }
};
```

### Isometric Camera Setup
```cpp
// Create isometric camera
Camera CreateIsometricCamera(f32 viewWidth, f32 viewHeight, f32 nearPlane, f32 farPlane) {
    Camera camera;
    
    // Set up orthographic projection
    camera.SetOrthographic(viewWidth, viewHeight, nearPlane, farPlane);
    
    // Position camera for isometric view
    Math::Vec3 cameraPos(viewWidth * 0.5f, viewHeight * 0.5f, -viewHeight);
    camera.SetPosition(cameraPos);
    
    // Look at center of view
    camera.LookAt(Math::Vec3(viewWidth * 0.5f, viewHeight * 0.5f, 0.0f));
    
    return camera;
}

// Usage
auto isometricCamera = CreateIsometricCamera(100.0f, 100.0f, 0.1f, 1000.0f);
```

### Camera System with Multiple Cameras
```cpp
class CameraSystem {
private:
    std::unordered_map<std::string, Camera> cameras;
    std::string activeCameraName;
    
public:
    void AddCamera(const std::string& name, const Camera& camera) {
        cameras[name] = camera;
        if (activeCameraName.empty()) {
            activeCameraName = name;
        }
    }
    
    void SetActiveCamera(const std::string& name) {
        if (cameras.find(name) != cameras.end()) {
            activeCameraName = name;
        }
    }
    
    Camera& GetActiveCamera() {
        return cameras[activeCameraName];
    }
    
    void UpdateCameras(f32 deltaTime) {
        // Update all cameras (e.g., for animation or following targets)
        for (auto& [name, camera] : cameras) {
            // Custom camera update logic
        }
    }
};

// Usage
CameraSystem cameraSystem;
cameraSystem.AddCamera("main", Camera(Math::Radians(60.0f), 16.0f/9.0f, 0.1f, 1000.0f));
cameraSystem.AddCamera("minimap", Camera(Math::Radians(30.0f), 1.0f, 1.0f, 100.0f));
cameraSystem.SetActiveCamera("main");
```

## Performance Considerations

### Matrix Caching
The Camera class uses lazy evaluation for matrix calculations:
- Matrices are only recalculated when needed
- Dirty flags track which matrices need updating
- This minimizes unnecessary computations

**Example:**
```cpp
// Matrix calculations are deferred until actually needed
camera.SetPosition(Math::Vec3(1.0f, 2.0f, 3.0f));  // Sets dirty flag
camera.SetRotation(Math::Quat::Identity);        // Sets dirty flag

// Matrix is calculated here when first accessed
const Math::Mat4& viewMatrix = camera.GetViewMatrix();

// Subsequent calls return the cached matrix
const Math::Mat4& sameViewMatrix = camera.GetViewMatrix();
```

### Frustum Culling Optimization
```cpp
// Efficient frustum culling for scene rendering
void RenderScene(const std::vector<std::shared_ptr<RenderObject>>& objects, const Camera& camera) {
    u32 visibleCount = 0;
    u32 culledCount = 0;
    
    for (const auto& object : objects) {
        // Get object's bounding sphere
        Math::Vec3 center = object->GetPosition();
        f32 radius = object->GetBoundingRadius();
        
        // Check visibility
        if (camera.IsSphereVisible(center, radius)) {
            // Object is visible, render it
            object->Render();
            visibleCount++;
        } else {
            // Object is not visible, skip rendering
            culledCount++;
        }
    }
    
    PYRAMID_LOG_INFO("Rendered: ", visibleCount, ", Culled: ", culledCount);
}
```

## Integration with Other Systems

### Integration with Math Library
```cpp
// Camera works seamlessly with math library types
Math::Vec3 position = Math::Vec3(10.0f, 5.0f, -20.0f);
Math::Quat rotation = Math::Quat::FromEuler(Math::Radians(30.0f), Math::Radians(-45.0f), 0.0f);
Math::Mat4 worldMatrix = Math::Mat4::CreateTRS(position, rotation, Math::Vec3::One);

camera.SetPosition(position);
camera.SetRotation(rotation);

// Use camera matrices with math operations
Math::Mat4 mvp = camera.GetViewProjectionMatrix() * worldMatrix;
```

### Integration with GraphicsDevice
```cpp
// Use camera matrices with GraphicsDevice
void RenderFrame(GraphicsDevice& device, const Camera& camera) {
    // Clear screen
    device.Clear(Color(0.1f, 0.1f, 0.1f, 1.0f));
    
    // Set up rendering state
    shader->bind();
    shader->setUniform("u_ViewProjection", camera.GetViewProjectionMatrix());
    shader->setUniform("u_CameraPosition", camera.GetPosition());
    
    // Render visible objects
    auto visibleObjects = GetVisibleObjects(camera);
    for (const auto& object : visibleObjects) {
        object->Render(device, shader);
    }
    
    // Present frame
    device.Present(true);
}
```

## Best Practices

### Camera Management
1. **Reuse Cameras**: Create cameras once and reuse them rather than creating new ones each frame
2. **Update Order**: Update camera position before accessing matrices for rendering
3. **Projection Settings**: Set appropriate near/far planes for your scene scale
4. **Aspect Ratio**: Update aspect ratio when window is resized

### Performance Optimization
1. **Frustum Culling**: Always use frustum culling for large scenes
2. **Matrix Access**: Minimize matrix access calls when possible (cache matrices locally)
3. **Camera Types**: Use orthographic projection for 2D/UI elements
4. **LOD Systems**: Combine camera distance with Level of Detail systems

### Visual Quality
1. **Field of View**: Use appropriate FOV values (typically 45-75 degrees for perspective)
2. **Near/Far Planes**: Balance depth precision with scene requirements
3. **Camera Movement**: Implement smooth camera movement for better user experience
4. **Multiple Cameras**: Use different cameras for different viewports (main view, minimap, etc.)

## Conclusion

The Pyramid Camera class provides a comprehensive camera system that supports both perspective and orthographic projections, includes efficient frustum culling, and offers various movement and orientation utilities. Its integration with the math library and graphics device makes it an essential component for any 3D application built with the Pyramid Engine.