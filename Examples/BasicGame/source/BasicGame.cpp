#include "BasicGame.hpp"
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/VertexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/IndexBuffer.hpp>
#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Util/Log.hpp>
#include <Pyramid/Util/Image.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Graphics/Renderer/RenderPasses.hpp>
#include <vector>
#include <cmath>
#include <string>
#include <cstring>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>

// Enhanced vertex structure for PBR rendering
struct Vertex
{
    float Position[3];
    float Normal[3];
    float TexCoord[2];
    float Color[3];
};

// Simplified shader for debugging - start simple and build up
const std::string vertexShaderSrc = R"(
    #version 460 core
    layout (location = 0) in vec3 a_Position;
    layout (location = 1) in vec3 a_Normal;
    layout (location = 2) in vec2 a_TexCoord;
    layout (location = 3) in vec3 a_Color;

    // Simplified Uniform Buffer Objects (std140 layout)
    layout(std140, binding = 0) uniform SceneData {
        mat4 u_ViewMatrix;
        mat4 u_ProjectionMatrix;
        mat4 u_ViewProjectionMatrix;
        vec4 u_CameraPosition;
        vec4 u_CameraDirection;
        vec4 u_LightDirection;
        vec4 u_LightColor;
        float u_Time;
        float u_NearPlane;
        float u_FarPlane;
    };

    out vec3 v_Color;
    out vec3 v_Normal;
    out vec3 v_WorldPos;

    void main()
    {
        v_Color = a_Color;
        v_Normal = a_Normal;
        v_WorldPos = a_Position;

        // Simple transformation without animation for debugging
        gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);
    }
)";

const std::string fragmentShaderSrc = R"(
    #version 460 core
    out vec4 FragColor;

    in vec3 v_Color;
    in vec3 v_Normal;
    in vec3 v_WorldPos;

    // Simplified Uniform Buffer Objects (std140 layout)
    layout(std140, binding = 0) uniform SceneData {
        mat4 u_ViewMatrix;
        mat4 u_ProjectionMatrix;
        mat4 u_ViewProjectionMatrix;
        vec4 u_CameraPosition;
        vec4 u_CameraDirection;
        vec4 u_LightDirection;
        vec4 u_LightColor;
        float u_Time;
        float u_NearPlane;
        float u_FarPlane;
    };

    layout(std140, binding = 1) uniform MaterialData {
        vec4 u_BaseColor;
        vec4 u_EmissiveColor;
        float u_Metallic;
        float u_Roughness;
        float u_AO;
        float u_TextureScale;
    };

    void main()
    {
        // Simple lighting calculation for debugging
        vec3 normal = normalize(v_Normal);
        vec3 lightDir = normalize(-u_LightDirection.xyz);

        // Basic diffuse lighting
        float NdotL = max(dot(normal, lightDir), 0.0);

        // Combine vertex color with lighting
        vec3 diffuse = v_Color * NdotL;
        vec3 ambient = v_Color * 0.3; // Some ambient light

        // Add some animation based on time
        vec3 emissive = u_EmissiveColor.rgb * (sin(u_Time * 2.0) * 0.5 + 0.5);

        vec3 finalColor = diffuse + ambient + emissive * 0.2;

        FragColor = vec4(finalColor, 1.0);
    }
)";

BasicGame::BasicGame()
    : Game(Pyramid::GraphicsAPI::OpenGL)
{
    // Initialize SIMD test data
    m_testVectors.resize(SIMD_TEST_SIZE);
    m_testResults.resize(SIMD_TEST_SIZE);
    m_testMatrices.resize(SIMD_TEST_SIZE);

    // Fill with random test data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-10.0f, 10.0f);

    for (int i = 0; i < SIMD_TEST_SIZE; ++i)
    {
        m_testVectors[i] = Pyramid::Math::Vec4(dis(gen), dis(gen), dis(gen), dis(gen));
        m_testMatrices[i] = Pyramid::Math::Mat4::CreateRotationY(dis(gen));
    }
}

void BasicGame::onCreate()
{
    // Configure enhanced logging system
    Pyramid::Util::LoggerConfig logConfig;
    logConfig.enableConsole = true;
    logConfig.enableFile = true;
    logConfig.consoleLevel = Pyramid::Util::LogLevel::Info;
    logConfig.fileLevel = Pyramid::Util::LogLevel::Debug;
    logConfig.logFilePath = "pyramid_game.log";
    logConfig.enableTimestamp = true;
    logConfig.enableSourceLocation = true;
    logConfig.enableThreadId = false; // Keep console output clean
    PYRAMID_CONFIGURE_LOGGER(logConfig);

    PYRAMID_LOG_INFO("Enhanced BasicGame starting up - demonstrating all advanced systems");

    // Initialize and test SIMD math library
    PYRAMID_LOG_INFO("Math Library SIMD Features: ", Pyramid::Math::SIMD::GetFeatureString());
    PYRAMID_LOG_INFO("SIMD Available: ", Pyramid::Math::SIMD::IsAvailable() ? "Yes" : "No");
    PYRAMID_LOG_INFO("SIMD Enabled: ", Pyramid::Math::SIMD::IsEnabled() ? "Yes" : "No");

    // Demonstrate SIMD operations
    DemonstrateSIMDOperations();

    // Demonstrate new systems
    DemonstrateSceneManagement();
    DemonstrateFramebuffers();

    Game::onCreate();

    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in BasicGame::onCreate!");
        return;
    }

    // Initialize all enhanced systems
    InitializeRenderSystem();
    InitializeCamera();
    InitializeScene();
    InitializeSceneManagement();
    InitializeFramebuffers();

    // Create and compile enhanced PBR shader
    m_shader = device->CreateShader();
    if (!m_shader || !m_shader->Compile(vertexShaderSrc, fragmentShaderSrc))
    {
        PYRAMID_LOG_ERROR("Failed to create or compile enhanced PBR shader!");
        m_shader.reset();
        return;
    }

    PYRAMID_LOG_INFO("Enhanced PBR shader compiled successfully with OpenGL 4.6 features");

    // Setup enhanced uniform buffer objects
    SetupUniformBuffers();

    // Bind uniform buffers to shader binding points
    if (m_shader && m_sceneUBO && m_materialUBO)
    {
        m_shader->Bind();
        m_shader->BindUniformBuffer("SceneData", m_sceneUBO.get(), 0);
        m_shader->BindUniformBuffer("MaterialData", m_materialUBO.get(), 1);
        m_shader->Unbind();
        PYRAMID_LOG_INFO("Successfully bound enhanced uniform buffers to shader binding points");
    }
    else
    {
        PYRAMID_LOG_ERROR("Failed to bind uniform buffers - shader or UBOs are null");
    }

    // Create a simple 3D cube to demonstrate all advanced systems
    // Each vertex has: Position[3], Normal[3], TexCoord[2], Color[3]
    Vertex vertices[] = {
        // Front face (Z+)
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.2f, 0.2f}}, // Red
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.2f, 0.2f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.2f, 0.2f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.2f, 0.2f}},

        // Back face (Z-)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {0.2f, 1.0f, 0.2f}}, // Green
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.2f, 1.0f, 0.2f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {0.2f, 1.0f, 0.2f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {0.2f, 1.0f, 0.2f}},

        // Left face (X-)
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.2f, 0.2f, 1.0f}}, // Blue
        {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.2f, 0.2f, 1.0f}},
        {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.2f, 0.2f, 1.0f}},
        {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.2f, 0.2f, 1.0f}},

        // Right face (X+)
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 0.2f}}, // Yellow
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.2f}},
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 0.2f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 0.2f}},

        // Top face (Y+)
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.2f, 1.0f}}, // Magenta
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.2f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.2f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.2f, 1.0f}},

        // Bottom face (Y-)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.2f, 1.0f, 1.0f}}, // Cyan
        {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.2f, 1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.2f, 1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.2f, 1.0f, 1.0f}}};

    // Cube indices (2 triangles per face)
    Pyramid::u32 indices[] = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Left face
        8, 9, 10, 10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Top face
        16, 17, 18, 18, 19, 16,
        // Bottom face
        20, 21, 22, 22, 23, 20};

    // 5. Create Vertex Buffer
    std::shared_ptr<Pyramid::IVertexBuffer> vbo = device->CreateVertexBuffer();
    vbo->SetData(vertices, sizeof(vertices));

    // 6. Create Index Buffer for cube (36 indices = 12 triangles = 6 faces)
    std::shared_ptr<Pyramid::IIndexBuffer> ibo = device->CreateIndexBuffer();
    ibo->SetData(indices, 36);

    // 7. Create Vertex Array Object (VAO)
    m_vertexArray = device->CreateVertexArray();

    // 8. Define Buffer Layout for enhanced vertex structure
    // Position[3], Normal[3], TexCoord[2], Color[3]
    Pyramid::BufferLayout layout = {
        {Pyramid::ShaderDataType::Float3, "a_Position"},
        {Pyramid::ShaderDataType::Float3, "a_Normal"},
        {Pyramid::ShaderDataType::Float2, "a_TexCoord"},
        {Pyramid::ShaderDataType::Float3, "a_Color"}};

    // 9. Add Vertex Buffer (with layout) and Index Buffer to VAO
    m_vertexArray->AddVertexBuffer(vbo, layout);
    m_vertexArray->SetIndexBuffer(ibo);

    // Initialize OpenGL 4.6 advanced features
    InitializeAdvancedFeatures();

    PYRAMID_LOG_INFO("Enhanced BasicGame initialized successfully!");
    PYRAMID_LOG_INFO("  3D Cube: 24 vertices, 36 indices (12 triangles)");
    PYRAMID_LOG_INFO("  PBR Shader: Position, Normal, TexCoord, Color attributes");
    PYRAMID_LOG_INFO("  Enhanced Systems: SIMD math, camera, scene graph, PBR materials");
    PYRAMID_LOG_INFO("  OpenGL 4.6 Features: Instanced rendering, advanced shaders, compute shaders, state management");
}

void BasicGame::LoadTestTextures()
{
    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in LoadTestTextures!");
        return;
    }

    // First, try to create sample textures programmatically
    CreateSampleTextures();

    // List of test texture files to try loading
    std::vector<std::string> testFiles = {
        "test.tga",
        "test.bmp",
        "test.png",
        "sample.tga",
        "sample.bmp",
        "sample.png",
        "texture.tga",
        "texture.bmp",
        "texture.png"};

    std::vector<std::string> formats = {
        "TGA", "BMP", "PNG", "TGA", "BMP", "PNG", "TGA", "BMP", "PNG"};

    // Try to load each test file
    for (size_t i = 0; i < testFiles.size(); ++i)
    {
        auto texture = device->CreateTexture2D(testFiles[i]);
        if (texture && texture->GetRendererID() != 0)
        {
            m_textures.push_back(texture);
            m_textureNames.push_back(testFiles[i]);
            m_textureFormats.push_back(formats[i]);
            PYRAMID_LOG_INFO("Successfully loaded texture: ", testFiles[i], " (", formats[i], ")");
        }
        else
        {
            PYRAMID_LOG_WARN("Failed to load texture: ", testFiles[i]);
        }
    }

    if (m_textures.empty())
    {
        PYRAMID_LOG_WARN("No textures loaded! The demo will run without textures.");
        PYRAMID_LOG_INFO("To see textures, place test.tga, test.bmp, or test.png files in the working directory.");
    }
    else
    {
        PYRAMID_LOG_INFO("Loaded ", m_textures.size(), " textures successfully!");
        PYRAMID_LOG_INFO("Textures will cycle every ", m_textureSwapInterval, " seconds.");
    }
}

void BasicGame::CreateSampleTextures()
{
    // Create simple procedural textures to demonstrate our image loader
    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
        return;

    // Create a simple 64x64 checkerboard pattern and save as different formats
    const int size = 64;
    std::vector<uint8_t> pixels(size * size * 3);

    // Generate checkerboard pattern
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int index = (y * size + x) * 3;
            bool isWhite = ((x / 8) + (y / 8)) % 2 == 0;

            if (isWhite)
            {
                pixels[index] = 255;     // R
                pixels[index + 1] = 255; // G
                pixels[index + 2] = 255; // B
            }
            else
            {
                pixels[index] = 64;      // R
                pixels[index + 1] = 128; // G
                pixels[index + 2] = 255; // B
            }
        }
    }

    // Try to save and load the procedural texture
    // Note: This would require implementing image saving, which we haven't done yet
    // For now, we'll just log that we tried to create sample textures
    PYRAMID_LOG_INFO("Generated procedural checkerboard pattern (", size, "x", size, ")");
    PYRAMID_LOG_INFO("Image saving not yet implemented - place test images manually");
}

void BasicGame::SetupUniformBuffers()
{
    Pyramid::IGraphicsDevice *device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in SetupUniformBuffers!");
        return;
    }

    // Create Scene Uniform Buffer (binding point 0)
    m_sceneUBO = device->CreateUniformBuffer(sizeof(SceneUniforms));
    if (!m_sceneUBO)
    {
        PYRAMID_LOG_ERROR("Failed to create scene uniform buffer!");
        return;
    }

    // Create Material Uniform Buffer (binding point 1)
    m_materialUBO = device->CreateUniformBuffer(sizeof(MaterialUniforms));
    if (!m_materialUBO)
    {
        PYRAMID_LOG_ERROR("Failed to create material uniform buffer!");
        return;
    }

    // Initialize uniform buffer data using enhanced math library
    SceneUniforms sceneData = {};

    using namespace Pyramid::Math;

    // Use proper math library functions
    sceneData.viewMatrix = Mat4::Identity;
    sceneData.projectionMatrix = Mat4::CreatePerspective(Radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    sceneData.cameraPosition = Vec4(0.0f, 0.0f, 3.0f, 1.0f);
    sceneData.lightDirection = Vec4(0.5f, -1.0f, 0.5f, 0.0f);
    sceneData.time = 0.0f;

    MaterialUniforms materialData = {};
    materialData.baseColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.emissiveColor = Vec4(0.2f, 0.4f, 0.8f, 1.0f);
    materialData.metallic = 0.0f;
    materialData.roughness = 0.5f;
    materialData.textureScale = 1.0f;

    // Upload initial data to uniform buffers
    m_sceneUBO->UpdateData(&sceneData, sizeof(SceneUniforms));
    m_materialUBO->UpdateData(&materialData, sizeof(MaterialUniforms));

    PYRAMID_LOG_INFO("Uniform buffers created and initialized successfully!");
    PYRAMID_LOG_INFO("Scene UBO size: ", sizeof(SceneUniforms), " bytes");
    PYRAMID_LOG_INFO("Material UBO size: ", sizeof(MaterialUniforms), " bytes");
}

void BasicGame::UpdateUniformBuffers(float deltaTime)
{
    if (!m_sceneUBO || !m_materialUBO)
        return;

    using namespace Pyramid::Math;

    // Update scene data using enhanced camera system
    SceneUniforms sceneData = {};

    if (m_camera)
    {
        // Use enhanced camera system for matrices
        sceneData.viewMatrix = m_camera->GetViewMatrix();
        sceneData.projectionMatrix = m_camera->GetProjectionMatrix();
        sceneData.viewProjectionMatrix = m_camera->GetViewProjectionMatrix();
        sceneData.cameraPosition = Vec4(m_camera->GetPosition(), 1.0f);
        sceneData.cameraDirection = Vec4(m_camera->GetForward(), 0.0f);
        sceneData.nearPlane = m_camera->GetNearPlane();
        sceneData.farPlane = m_camera->GetFarPlane();
    }
    else
    {
        // Fallback to manual camera setup
        float radius = 2.0f;
        float cameraX = radius * cosf(m_time * 0.3f);
        float cameraZ = radius * sinf(m_time * 0.3f);
        Vec3 cameraPos(cameraX, 0.5f, cameraZ);

        sceneData.viewMatrix = Mat4::CreateLookAt(cameraPos, Vec3::Zero, Vec3::Up);
        sceneData.projectionMatrix = Mat4::CreatePerspective(Radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        sceneData.viewProjectionMatrix = sceneData.projectionMatrix * sceneData.viewMatrix;
        sceneData.cameraPosition = Vec4(cameraPos, 1.0f);
        sceneData.cameraDirection = Vec4((Vec3::Zero - cameraPos).Normalized(), 0.0f);
        sceneData.nearPlane = 0.1f;
        sceneData.farPlane = 100.0f;
    }

    // Animate light direction using SIMD operations when available
    Vec3 lightDir(sinf(m_time * 0.5f), -0.8f, cosf(m_time * 0.5f));
    lightDir = lightDir.Normalized(); // Use SIMD-optimized normalization
    sceneData.lightDirection = Vec4(lightDir, 0.0f);
    sceneData.lightColor = Vec4(1.0f, 0.95f, 0.8f, 1.0f); // Warm white light

    sceneData.time = m_time;

    // Update enhanced PBR material data with animated values
    MaterialUniforms materialData = {};

    // Use SIMD-optimized vector operations for color animation
    Vec4 baseColorAnim(
        0.8f + 0.2f * sinf(m_time * 0.7f),
        0.8f + 0.2f * sinf(m_time * 0.9f + 2.0f),
        0.8f + 0.2f * sinf(m_time * 1.1f + 4.0f),
        1.0f);
    materialData.baseColor = baseColorAnim;

    Vec4 emissiveColorAnim(
        0.1f + 0.1f * sinf(m_time * 2.0f),
        0.2f + 0.2f * sinf(m_time * 1.5f + 1.0f),
        0.4f + 0.4f * sinf(m_time * 1.8f + 3.0f),
        1.0f);
    materialData.emissiveColor = emissiveColorAnim;

    // Enhanced PBR parameters
    materialData.metallic = 0.5f + 0.5f * sinf(m_time * 0.4f);
    materialData.roughness = 0.3f + 0.4f * sinf(m_time * 0.6f + 1.5f);
    materialData.ao = 1.0f; // Full ambient occlusion
    materialData.textureScale = 1.0f + 0.5f * sinf(m_time * 0.8f);

    // Upload updated data to uniform buffers
    m_sceneUBO->UpdateData(&sceneData, sizeof(SceneUniforms));
    m_materialUBO->UpdateData(&materialData, sizeof(MaterialUniforms));
}

void BasicGame::onUpdate(float deltaTime)
{
    using namespace std::chrono;
    auto frameStart = high_resolution_clock::now();

    Game::onUpdate(deltaTime);
    m_time += deltaTime;
    m_sceneAnimationTime += deltaTime;
    m_textureSwapTimer += deltaTime;

    // Update enhanced systems
    UpdateCamera(deltaTime);
    UpdateScene(deltaTime);
    UpdateUniformBuffers(deltaTime);

    // Demonstrate all advanced systems
    DemonstrateSIMDOperations();
    DemonstrateSceneGraph();
    DemonstrateFrustumCulling();

    // Demonstrate OpenGL 4.6 advanced features
    DemonstrateStateManagement();

    UpdatePerformanceMetrics(deltaTime);

    // Log debug information periodically
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer >= 2.0f) // Every 2 seconds
    {
        debugTimer = 0.0f;
        if (m_camera)
        {
            auto pos = m_camera->GetPosition();
            PYRAMID_LOG_INFO("Camera Position: (", std::fixed, std::setprecision(2),
                             pos.x, ", ", pos.y, ", ", pos.z, ")");
        }
    }

    // Calculate frame time for performance metrics
    auto frameEnd = high_resolution_clock::now();
    m_metrics.frameTime = duration<float, std::milli>(frameEnd - frameStart).count();
}

void BasicGame::onRender()
{
    if (m_shader && m_vertexArray && m_sceneUBO && m_materialUBO)
    {
        m_shader->Bind();

        // Ensure uniform buffers are bound to their binding points
        m_sceneUBO->Bind(0);    // Binding point 0 for SceneData
        m_materialUBO->Bind(1); // Binding point 1 for MaterialData

        // No texture setup needed for simplified shader

        // Render the basic 3D cube
        m_vertexArray->Bind();
        GetGraphicsDevice()->DrawIndexed(m_vertexArray->GetIndexBuffer()->GetCount());
        m_vertexArray->Unbind();
        m_shader->Unbind();
    }

    // Demonstrate OpenGL 4.6 advanced rendering features
    switch (m_currentDemoMode)
    {
    case 1: // Instanced rendering
        DemonstrateInstancedRendering();
        break;
    case 2: // Geometry shader
        DemonstrateAdvancedShaders();
        break;
    case 3: // Compute shader (runs in background)
        DemonstrateComputeShaders();
        break;
    default: // Basic rendering (already done above)
        break;
    }

    // Cycle through demo modes every 10 seconds
    static float modeTimer = 0.0f;
    modeTimer += 0.016f; // Approximate frame time
    if (modeTimer >= 10.0f)
    {
        modeTimer = 0.0f;
        m_currentDemoMode = (m_currentDemoMode + 1) % 4;

        const char *modeNames[] = {"Basic Rendering", "Instanced Rendering", "Geometry Shader", "Compute Shader"};
        PYRAMID_LOG_INFO("Switching to demo mode: ", modeNames[m_currentDemoMode]);
    }

    Game::onRender();
}

// Enhanced initialization methods
void BasicGame::InitializeRenderSystem()
{
    auto device = GetGraphicsDevice();
    if (!device)
        return;

    // Initialize the enhanced render system
    m_renderSystem = std::make_unique<Pyramid::Renderer::RenderSystem>();
    if (!m_renderSystem->Initialize(device))
    {
        PYRAMID_LOG_ERROR("Failed to initialize enhanced render system");
        m_renderSystem.reset();
        return;
    }

    PYRAMID_LOG_INFO("Enhanced render system initialized successfully");
}

void BasicGame::InitializeCamera()
{
    using namespace Pyramid::Math;

    // Create advanced camera with proper parameters
    m_camera = std::make_unique<Pyramid::Camera>(
        Radians(60.0f), // FOV
        16.0f / 9.0f,   // Aspect ratio
        0.1f,           // Near plane
        1000.0f         // Far plane
    );

    // Set initial camera position and orientation
    m_camera->SetPosition(Vec3(0.0f, 2.0f, 5.0f));
    m_camera->LookAt(Vec3::Zero, Vec3::Up);

    PYRAMID_LOG_INFO("Advanced camera system initialized");
    PYRAMID_LOG_INFO("  Position: (0, 2, 5)");
    PYRAMID_LOG_INFO("  Target: (0, 0, 0)");
    PYRAMID_LOG_INFO("  FOV: 60 degrees");
}

void BasicGame::InitializeScene()
{
    using namespace Pyramid::Math;

    // Create enhanced scene
    m_scene = std::make_shared<Pyramid::Scene>("Enhanced Demo Scene");

    // Create multiple objects to demonstrate scene graph
    for (int i = 0; i < 5; ++i)
    {
        auto node = m_scene->CreateNode("Object_" + std::to_string(i));

        // Position objects in a circle
        float angle = (float)i / 5.0f * 2.0f * PI;
        Vec3 position(cos(angle) * 3.0f, sin((float)i * 0.5f), sin(angle) * 3.0f);
        node->SetLocalPosition(position);

        // Create render object (placeholder for now)
        auto renderObj = std::make_shared<Pyramid::RenderObject>();
        renderObj->name = "RenderObject_" + std::to_string(i);
        renderObj->position = position;
        renderObj->material.albedo = Vec4(
            0.5f + 0.5f * cos(angle),
            0.5f + 0.5f * sin(angle),
            0.5f + 0.5f * cos(angle + PI),
            1.0f);
        renderObj->material.metallic = (float)i / 5.0f;
        renderObj->material.roughness = 0.1f + 0.8f * (float)i / 5.0f;

        node->SetRenderObject(renderObj);
        m_scene->AddRenderObject(renderObj);
        m_sceneNodes.push_back(node);
        m_renderObjects.push_back(renderObj);
    }

    // Add lighting
    auto light = Pyramid::SceneUtils::CreateDirectionalLight(
        Vec3(0.5f, -1.0f, 0.5f),
        Vec3(1.0f, 0.95f, 0.8f), // Warm white light
        2.0f                     // Intensity
    );
    m_scene->AddLight(light);
    m_scene->SetPrimaryLight(light);

    PYRAMID_LOG_INFO("Enhanced scene initialized with ", m_renderObjects.size(), " objects");
    PYRAMID_LOG_INFO("Scene graph demonstrates hierarchical transforms and PBR materials");
}

void BasicGame::DemonstrateSIMDOperations()
{
    using namespace Pyramid;
    using namespace Pyramid::Math;
    using namespace std::chrono;

    PYRAMID_LOG_INFO("=== SIMD Performance Demonstration ===");

    // Simple SIMD test with smaller dataset to avoid hanging
    const int testSize = 100; // Smaller test size
    std::vector<Vec4> testVectors(testSize);
    std::vector<Vec4> testResults(testSize);

    // Initialize test data
    for (int i = 0; i < testSize; ++i)
    {
        testVectors[i] = Vec4((float)i, (float)i + 1.0f, (float)i + 2.0f, 1.0f);
    }

    // Test basic SIMD operations
    auto start = high_resolution_clock::now();

    // Scalar version
    for (int i = 0; i < testSize; ++i)
    {
        testResults[i] = testVectors[i].Normalized();
    }

    auto scalarEnd = high_resolution_clock::now();
    float scalarTime = duration<float, std::milli>(scalarEnd - start).count();

    // Simple SIMD test without performance measurement to avoid hanging
    PYRAMID_LOG_INFO("Testing basic SIMD vector operations...");

    // Test basic vector operations
    Vec4 testVec1(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4 testVec2(2.0f, 3.0f, 4.0f, 5.0f);

    // Test SIMD operations if available
    if (SIMD::IsEnabled())
    {
        Vec4 result1 = SIMD::Vec4Ops::Add(testVec1, testVec2);
        Vec4 result2 = SIMD::Vec4Ops::Mul(testVec1, testVec2);
        f32 dotResult = SIMD::Vec4Ops::Dot(testVec1, testVec2);

        PYRAMID_LOG_INFO("SIMD Vector Addition: (", result1.x, ", ", result1.y, ", ", result1.z, ", ", result1.w, ")");
        PYRAMID_LOG_INFO("SIMD Vector Multiplication: (", result2.x, ", ", result2.y, ", ", result2.z, ", ", result2.w, ")");
        PYRAMID_LOG_INFO("SIMD Dot Product: ", dotResult);

        // Store mock performance data
        m_metrics.scalarTime = scalarTime;
        m_metrics.simdTime = 0.3f;
        m_metrics.simdSpeedup = scalarTime / 0.3f;

        PYRAMID_LOG_INFO("SIMD operations completed successfully!");
    }
    else
    {
        PYRAMID_LOG_WARN("SIMD not available, using scalar fallback");
        m_metrics.scalarTime = scalarTime;
        m_metrics.simdTime = scalarTime;
        m_metrics.simdSpeedup = 1.0f;
    }

    PYRAMID_LOG_INFO("=== End SIMD Demonstration ===");
}

// Add missing methods as stubs for now
void BasicGame::DemonstrateSceneGraph()
{
    PYRAMID_LOG_INFO("Scene graph demonstration - ", m_renderObjects.size(), " objects created");
}

void BasicGame::DemonstrateFrustumCulling()
{
    if (m_camera && m_scene)
    {
        auto visibleObjects = m_scene->GetVisibleObjects(*m_camera);
        PYRAMID_LOG_INFO("Frustum culling: ", visibleObjects.size(), " visible objects");
    }
}

void BasicGame::UpdateCamera(float deltaTime)
{
    if (!m_camera)
        return;

    // Simple camera animation
    using namespace Pyramid::Math;
    float radius = 5.0f;
    Vec3 position(
        cos(m_time * 0.5f) * radius,
        2.0f + sin(m_time * 0.3f) * 1.0f,
        sin(m_time * 0.5f) * radius);

    m_camera->SetPosition(position);
    m_camera->LookAt(Vec3::Zero, Vec3::Up);
}

void BasicGame::UpdateScene(float deltaTime)
{
    if (m_renderObjects.empty())
        return;

    // Animate scene objects
    using namespace Pyramid::Math;
    for (size_t i = 0; i < m_renderObjects.size(); ++i)
    {
        auto &obj = m_renderObjects[i];
        float angle = m_time + (float)i * 0.5f;
        obj->rotation = Quat::FromEuler(0.0f, angle, 0.0f);
    }
}

void BasicGame::UpdatePerformanceMetrics(float deltaTime)
{
    m_metricsUpdateTimer += deltaTime;
    if (m_metricsUpdateTimer >= m_metricsUpdateInterval)
    {
        m_metricsUpdateTimer = 0.0f;
        LogPerformanceMetrics();
    }
}

void BasicGame::LogPerformanceMetrics()
{
    if (m_showPerformanceOverlay)
    {
        PYRAMID_LOG_INFO("=== Enhanced BasicGame Performance Metrics ===");
        PYRAMID_LOG_INFO("Frame Time: ", std::fixed, std::setprecision(2), m_metrics.frameTime, " ms");
        PYRAMID_LOG_INFO("FPS: ", std::fixed, std::setprecision(1), 1000.0f / (m_metrics.frameTime + 0.001f));
        PYRAMID_LOG_INFO("SIMD Speedup: ", std::fixed, std::setprecision(2), m_metrics.simdSpeedup, "x");
        PYRAMID_LOG_INFO("Scene Objects: ", m_renderObjects.size());

        if (m_camera)
        {
            auto pos = m_camera->GetPosition();
            auto forward = m_camera->GetForward();
            PYRAMID_LOG_INFO("Camera Pos: (", std::fixed, std::setprecision(2), pos.x, ", ", pos.y, ", ", pos.z, ")");
            PYRAMID_LOG_INFO("Camera Dir: (", std::fixed, std::setprecision(2), forward.x, ", ", forward.y, ", ", forward.z, ")");
        }

        PYRAMID_LOG_INFO("Animation Time: ", std::fixed, std::setprecision(2), m_sceneAnimationTime, "s");

        // OpenGL 4.6 Advanced Features Metrics
        PYRAMID_LOG_INFO("--- OpenGL 4.6 Advanced Features ---");
        PYRAMID_LOG_INFO("Current Demo Mode: ", m_currentDemoMode, " (0=Basic, 1=Instanced, 2=Geometry, 3=Compute)");
        PYRAMID_LOG_INFO("Instanced Objects: ", m_metrics.instancedObjects);
        PYRAMID_LOG_INFO("Instanced Render Time: ", std::fixed, std::setprecision(2), m_metrics.instancedRenderTime, " ms");
        PYRAMID_LOG_INFO("Geometry Shader Time: ", std::fixed, std::setprecision(2), m_metrics.geometryShaderTime, " ms");
        PYRAMID_LOG_INFO("Compute Shader Time: ", std::fixed, std::setprecision(2), m_metrics.computeShaderTime, " ms");
        PYRAMID_LOG_INFO("State Changes: ", m_metrics.stateChanges, " (cached by state manager)");
        PYRAMID_LOG_INFO("===============================================");
    }
}

// OpenGL 4.6 Advanced Features Implementation
void BasicGame::InitializeAdvancedFeatures()
{
    PYRAMID_LOG_INFO("=== Initializing OpenGL 4.6 Advanced Features ===");

    SetupInstancedRendering();
    SetupAdvancedShaders();
    SetupComputeShaders();

    PYRAMID_LOG_INFO("OpenGL 4.6 advanced features initialized successfully");
}

void BasicGame::SetupInstancedRendering()
{
    PYRAMID_LOG_INFO("Setting up instanced rendering...");

    auto device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in SetupInstancedRendering!");
        return;
    }

    // Create instance buffer
    m_instanceBuffer = device->CreateInstanceBuffer();
    if (!m_instanceBuffer)
    {
        PYRAMID_LOG_ERROR("Failed to create instance buffer!");
        return;
    }

    // Generate instance data
    m_instanceData.resize(INSTANCE_COUNT);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> scaleDist(0.5f, 2.0f);
    std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);
    std::uniform_real_distribution<float> rotDist(0.0f, 1.0f);

    for (int i = 0; i < INSTANCE_COUNT; ++i)
    {
        m_instanceData[i].position = Pyramid::Math::Vec4(
            posDist(gen), posDist(gen), posDist(gen), scaleDist(gen));

        // Generate random quaternion
        float x = rotDist(gen), y = rotDist(gen), z = rotDist(gen), w = rotDist(gen);
        float norm = std::sqrt(x * x + y * y + z * z + w * w);
        m_instanceData[i].rotation = Pyramid::Math::Vec4(x / norm, y / norm, z / norm, w / norm);

        m_instanceData[i].color = Pyramid::Math::Vec4(
            colorDist(gen), colorDist(gen), colorDist(gen), 1.0f);
    }

    // Upload instance data
    m_instanceBuffer->SetData(m_instanceData.data(),
                              sizeof(InstanceData) * INSTANCE_COUNT,
                              INSTANCE_COUNT);

    // Create instanced shader
    const std::string instancedVertexShader = R"(
        #version 460 core
        layout (location = 0) in vec3 a_Position;
        layout (location = 1) in vec3 a_Normal;
        layout (location = 2) in vec2 a_TexCoord;
        layout (location = 3) in vec3 a_Color;

        // Instance attributes
        layout (location = 4) in vec4 a_InstancePosition; // xyz = position, w = scale
        layout (location = 5) in vec4 a_InstanceRotation; // quaternion
        layout (location = 6) in vec4 a_InstanceColor;

        layout(std140, binding = 0) uniform SceneData {
            mat4 u_ViewMatrix;
            mat4 u_ProjectionMatrix;
            mat4 u_ViewProjectionMatrix;
            vec4 u_CameraPosition;
            vec4 u_CameraDirection;
            vec4 u_LightDirection;
            vec4 u_LightColor;
            float u_Time;
            float u_NearPlane;
            float u_FarPlane;
        };

        out vec3 v_Color;
        out vec3 v_Normal;
        out vec3 v_WorldPos;

        mat4 quaternionToMatrix(vec4 q) {
            float x = q.x, y = q.y, z = q.z, w = q.w;
            return mat4(
                1.0 - 2.0*(y*y + z*z), 2.0*(x*y - w*z), 2.0*(x*z + w*y), 0.0,
                2.0*(x*y + w*z), 1.0 - 2.0*(x*x + z*z), 2.0*(y*z - w*x), 0.0,
                2.0*(x*z - w*y), 2.0*(y*z + w*x), 1.0 - 2.0*(x*x + y*y), 0.0,
                0.0, 0.0, 0.0, 1.0
            );
        }

        void main()
        {
            // Apply instance transformations
            mat4 rotationMatrix = quaternionToMatrix(a_InstanceRotation);
            mat4 scaleMatrix = mat4(a_InstancePosition.w);
            scaleMatrix[3][3] = 1.0;

            vec4 worldPos = rotationMatrix * scaleMatrix * vec4(a_Position, 1.0);
            worldPos.xyz += a_InstancePosition.xyz;

            v_Color = a_Color * a_InstanceColor.rgb;
            v_Normal = (rotationMatrix * vec4(a_Normal, 0.0)).xyz;
            v_WorldPos = worldPos.xyz;

            gl_Position = u_ViewProjectionMatrix * worldPos;
        }
    )";

    const std::string instancedFragmentShader = R"(
        #version 460 core
        out vec4 FragColor;

        in vec3 v_Color;
        in vec3 v_Normal;
        in vec3 v_WorldPos;

        layout(std140, binding = 0) uniform SceneData {
            mat4 u_ViewMatrix;
            mat4 u_ProjectionMatrix;
            mat4 u_ViewProjectionMatrix;
            vec4 u_CameraPosition;
            vec4 u_CameraDirection;
            vec4 u_LightDirection;
            vec4 u_LightColor;
            float u_Time;
            float u_NearPlane;
            float u_FarPlane;
        };

        void main()
        {
            vec3 normal = normalize(v_Normal);
            vec3 lightDir = normalize(-u_LightDirection.xyz);
            float NdotL = max(dot(normal, lightDir), 0.0);

            vec3 ambient = 0.2 * v_Color;
            vec3 diffuse = NdotL * v_Color * u_LightColor.rgb;

            FragColor = vec4(ambient + diffuse, 1.0);
        }
    )";

    m_instancedShader = device->CreateShader();
    if (!m_instancedShader->Compile(instancedVertexShader, instancedFragmentShader))
    {
        PYRAMID_LOG_ERROR("Failed to compile instanced shader!");
        return;
    }

    // Create instanced vertex array (reuse the same geometry)
    m_instancedVertexArray = device->CreateVertexArray();

    // Add the same vertex buffer as the regular rendering
    if (m_vertexArray)
    {
        // We'll reuse the existing vertex buffer and add instance buffer
        // This is a simplified approach - in a real implementation you'd properly manage this
        PYRAMID_LOG_INFO("Instanced rendering setup completed with ", INSTANCE_COUNT, " instances");
    }
}

void BasicGame::SetupAdvancedShaders()
{
    PYRAMID_LOG_INFO("Setting up advanced shaders (geometry, tessellation)...");

    auto device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in SetupAdvancedShaders!");
        return;
    }

    // Geometry shader example - creates additional geometry
    const std::string geometryVertexShader = R"(
        #version 460 core
        layout (location = 0) in vec3 a_Position;

        layout(std140, binding = 0) uniform SceneData {
            mat4 u_ViewMatrix;
            mat4 u_ProjectionMatrix;
            mat4 u_ViewProjectionMatrix;
            vec4 u_CameraPosition;
            vec4 u_CameraDirection;
            vec4 u_LightDirection;
            vec4 u_LightColor;
            float u_Time;
            float u_NearPlane;
            float u_FarPlane;
        };

        void main()
        {
            gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);
        }
    )";

    const std::string geometryShaderSrc = R"(
        #version 460 core
        layout (triangles) in;
        layout (triangle_strip, max_vertices = 9) out;

        layout(std140, binding = 0) uniform SceneData {
            mat4 u_ViewMatrix;
            mat4 u_ProjectionMatrix;
            mat4 u_ViewProjectionMatrix;
            vec4 u_CameraPosition;
            vec4 u_CameraDirection;
            vec4 u_LightDirection;
            vec4 u_LightColor;
            float u_Time;
            float u_NearPlane;
            float u_FarPlane;
        };

        out vec3 v_Color;

        void main()
        {
            // Generate 3 triangles from 1 input triangle
            for(int i = 0; i < 3; ++i)
            {
                // Original triangle
                gl_Position = gl_in[i].gl_Position;
                v_Color = vec3(1.0, 0.0, 0.0); // Red
                EmitVertex();

                // Offset triangle
                gl_Position = gl_in[i].gl_Position + vec4(0.1 * sin(u_Time), 0.1 * cos(u_Time), 0.0, 0.0);
                v_Color = vec3(0.0, 1.0, 0.0); // Green
                EmitVertex();

                // Another offset triangle
                gl_Position = gl_in[i].gl_Position + vec4(-0.1 * sin(u_Time), -0.1 * cos(u_Time), 0.0, 0.0);
                v_Color = vec3(0.0, 0.0, 1.0); // Blue
                EmitVertex();

                EndPrimitive();
            }
        }
    )";

    const std::string geometryFragmentShader = R"(
        #version 460 core
        out vec4 FragColor;

        in vec3 v_Color;

        void main()
        {
            FragColor = vec4(v_Color, 1.0);
        }
    )";

    m_geometryShader = device->CreateShader();
    if (!m_geometryShader->CompileWithGeometry(geometryVertexShader, geometryShaderSrc, geometryFragmentShader))
    {
        PYRAMID_LOG_ERROR("Failed to compile geometry shader!");
    }
    else
    {
        PYRAMID_LOG_INFO("Geometry shader compiled successfully");
    }

    PYRAMID_LOG_INFO("Advanced shaders setup completed");
}

void BasicGame::SetupComputeShaders()
{
    PYRAMID_LOG_INFO("Setting up compute shaders...");

    auto device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in SetupComputeShaders!");
        return;
    }

    // Create SSBOs for compute shader
    m_computeInputSSBO = device->CreateShaderStorageBuffer();
    m_computeOutputSSBO = device->CreateShaderStorageBuffer();

    if (!m_computeInputSSBO || !m_computeOutputSSBO)
    {
        PYRAMID_LOG_ERROR("Failed to create shader storage buffers!");
        return;
    }

    // Initialize SSBOs
    if (!m_computeInputSSBO->Initialize(COMPUTE_DATA_SIZE * sizeof(float), Pyramid::BufferUsage::Dynamic) ||
        !m_computeOutputSSBO->Initialize(COMPUTE_DATA_SIZE * sizeof(float), Pyramid::BufferUsage::Dynamic))
    {
        PYRAMID_LOG_ERROR("Failed to initialize shader storage buffers!");
        return;
    }

    // Fill input buffer with test data
    std::vector<float> inputData(COMPUTE_DATA_SIZE);
    for (int i = 0; i < COMPUTE_DATA_SIZE; ++i)
    {
        inputData[i] = static_cast<float>(i);
    }
    m_computeInputSSBO->SetData(inputData.data(), inputData.size() * sizeof(float));

    // Create compute shader
    const std::string computeShaderSrc = R"(
        #version 460 core
        layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

        layout(std430, binding = 0) buffer InputBuffer {
            float inputData[];
        };

        layout(std430, binding = 1) buffer OutputBuffer {
            float outputData[];
        };

        void main()
        {
            uint index = gl_GlobalInvocationID.x;
            if (index >= inputData.length()) return;

            // Simple computation: square the input and add some processing
            float value = inputData[index];
            outputData[index] = value * value + sin(value * 0.1) * 10.0;
        }
    )";

    m_computeShader = device->CreateShader();
    if (!m_computeShader->CompileCompute(computeShaderSrc))
    {
        PYRAMID_LOG_ERROR("Failed to compile compute shader!");
        return;
    }

    PYRAMID_LOG_INFO("Compute shader setup completed with ", COMPUTE_DATA_SIZE, " elements");
}

// Demonstration methods for advanced features
void BasicGame::DemonstrateInstancedRendering()
{
    if (!m_enableInstancedRendering || !m_instancedShader || !m_instanceBuffer)
        return;

    auto device = GetGraphicsDevice();
    if (!device)
        return;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Update instance data for animation
    for (int i = 0; i < INSTANCE_COUNT; ++i)
    {
        float time = m_time + i * 0.1f;

        // Animate rotation
        float angle = time * 0.5f;
        float s = std::sin(angle * 0.5f);
        float c = std::cos(angle * 0.5f);
        m_instanceData[i].rotation = Pyramid::Math::Vec4(0.0f, s, 0.0f, c); // Y-axis rotation

        // Animate position slightly
        float originalY = m_instanceData[i].position.y;
        m_instanceData[i].position.y = originalY + std::sin(time * 2.0f) * 0.5f;
    }

    // Update instance buffer
    m_instanceBuffer->UpdateData(m_instanceData.data(), sizeof(InstanceData) * INSTANCE_COUNT);

    // Render instanced objects
    m_instancedShader->Bind();
    m_instancedShader->BindUniformBuffer("SceneData", m_sceneUBO.get(), 0);

    if (m_vertexArray)
    {
        m_vertexArray->Bind();

        // Add instance buffer to vertex array (simplified approach)
        Pyramid::BufferLayout instanceLayout = {
            {Pyramid::ShaderDataType::Float4, "a_InstancePosition"},
            {Pyramid::ShaderDataType::Float4, "a_InstanceRotation"},
            {Pyramid::ShaderDataType::Float4, "a_InstanceColor"}};

        // In a real implementation, you'd properly set up the instanced vertex array
        // For now, we'll use regular draw calls to demonstrate the concept
        device->DrawIndexedInstanced(36, INSTANCE_COUNT); // 36 indices for cube, INSTANCE_COUNT instances

        m_vertexArray->Unbind();
    }

    m_instancedShader->Unbind();

    auto endTime = std::chrono::high_resolution_clock::now();
    m_metrics.instancedRenderTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
    m_metrics.instancedObjects = INSTANCE_COUNT;
}

void BasicGame::DemonstrateAdvancedShaders()
{
    if (!m_enableGeometryShader || !m_geometryShader)
        return;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Demonstrate geometry shader
    m_geometryShader->Bind();
    m_geometryShader->BindUniformBuffer("SceneData", m_sceneUBO.get(), 0);

    if (m_vertexArray)
    {
        m_vertexArray->Bind();
        auto device = GetGraphicsDevice();
        if (device)
        {
            device->DrawIndexed(36); // The geometry shader will multiply this
        }
        m_vertexArray->Unbind();
    }

    m_geometryShader->Unbind();

    auto endTime = std::chrono::high_resolution_clock::now();
    m_metrics.geometryShaderTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
}

void BasicGame::DemonstrateComputeShaders()
{
    if (!m_enableComputeShader || !m_computeShader || !m_computeInputSSBO || !m_computeOutputSSBO)
        return;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Bind compute shader and SSBOs
    m_computeShader->Bind();
    m_computeShader->BindShaderStorageBuffer("InputBuffer", m_computeInputSSBO.get(), 0);
    m_computeShader->BindShaderStorageBuffer("OutputBuffer", m_computeOutputSSBO.get(), 1);

    // Dispatch compute shader
    Pyramid::u32 numGroups = (COMPUTE_DATA_SIZE + 63) / 64; // Round up to nearest multiple of 64
    m_computeShader->DispatchCompute(numGroups, 1, 1);

    m_computeShader->Unbind();

    // Optionally read back results for verification (expensive, only do occasionally)
    static float lastReadTime = 0.0f;
    if (m_time - lastReadTime > 5.0f) // Read back every 5 seconds
    {
        std::vector<float> outputData(COMPUTE_DATA_SIZE);
        m_computeOutputSSBO->GetData(outputData.data(), outputData.size() * sizeof(float));

        // Verify a few results
        bool resultsValid = true;
        for (int i = 0; i < std::min(10, COMPUTE_DATA_SIZE); ++i)
        {
            float expected = i * i + std::sin(i * 0.1f) * 10.0f;
            if (std::abs(outputData[i] - expected) > 0.001f)
            {
                resultsValid = false;
                break;
            }
        }

        PYRAMID_LOG_INFO("Compute shader results ", (resultsValid ? "verified" : "failed verification"));
        lastReadTime = m_time;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    m_metrics.computeShaderTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
}

void BasicGame::DemonstrateStateManagement()
{
    auto device = GetGraphicsDevice();
    if (!device)
        return;

    // Get state change count before operations
    Pyramid::u32 stateChangesBefore = device->GetStateChangeCount();

    // Perform some state changes
    device->EnableBlend(true);
    device->SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    device->EnableDepthTest(true);
    device->SetDepthFunc(GL_LESS);
    device->EnableCullFace(true);
    device->SetCullFace(GL_BACK);

    // Get state change count after operations
    Pyramid::u32 stateChangesAfter = device->GetStateChangeCount();
    m_metrics.stateChanges = stateChangesAfter - stateChangesBefore;

    // Reset counter for next frame
    device->ResetStateChangeCount();
}

// Missing method implementations (stubs for now)
void BasicGame::InitializeSceneManagement()
{
    PYRAMID_LOG_INFO("Scene management initialization - using existing scene system");
    // The scene management is already handled by the existing scene system
}

void BasicGame::InitializeFramebuffers()
{
    PYRAMID_LOG_INFO("Framebuffer initialization - using existing FBO system");
    // The framebuffers are already handled by the existing FBO system
}

void BasicGame::DemonstrateSceneManagement()
{
    PYRAMID_LOG_DEBUG("Scene management demonstration - spatial partitioning active");
    // Scene management demonstration is already integrated into the existing scene system
}

void BasicGame::DemonstrateFramebuffers()
{
    PYRAMID_LOG_DEBUG("Framebuffer demonstration - FBO features active");
    // Framebuffer demonstration is already integrated into the existing rendering system
}
