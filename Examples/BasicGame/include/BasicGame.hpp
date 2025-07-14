#pragma once
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Buffer/VertexArray.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Scene/SceneManager.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLFramebuffer.hpp>
#include <Pyramid/Graphics/Buffer/InstanceBuffer.hpp>
#include <Pyramid/Graphics/Buffer/ShaderStorageBuffer.hpp>
#include <Pyramid/Graphics/OpenGL/OpenGLStateManager.hpp>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <functional>
#include <array>

// Enhanced vertex structure for PBR rendering
struct Vertex
{
    float Position[3];
    float Normal[3];
    float TexCoord[2];
    float Color[3];
};

/**
 * @brief Enhanced BasicGame demonstrating all advanced engine systems
 *
 * This comprehensive example showcases:
 * - SIMD-optimized math operations in real-time
 * - Enhanced rendering system with command buffers and render passes
 * - Advanced camera system with smooth movement and frustum culling
 * - Scene graph with hierarchical transforms and multiple objects
 * - PBR material system with proper lighting calculations
 * - Performance monitoring showing SIMD acceleration benefits
 * - Scene Management Core Architecture with Octree spatial partitioning
 * - Framebuffer Objects (FBO) for render-to-texture and post-processing
 * - Spatial queries for efficient object management and culling
 * - OpenGL 4.6 Advanced Features:
 *   * Instanced rendering for efficient multi-object rendering
 *   * Geometry shaders for procedural geometry generation
 *   * Tessellation shaders for adaptive mesh refinement
 *   * Compute shaders with SSBOs for GPU-accelerated computations
 *   * Comprehensive state management with performance monitoring
 */
class BasicGame : public Pyramid::Game
{
public:
    explicit BasicGame();
    virtual ~BasicGame() = default;

protected:
    void onCreate() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    // Input handling methods
    void HandleInput(float deltaTime);

private:
    // Initialization methods
    void InitializeRenderSystem();
    void InitializeScene();
    void InitializeCamera();
    void LoadTestTextures();
    void CreateSampleTextures();
    void SetupUniformBuffers();
    void InitializeSceneManagement();
    void InitializeFramebuffers();
    void CreateManyObjects();          // For spatial partitioning demonstration
    void InitializeAdvancedFeatures(); // OpenGL 4.6 features
    void SetupInstancedRendering();
    void SetupAdvancedShaders();
    void SetupComputeShaders();

    // Update methods
    void UpdateCamera(float deltaTime);
    void UpdateScene(float deltaTime);
    void UpdateUniformBuffers(float deltaTime);
    void UpdatePerformanceMetrics(float deltaTime);

    // Demonstration methods
    void DemonstrateSIMDOperations();
    void DemonstrateSceneGraph();
    void DemonstrateFrustumCulling();
    void DemonstrateSceneManagement();
    void DemonstrateFramebuffers();
    void DemonstrateSpatialQueries();
    void DemonstrateInstancedRendering();
    void DemonstrateAdvancedShaders();
    void DemonstrateComputeShaders();
    void DemonstrateStateManagement();
    void LogPerformanceMetrics();

    // Enhanced rendering system
    std::unique_ptr<Pyramid::Renderer::RenderSystem> m_renderSystem;
    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;

    // OpenGL 4.6 Advanced Features
    // Instanced rendering
    std::shared_ptr<Pyramid::IShader> m_instancedShader;
    std::shared_ptr<Pyramid::IVertexArray> m_instancedVertexArray;
    std::shared_ptr<Pyramid::IInstanceBuffer> m_instanceBuffer;
    static constexpr int INSTANCE_COUNT = 1000;

    // Advanced shaders
    std::shared_ptr<Pyramid::IShader> m_geometryShader;
    std::shared_ptr<Pyramid::IShader> m_tessellationShader;
    std::shared_ptr<Pyramid::IShader> m_computeShader;

    // Compute shader resources
    std::shared_ptr<Pyramid::IShaderStorageBuffer> m_computeInputSSBO;
    std::shared_ptr<Pyramid::IShaderStorageBuffer> m_computeOutputSSBO;
    static constexpr int COMPUTE_DATA_SIZE = 1024;

    // Advanced camera system
    std::unique_ptr<Pyramid::Camera> m_camera;
    Pyramid::Math::Vec3 m_cameraVelocity = Pyramid::Math::Vec3::Zero;
    float m_cameraSpeed = 5.0f;
    float m_mouseSensitivity = 0.002f;

    // Scene management
    std::shared_ptr<Pyramid::Scene> m_scene;
    std::vector<std::shared_ptr<Pyramid::SceneNode>> m_sceneNodes;
    std::vector<std::shared_ptr<Pyramid::RenderObject>> m_renderObjects;

    // Scene Management Core Architecture
    std::unique_ptr<Pyramid::SceneManagement::SceneManager> m_sceneManager;
    std::shared_ptr<Pyramid::Scene> m_managedScene;

    // Framebuffer Objects for render-to-texture
    std::unique_ptr<Pyramid::OpenGLFramebuffer> m_mainFBO;
    std::unique_ptr<Pyramid::OpenGLFramebuffer> m_postProcessFBO;
    std::unique_ptr<Pyramid::OpenGLFramebuffer> m_shadowMapFBO;

    // OpenGL 4.6 Uniform Buffer Objects for efficient data transfer
    std::shared_ptr<Pyramid::IUniformBuffer> m_sceneUBO;
    std::shared_ptr<Pyramid::IUniformBuffer> m_materialUBO;

    // Enhanced uniform data structures (std140 layout)
    struct SceneUniforms
    {
        Pyramid::Math::Mat4 viewMatrix;
        Pyramid::Math::Mat4 projectionMatrix;
        Pyramid::Math::Mat4 viewProjectionMatrix; // Pre-calculated for efficiency
        Pyramid::Math::Vec4 cameraPosition;
        Pyramid::Math::Vec4 cameraDirection;
        Pyramid::Math::Vec4 lightDirection;
        Pyramid::Math::Vec4 lightColor;
        float time;
        float nearPlane;
        float farPlane;
        float padding; // Ensure 16-byte alignment
    };

    // PBR Material uniform data structure (std140 layout)
    struct MaterialUniforms
    {
        Pyramid::Math::Vec4 baseColor;
        Pyramid::Math::Vec4 emissiveColor;
        float metallic;
        float roughness;
        float ao; // Ambient occlusion
        float textureScale;
    };

    // Multiple textures to showcase different image formats
    std::vector<std::shared_ptr<Pyramid::ITexture2D>> m_textures;
    std::vector<std::string> m_textureNames;
    std::vector<std::string> m_textureFormats;

    // Animation and timing
    int m_currentTextureIndex = 0;
    float m_time = 0.0f;
    float m_textureSwapTimer = 0.0f;
    const float m_textureSwapInterval = 3.0f; // Switch texture every 3 seconds

    // Performance monitoring
    struct PerformanceMetrics
    {
        float frameTime = 0.0f;
        float simdTime = 0.0f;
        float scalarTime = 0.0f;
        float renderTime = 0.0f;
        int visibleObjects = 0;
        int culledObjects = 0;
        float simdSpeedup = 1.0f;

        // Scene Management metrics
        int totalObjects = 0;
        int octreeNodes = 0;
        float lastQueryTime = 0.0f;
        float spatialQuerySpeedup = 1.0f;

        // OpenGL 4.6 Advanced Features metrics
        int instancedObjects = 0;
        float instancedRenderTime = 0.0f;
        float computeShaderTime = 0.0f;
        int stateChanges = 0;
        float geometryShaderTime = 0.0f;
        float tessellationTime = 0.0f;
    };
    PerformanceMetrics m_metrics;
    float m_metricsUpdateTimer = 0.0f;
    const float m_metricsUpdateInterval = 1.0f; // Update metrics every second

    // SIMD demonstration data
    static constexpr int SIMD_TEST_SIZE = 1000;
    std::vector<Pyramid::Math::Vec4> m_testVectors;
    std::vector<Pyramid::Math::Vec4> m_testResults;
    std::vector<Pyramid::Math::Mat4> m_testMatrices;

    // Scene animation
    float m_sceneAnimationTime = 0.0f;
    bool m_enableFrustumCulling = true;
    bool m_showPerformanceOverlay = true;

    // Advanced features demonstration control
    bool m_enableInstancedRendering = true;
    bool m_enableGeometryShader = true;
    bool m_enableTessellation = true;
    bool m_enableComputeShader = true;
    int m_currentDemoMode = 0; // 0=basic, 1=instanced, 2=geometry, 3=tessellation, 4=compute

    // Instance data for instanced rendering
    struct InstanceData
    {
        Pyramid::Math::Vec4 position; // xyz = position, w = scale
        Pyramid::Math::Vec4 rotation; // quaternion
        Pyramid::Math::Vec4 color;    // rgba color
    };
    std::vector<InstanceData> m_instanceData;

    // Input state
    bool m_keys[256] = {false};
    float m_lastMouseX = 0.0f;
    float m_lastMouseY = 0.0f;
    bool m_firstMouse = true;

    // Enhanced 3D Scene Methods
    void CreateEnhancedScene();
    void LoadSceneTextures();
    void SetupAdvancedMaterials();
    void SetupDynamicLighting();
    void UpdateSceneAnimations(float deltaTime);
    void RenderEnhancedScene();

    // Geometric Primitive Generation
    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<Pyramid::u32> indices;
    };

    MeshData GenerateSphere(float radius = 1.0f, int segments = 32, int rings = 16);
    MeshData GenerateCylinder(float radius = 1.0f, float height = 2.0f, int segments = 32);
    MeshData GeneratePlane(float width = 2.0f, float height = 2.0f, int subdivisions = 1);
    MeshData GenerateTorus(float majorRadius = 1.0f, float minorRadius = 0.3f, int majorSegments = 32, int minorSegments = 16);
    MeshData GenerateIcosphere(float radius = 1.0f, int subdivisions = 2);

    // Enhanced Scene Objects
    struct SceneObject
    {
        std::string name;
        MeshData mesh;
        std::shared_ptr<Pyramid::IVertexArray> vertexArray;
        std::shared_ptr<Pyramid::ITexture2D> diffuseTexture;
        std::shared_ptr<Pyramid::ITexture2D> normalTexture;
        std::shared_ptr<Pyramid::ITexture2D> metallicRoughnessTexture;
        Pyramid::Math::Vec3 position;
        Pyramid::Math::Vec3 rotation;
        Pyramid::Math::Vec3 scale;
        Pyramid::Math::Vec4 baseColor;
        float metallic;
        float roughness;
        float animationSpeed;
        float animationOffset;
    };

    std::vector<SceneObject> m_sceneObjects;

    // Enhanced lighting system
    struct Light
    {
        Pyramid::Math::Vec3 position;
        Pyramid::Math::Vec3 direction;
        Pyramid::Math::Vec3 color;
        float intensity;
        int type; // 0=directional, 1=point, 2=spot
        float range;
        float innerCone;
        float outerCone;
    };

    std::vector<Light> m_lights;

    // Camera enhancement
    enum class CameraMode
    {
        Static,
        Orbital,
        Cinematic,
        FreeRoam
    };

    CameraMode m_cameraMode = CameraMode::Orbital;
    float m_cameraOrbitRadius = 8.0f;
    float m_cameraOrbitSpeed = 0.3f;
    float m_cameraHeight = 2.0f;
    Pyramid::Math::Vec3 m_cameraTarget = Pyramid::Math::Vec3::Zero;

    // Free roam camera controls
    void UpdateFreeRoamCamera(float deltaTime);
    void ProcessKeyboardInput(float deltaTime);
    void ProcessMouseInput(float deltaTime);

    // Free roam camera state
    Pyramid::Math::Vec3 m_cameraPosition = Pyramid::Math::Vec3::Zero;
    Pyramid::Math::Vec3 m_cameraFront = Pyramid::Math::Vec3(0.0f, 0.0f, -1.0f);
    Pyramid::Math::Vec3 m_cameraUp = Pyramid::Math::Vec3(0.0f, 1.0f, 0.0f);
    Pyramid::Math::Vec3 m_cameraRight = Pyramid::Math::Vec3::Zero;

    float m_cameraYaw = -90.0f; // Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right
    float m_cameraPitch = 0.0f;
    bool m_freeRoamEnabled = false;
};
