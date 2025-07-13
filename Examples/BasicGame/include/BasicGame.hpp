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
#include <memory>
#include <vector>
#include <string>
#include <chrono>

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
    void CreateManyObjects(); // For spatial partitioning demonstration

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
    void LogPerformanceMetrics();

    // Enhanced rendering system
    std::unique_ptr<Pyramid::Renderer::RenderSystem> m_renderSystem;
    std::shared_ptr<Pyramid::IShader> m_shader;
    std::shared_ptr<Pyramid::IVertexArray> m_vertexArray;

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

    // Input state
    bool m_keys[256] = {false};
    float m_lastMouseX = 0.0f;
    float m_lastMouseY = 0.0f;
    bool m_firstMouse = true;
};
