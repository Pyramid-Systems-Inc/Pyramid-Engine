# API overview

Public headers under `Engine/*/include` are the signature-level source of truth. `PYRAMID_VERSION_STRING` is exported as a target compile definition and currently evaluates to `0.6.0-pre-alpha`.

## Application

Header: `Pyramid/Core/Game.hpp`

```cpp
class Game
{
public:
    explicit Game(GraphicsAPI api = GraphicsAPI::OpenGL);
    void run();
    void quit();
    bool IsInitialized() const;

protected:
    virtual void onCreate();
    virtual void onUpdate(float deltaTime);
    virtual void onRender();
    virtual void onWindowResize(const WindowResizeEvent& event);
    IGraphicsDevice* GetGraphicsDevice() const;
    const InputState& GetInput() const;
    void SetActiveCamera(Camera* camera);
    Camera* GetActiveCamera() const;
    void SetRenderSystem(Renderer::RenderSystem* renderSystem);
    Renderer::RenderSystem* GetRenderSystem() const;
    bool IsRenderSurfaceAvailable() const;
};
```

Only `GraphicsAPI::OpenGL` is supported. A derived `onCreate()` must call `Game::onCreate()` first.

## Window

Header: `Pyramid/Platform/Window.hpp`

The window interface requires initialization, presentation, context activation, message processing, close-state reporting, title/size/position/visibility mutation, and minimized/maximized queries. The checked-in implementation is `Win32OpenGLWindow`.

`Window::SetResizeCallback()` receives platform-neutral `WindowResizeEvent` values while `ProcessMessages()` dispatches native messages. Each event includes client width, client height, and a `Restored`, `Minimized`, or `Maximized` state. `WindowResizeEvent::HasRenderableArea()` is false for minimized or zero-sized windows. `Game` installs this callback, updates the default graphics viewport, synchronizes the registered active camera and render system, suspends rendering for non-renderable client areas, and then forwards the event to the overridable `onWindowResize()` hook.

```cpp
void MyGame::onWindowResize(const Pyramid::WindowResizeEvent& event)
{
    Game::onWindowResize(event);
    if (!event.HasRenderableArea())
        return;

    // Default viewport, the camera registered with SetActiveCamera(), and the
    // RenderSystem registered with SetRenderSystem() are already synchronized.
    // Resize standalone framebuffers here.
}
```

## Input

Header: `Pyramid/Platform/Input.hpp`

`Win32OpenGLWindow` converts native keyboard, mouse-button, pointer, wheel, and focus messages into one platform-neutral `InputState`. `Window::ProcessMessages()` starts a new input frame before dispatching messages, so transitions and deltas are valid during the immediately following `Game::onUpdate()`.

```cpp
void MyGame::onUpdate(float deltaTime)
{
    Game::onUpdate(deltaTime);
    const auto& input = GetInput();

    if (input.WasKeyPressed(Pyramid::Key::Escape))
        quit();

    if (input.IsKeyDown(Pyramid::Key::W))
        MoveForward(deltaTime);

    if (input.IsMouseButtonDown(Pyramid::MouseButton::Right))
    {
        const auto delta = input.GetMouseDelta();
        RotateCamera(delta.x, delta.y);
    }

    ZoomCamera(input.GetMouseWheelDelta());
}
```

Queries include held, pressed-this-frame, and released-this-frame states for keyboard and mouse buttons; client-space pointer position; aggregated pointer movement; and vertical/horizontal wheel steps. Native key-repeat messages do not retrigger `WasKeyPressed()`. Losing focus releases every held key/button and resets the pointer baseline, preventing stuck controls or a large mouse jump after focus returns.

Input is currently a direct polling layer. Action names, rebinding, chords, device abstraction, text input, raw relative mouse mode, and controllers are future work.

## Graphics device and resources

Primary headers:

- `Pyramid/Graphics/GraphicsDevice.hpp`
- `Pyramid/Graphics/Buffer/*.hpp`
- `Pyramid/Graphics/Shader/Shader.hpp`
- `Pyramid/Graphics/Shader/ShaderProgram.hpp`
- `Pyramid/Graphics/Shader/ShaderCache.hpp`
- `Pyramid/Graphics/Texture.hpp`
- `Pyramid/Graphics/Texture/TextureResource.hpp`
- `Pyramid/Graphics/Texture/TextureCache.hpp`
- `Pyramid/Graphics/Geometry/Vertex.hpp`
- `Pyramid/Graphics/Geometry/Mesh.hpp`
- `Pyramid/Graphics/Geometry/MeshCache.hpp`
- `Pyramid/Graphics/Material/Material.hpp`
- `Pyramid/Graphics/Material/MaterialCache.hpp`
- `Pyramid/Graphics/Resources/ResourceHandle.hpp`
- `Pyramid/Graphics/Resources/ResourceManifest.hpp`
- `Pyramid/Graphics/Resources/ResourceRegistry.hpp`
- `Pyramid/Graphics/Scene/SceneSerializer.hpp`

### Shader programs

```cpp
Pyramid::ShaderProgramSpecification shaderSpec;
shaderSpec.vertexSource = vertexSource;
shaderSpec.fragmentSource = fragmentSource;
shaderSpec.name = "Player Forward";
shaderSpec.assetId =
    Pyramid::ShaderAssetId::FromString("shaders/player-forward");

Pyramid::ShaderCache shaderCache(*device);
auto shader = shaderCache.GetOrCreate(shaderSpec);
```

`ShaderProgram` is an immutable compiled resource that implements `IShader`, so it can be assigned directly to materials and command buffers. Exact stage-source sets receive a deterministic content identifier and compile only once even when requested through several caller-defined stable aliases. Graphics specifications require vertex and fragment stages; tessellation control/evaluation stages must be paired; compute programs cannot mix with graphics stages. Debug names are excluded from content identity.

Changing a stable asset uses transactional replacement:

```cpp
Pyramid::ShaderProgramSpecification replacement = shaderSpec;
replacement.fragmentSource = updatedFragmentSource;
replacement.assetId = {};

if (shaderCache.Recompile(shaderSpec.assetId, replacement))
    shader = shaderCache.Find(shaderSpec.assetId);
```

`Recompile()` compiles or resolves the replacement before changing the stable alias. Failure leaves the previous cached program active. Existing external owners of the old program remain valid and must reacquire the stable alias when they want the replacement. Content-derived identifiers are immutable and cannot be recompiled. `Evict()`, `CollectUnused()`, `Clear()`, and `GetStats()` provide explicit lifetime and diagnostics controls. Destroy the cache before its graphics device/context and use it from the graphics thread.

### Geometry

Typical geometry setup:

```cpp
auto* device = GetGraphicsDevice();

Pyramid::MeshSpecification meshSpec;
meshSpec.vertexData = vertexData;
meshSpec.vertexDataSize = vertexBytes;
meshSpec.vertexCount = vertexCount;
meshSpec.layout = {
    {Pyramid::ShaderDataType::Float3, "a_Position"},
    {Pyramid::ShaderDataType::Float4, "a_Color"}
};
meshSpec.indexData = indexData;
meshSpec.indexCount = indexCount;
meshSpec.topology = Pyramid::PrimitiveTopology::Triangles;
meshSpec.name = "PlayerMesh";
meshSpec.assetId = Pyramid::MeshAssetId::FromString("meshes/player");

Pyramid::MeshCache meshCache(*device);
auto mesh = meshCache.GetOrCreate(meshSpec);
renderObject->mesh = mesh;
```

`Mesh` owns the created vertex array, vertex buffer, and optional index buffer. Its validated layout, vertex/index counts, primitive topology, identifiers, and local bounds are immutable after creation. Indexed and non-indexed meshes are supported for points, lines, line strips, triangles, and triangle strips. Creation rejects mismatched byte counts, missing/invalid position semantics, non-finite positions, incompatible topology counts, and out-of-range indices.

`MeshAssetId::FromString()` creates a deterministic 128-bit identifier from a stable caller-owned name such as an asset path. If `MeshSpecification::assetId` is left invalid, `Mesh::CalculateContentId()` derives the asset identifier from the exact geometry bytes and immutable draw metadata. Debug names are excluded, so renaming a mesh does not create another GPU upload.

`MeshCache` is bound to one graphics device and owns one strong reference per unique content fingerprint. Requests through different stable IDs share the same resident mesh when their geometry is identical. Reusing a resident explicit ID for different geometry fails rather than silently returning the wrong resource. `Find()` resolves either a stable alias or the content ID. `Evict()` removes the mesh and all aliases from the cache while existing external `shared_ptr` owners remain valid; `CollectUnused()` removes resources owned only by the cache, and `Clear()` removes all cache ownership. `GetStats()` reports residency, bytes, hits, misses, conflicts, failures, creations, and evictions. Destroy the cache before its graphics device/context and call it from the graphics thread. Direct `Mesh::Create()` remains available for intentionally uncached one-off geometry.

### Textures

```cpp
Pyramid::TextureSpecification spec;
spec.Width = 512;
spec.Height = 512;
spec.Format = Pyramid::TextureFormat::RGBA8;
spec.GenerateMips = false;

auto texture = Pyramid::ITexture2D::Create(spec, pixels);
auto blank = Pyramid::ITexture2D::Create(512, 512);
auto target = Pyramid::ITexture2D::CreateRenderTarget(1280, 720);
auto white = Pyramid::ITexture2D::CreateFromColor(1, 1, Pyramid::Color::White);
```

For shared sampled textures, prefer the immutable cache path:

```cpp
Pyramid::TextureCache textureCache(*device);

Pyramid::TextureFileSpecification fileSpec;
fileSpec.filepath = "Assets/albedo.png";
fileSpec.colorSpace = Pyramid::TextureColorSpace::SRGB;
fileSpec.assetId = Pyramid::TextureAssetId::FromString("textures/player/albedo");

auto albedo = textureCache.GetOrCreate(fileSpec);
```

`TextureCache` fingerprints exact decoded pixels plus dimensions, format, mip policy, sampler state, border color, anisotropy request, and explicit linear/sRGB intent. Identical memory or file requests share one GPU upload across aliases. Reusing one stable ID for different resident content fails. `Reload()` is transactional for caller-defined file aliases: a replacement is decoded and uploaded before the alias changes; failure preserves the previous resource. `Evict()`, `CollectUnused()`, `Clear()`, and `GetStats()` provide explicit residency control. Cached `TextureResource` objects are immutable; reacquire the stable alias after a successful reload. Destroy the cache before the graphics device/context.

The direct `ITexture2D` path remains available for intentionally uncached or mutable textures. RGB8/RGBA8 files support JPEG/PNG/TGA/BMP. `IsLoaded()` and `GetLastError()` expose state, RGB rows use safe unpack alignment, sRGB uploads select sRGB internal formats, and mipmapped filters are mapped completely. Cached file specifications can flip decoded rows before upload; direct `TextureSpecification::FlipY` and anisotropic filtering are not yet applied consistently by every path. `CreateDepthTarget` returns `nullptr`; use `OpenGLFramebuffer` for depth attachments.

### Materials

```cpp
Pyramid::MaterialSpecification materialSpec;
materialSpec.shader = shader;
materialSpec.textures = {
    {"u_AlbedoMap", 0, albedo}
};
materialSpec.uniforms = {
    {"u_AlbedoColor", Pyramid::Math::Vec4(1.0f)},
    {"u_Metallic", 0.0f},
    {"u_Roughness", 0.5f}
};
materialSpec.renderState.blendMode = Pyramid::MaterialBlendMode::Opaque;
materialSpec.renderState.depthTest = true;
materialSpec.renderState.cullMode = Pyramid::MaterialCullMode::Back;
materialSpec.assetId =
    Pyramid::MaterialAssetId::FromString("materials/player");

Pyramid::MaterialCache materialCache;
auto material = materialCache.GetOrCreate(materialSpec);
renderObject->material = material;
```

`Material` is immutable and owns exact references to one graphics `ShaderProgram` and zero or more `TextureResource` objects. Its content identity includes shader and texture content IDs, sampler uniform names and slots, typed uniforms, and blend/depth/cull/polygon state while excluding the debug name. Creation rejects compute shaders, duplicate slots or names, unloaded textures, and non-finite values. `CommandBuffer::SetMaterial()` applies the material in draw order; dynamic object/camera matrices are recorded separately with typed `SetUniform*()` commands so they do not alter material identity.

`MaterialCache` keeps one strong resident reference per exact material content fingerprint. Different stable IDs share one `Material` when shader, textures, uniforms, and fixed state are identical. Reusing one stable ID for different resident content is rejected. `Find()`, `Evict()`, `CollectUnused()`, `Clear()`, and `GetStats()` provide explicit lookup, lifetime, and diagnostics control. Destroy the material cache before the shader and texture caches that own its referenced resources.

Stable aliases can be replaced transactionally:

```cpp
Pyramid::MaterialSpecification replacement = materialSpec;
replacement.uniforms = {
    {"u_AlbedoColor", Pyramid::Math::Vec4(0.8f, 0.9f, 1.0f, 1.0f)},
    {"u_Metallic", 0.2f},
    {"u_Roughness", 0.35f}
};
replacement.assetId = {};

if (materialCache.Replace(materialSpec.assetId, replacement))
    material = materialCache.Find(materialSpec.assetId);
```

`Replace()` validates or resolves the complete replacement before changing the stable alias. Failure preserves the previously active material, while existing external owners of older material versions remain valid. Content-derived identifiers are immutable and cannot be replaced.


### Resource registry

`Game` owns one `ResourceRegistry` for its graphics device and destroys it before device shutdown:

```cpp
auto* resources = GetResourceRegistry();
if (!resources)
    return;

auto mesh = resources->Meshes().GetOrCreate(meshSpec);
auto shader = resources->Shaders().GetOrCreate(shaderSpec);
auto texture = resources->Textures().GetOrCreate(textureSpec);
auto material = resources->Materials().GetOrCreate(materialSpec);
```

The registry is the preferred application-level entry point for reusable graphics assets. It guarantees dependency-safe teardown and maintenance ordering: materials first, then textures, shaders, and meshes. `CollectUnused()` removes cache-only resources while preserving dependencies referenced by externally owned materials. `Clear()` removes every cached alias/resource without invalidating external `shared_ptr` owners. Those external owners must still be released before the graphics device/context. `GetStats()` aggregates all four cache snapshots and reports estimated total resident bytes.

For scene, serialization, and long-lived asset references, acquire typed non-owning handles instead of retaining cache identifiers or resource owners:

```cpp
auto meshHandle = resources->AcquireMesh(meshSpec);
auto materialHandle = resources->AcquireMaterial(materialSpec);

if (auto mesh = resources->Resolve(meshHandle))
{
    // The alias still exists and its generation matches the handle.
}

if (!resources->IsAlive(materialHandle))
{
    // The resource was evicted, collected, cleared, or replaced.
}
```

`MeshHandle`, `ShaderHandle`, `TextureHandle`, and `MaterialHandle` contain a stable asset identifier and a non-zero alias generation. They are small serializable value types and do not keep GPU resources alive. Alias creation, remapping, eviction, collection, and clearing advance persistent generation tombstones; therefore a stale handle never resolves to a later resource that reused the same stable ID. `FromParts()` reconstructs a handle from serialized ID/generation data.

Mutations return a new current-generation handle and leave the previous handle stale when content changes:

```cpp
shaderHandle = resources->RecompileShader(shaderHandle, replacementShader);
textureHandle = resources->ReloadTexture(textureHandle);
materialHandle = resources->ReplaceMaterial(materialHandle, replacementMaterial);
```

Handle-backed scene objects avoid long-lived resource ownership while retaining mesh bounds for culling:

```cpp
auto object = std::make_shared<Pyramid::RenderObject>();
object->SetMeshHandle(meshHandle, *resources);
object->SetMaterialHandle(materialHandle, *resources);
```

Register the render system through `Game::SetRenderSystem()` so render passes receive the same registry and resolve handles at submission time. A stale mesh or material handle causes that object to be skipped rather than bound to unrelated replacement content. Assigning a newly issued mesh handle refreshes the cached local bounds.

Standalone cache construction remains available for tooling and focused tests, but a registry must always be destroyed before its graphics device and native context.

### Resource manifests

`ResourceManifest` serializes a named set of typed handles without a JSON dependency. The current format is deterministic and versioned:

```text
PYRAMID_RESOURCE_MANIFEST	1
mesh	player.mesh	0123456789abcdef0123456789abcdef	4
material	player.material	fedcba9876543210fedcba9876543210	2
```

```cpp
Pyramid::ResourceManifest manifest;
manifest.Add("player.mesh", meshHandle);
manifest.Add("player.material", materialHandle);

const std::string serialized = manifest.Serialize();

Pyramid::ResourceManifest restored;
std::vector<Pyramid::ResourceManifestDiagnostic> diagnostics;
if (!Pyramid::ResourceManifest::Deserialize(serialized, restored, diagnostics))
{
    // Unsupported versions and malformed entries leave `restored` unchanged.
}

auto restoredMesh = restored.GetMeshHandle("player.mesh");
const auto report = restored.Restore(*resources);
```

Manifest keys are caller-owned stable reference names containing letters, digits, `.`, `_`, `-`, or `/`. The serialized 128-bit asset ID is restored exactly; it is not re-hashed from a path. `Restore()` validates the current registry and reports `MissingAsset` separately from `StaleGeneration`. It never updates a serialized generation automatically.

### Scene

### Entities and components

```cpp
Pyramid::Scene scene("Battlefield");

auto army = scene.CreateEntity("Army");
auto unit = scene.CreateEntity("Infantry");
unit.SetParent(army);
unit.SetLocalPosition({2.0f, 0.0f, 4.0f});

Pyramid::MeshRendererComponent renderer;
renderer.mesh = meshHandle;
renderer.material = materialHandle;
unit.SetMeshRenderer(renderer, &registry);

auto sun = scene.CreateEntity("Sun");
Pyramid::LightComponent light;
light.type = Pyramid::LightType::Directional;
light.intensity = 4.0f;
sun.SetLight(light);
scene.SetPrimaryLight(sun);
```

Every entity has a stable `EntityId`, name, visibility flag, and mandatory `TransformComponent`. Optional `MeshRendererComponent` and `LightComponent` values provide rendering state. `Entity` is a lightweight non-owning facade; it becomes invalid after its scene or entity is destroyed.

```cpp
const Pyramid::EntityId id = unit.GetId();
auto restored = scene.FindEntity(id);
auto children = army.GetChildren();
auto world = unit.GetWorldMatrix();
```

Hierarchy edits reject missing entities, cycles, and self-parenting. Reparenting preserves local TRS. World matrices and inherited visibility are recalculated from the authoritative hierarchy.

### Renderer proxies

`Scene::GetRenderObjects()` and `Scene::GetLights()` return generated proxies used by render passes, culling, and the octree. Do not author scene state by mutating those proxies. New code should edit entities/components and request the proxies again. Transitional `AddRenderObject()` and `AddLight()` helpers exist only for low-level compatibility.

### Spatial management

`SceneManager` synchronizes entity-generated render proxies into the octree. Point, sphere, box, ray, nearest, and K-nearest queries operate on complete world AABBs. `QueryResult::entities` identifies the owning scene entities.

## Math

Umbrella header: `Pyramid/Math/Math.hpp`

```cpp
using namespace Pyramid::Math;

Vec3 position(1.0f, 2.0f, 3.0f);
Mat4 model = Mat4::CreateTranslation(position)
           * Mat4::CreateRotationY(Radians(30.0f));
```

Concrete headers define exact conventions and available operations for vectors, matrices, quaternions, geometry, interpolation, and SIMD helpers.

## Images

Header: `Pyramid/Util/Image.hpp`

```cpp
auto image = Pyramid::Util::Image::Load("assets/texture.png");
if (image.Data)
{
    // Consume image.Width, image.Height, image.Channels, and image.Data.
    Pyramid::Util::Image::Free(image.Data);
}
```

PNG uses the engine's custom non-interlaced decoder. JPEG uses libjpeg-turbo and is tested with baseline RGB and progressive grayscale fixtures; output is normalized to tightly packed 8-bit RGB. `ImageData::Data` remains manually owned and must be released with `Image::Free()`.

## Logging

Header: `Pyramid/Util/Log.hpp`

```cpp
Pyramid::Util::LoggerConfig config;
config.consoleLevel = Pyramid::Util::LogLevel::Info;
config.fileLevel = Pyramid::Util::LogLevel::Debug;
config.logFilePath = "pyramid_game.log";
Pyramid::Util::Logger::GetInstance().Configure(config);

PYRAMID_LOG_INFO("Loaded ", objectCount, " objects");
PYRAMID_ASSERT(resource != nullptr, "Resource must exist");
```
