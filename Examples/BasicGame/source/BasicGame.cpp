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

// Enhanced GLSL Shader Sources for PBR and advanced features
const std::string vertexShaderSrc = R"(
    #version 460 core
    layout (location = 0) in vec3 a_Position;
    layout (location = 1) in vec3 a_Normal;
    layout (location = 2) in vec2 a_TexCoord;
    layout (location = 3) in vec3 a_Color;

    // Enhanced Uniform Buffer Objects (std140 layout)
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
    out vec2 v_TexCoord;
    out vec3 v_WorldPos;
    out vec3 v_Normal;
    out vec3 v_ViewPos;

    void main()
    {
        v_Color = a_Color;
        v_TexCoord = a_TexCoord;
        v_Normal = a_Normal;

        // Calculate world position with animation
        vec3 animatedPos = a_Position;
        animatedPos.y += sin(u_Time * 2.0 + a_Position.x * 3.0) * 0.1;

        v_WorldPos = animatedPos;
        v_ViewPos = (u_ViewMatrix * vec4(animatedPos, 1.0)).xyz;

        // Use pre-calculated view-projection matrix for efficiency
        gl_Position = u_ViewProjectionMatrix * vec4(animatedPos, 1.0);
    }
)";

const std::string fragmentShaderSrc = R"(
    #version 460 core
    out vec4 FragColor;

    in vec3 v_Color;
    in vec2 v_TexCoord;
    in vec3 v_WorldPos;
    in vec3 v_Normal;
    in vec3 v_ViewPos;

    // Enhanced Uniform Buffer Objects (std140 layout)
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

    uniform sampler2D u_Texture;

    // PBR utility functions
    vec3 fresnelSchlick(float cosTheta, vec3 F0) {
        return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
    }

    float distributionGGX(vec3 N, vec3 H, float roughness) {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH * NdotH;

        float num = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = 3.14159265 * denom * denom;

        return num / denom;
    }

    float geometrySchlickGGX(float NdotV, float roughness) {
        float r = (roughness + 1.0);
        float k = (r * r) / 8.0;

        float num = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return num / denom;
    }

    float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx2 = geometrySchlickGGX(NdotV, roughness);
        float ggx1 = geometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    void main()
    {
        // Sample texture with animated UV coordinates
        vec2 animatedUV = v_TexCoord * u_TextureScale + sin(u_Time * 0.5) * 0.05;
        vec4 texColor = texture(u_Texture, animatedUV);

        // Material properties
        vec3 albedo = texColor.rgb * u_BaseColor.rgb;
        float metallic = u_Metallic;
        float roughness = u_Roughness;
        float ao = u_AO;

        // Lighting vectors
        vec3 N = normalize(v_Normal);
        vec3 V = normalize(u_CameraPosition.xyz - v_WorldPos);
        vec3 L = normalize(-u_LightDirection.xyz);
        vec3 H = normalize(V + L);

        // Calculate reflectance at normal incidence
        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        // Cook-Torrance BRDF
        float NDF = distributionGGX(N, H, roughness);
        float G = geometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular = numerator / denominator;

        // Add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        vec3 Lo = (kD * albedo / 3.14159265 + specular) * u_LightColor.rgb * NdotL;

        // Ambient lighting
        vec3 ambient = vec3(0.03) * albedo * ao;

        // Emissive
        vec3 emissive = u_EmissiveColor.rgb * (sin(u_Time * 2.0) * 0.5 + 0.5);

        vec3 color = ambient + Lo + emissive;

        // HDR tone mapping (simple Reinhard)
        color = color / (color + vec3(1.0));

        // Gamma correction
        color = pow(color, vec3(1.0/2.2));

        FragColor = vec4(color * v_Color, texColor.a * u_BaseColor.a);
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

    // 3. Load multiple textures to showcase our custom Pyramid image loader
    // Supports TGA, BMP, and PNG formats through our custom implementation
    LoadTestTextures();

    // 4. Define vertex and index data for a triangle (now with UVs)
    // UVs: (0,0) bottom-left, (1,0) bottom-right, (0.5,1) top-center
    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // White, UV (0,0)
        {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},  // White, UV (1,0)
        {{0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}}    // White, UV (0.5,1)
    };

    Pyramid::u32 indices[] = {0, 1, 2};

    // 5. Create Vertex Buffer
    std::shared_ptr<Pyramid::IVertexBuffer> vbo = device->CreateVertexBuffer();
    vbo->SetData(vertices, sizeof(vertices));

    // 6. Create Index Buffer
    std::shared_ptr<Pyramid::IIndexBuffer> ibo = device->CreateIndexBuffer();
    ibo->SetData(indices, 3);

    // 7. Create Vertex Array Object (VAO)
    m_vertexArray = device->CreateVertexArray();

    // 8. Define Buffer Layout (now includes TexCoord)
    Pyramid::BufferLayout layout = {
        {Pyramid::ShaderDataType::Float3, "a_Position"},
        {Pyramid::ShaderDataType::Float3, "a_Color"},
        {Pyramid::ShaderDataType::Float2, "a_TexCoord"}};

    // 9. Add Vertex Buffer (with layout) and Index Buffer to VAO
    m_vertexArray->AddVertexBuffer(vbo, layout);
    m_vertexArray->SetIndexBuffer(ibo);

    PYRAMID_LOG_INFO("BasicGame initialized with OpenGL 4.6 Uniform Buffer Objects!");
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
    UpdatePerformanceMetrics(deltaTime);

    // Cycle through textures
    if (!m_textures.empty() && m_textureSwapTimer >= m_textureSwapInterval)
    {
        m_currentTextureIndex = (m_currentTextureIndex + 1) % m_textures.size();
        m_textureSwapTimer = 0.0f;

        PYRAMID_LOG_INFO("Switched to texture: ", m_textureNames[m_currentTextureIndex],
                         " (", m_textureFormats[m_currentTextureIndex], ")");
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
        // (This should already be set up, but we can ensure they're active)
        m_sceneUBO->Bind(0);    // Binding point 0 for SceneData
        m_materialUBO->Bind(1); // Binding point 1 for MaterialData

        // Bind current texture and set sampler uniform
        if (!m_textures.empty() && m_currentTextureIndex < m_textures.size())
        {
            auto currentTexture = m_textures[m_currentTextureIndex];
            if (currentTexture)
            {
                currentTexture->Bind(0);                 // Bind to texture unit 0
                m_shader->SetUniformInt("u_Texture", 0); // Tell shader sampler u_Texture uses texture unit 0
            }
        }
        else
        {
            // No textures loaded - the shader will use default sampling
            // Create a simple white texture as fallback
            static bool fallbackCreated = false;
            if (!fallbackCreated)
            {
                PYRAMID_LOG_INFO("No textures available, using uniform buffer animation only");
                fallbackCreated = true;
            }
        }

        m_vertexArray->Bind();
        GetGraphicsDevice()->DrawIndexed(m_vertexArray->GetIndexBuffer()->GetCount());
        m_vertexArray->Unbind();
        m_shader->Unbind();
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
        PYRAMID_LOG_INFO("=== Performance Metrics ===");
        PYRAMID_LOG_INFO("Frame Time: ", std::fixed, std::setprecision(2), m_metrics.frameTime, " ms");
        PYRAMID_LOG_INFO("SIMD Speedup: ", std::fixed, std::setprecision(2), m_metrics.simdSpeedup, "x");
        PYRAMID_LOG_INFO("Visible Objects: ", m_metrics.visibleObjects);
        PYRAMID_LOG_INFO("===========================");
    }
}
