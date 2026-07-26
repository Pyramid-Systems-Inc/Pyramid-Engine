#include "BasicRendering.hpp"

#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Input/InputActions.hpp>
#include <Pyramid/Graphics/CameraController.hpp>

#include <array>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Util/Log.hpp>
#include <cmath>
#include <string_view>
#include <utility>

namespace
{
    constexpr std::string_view kCameraContext = "rts-camera-reference";
    constexpr std::string_view kQuitAction = "Quit";
    constexpr std::string_view kResetAction = "ResetCamera";
    constexpr std::string_view kMoveAction = "Move";
    constexpr std::string_view kOrbitDeltaAction = "OrbitDelta";
    constexpr std::string_view kOrbitRateAction = "OrbitRate";
    constexpr std::string_view kZoomDeltaAction = "ZoomDelta";
    constexpr std::string_view kZoomRateAction = "ZoomRate";
    constexpr std::string_view kBoostAction = "Boost";
}

// Vertex shader source
const std::string vertexShaderSrc = R"(
    #version 330 core
    layout (location = 0) in vec3 a_Position;
    layout (location = 1) in vec3 a_Normal;
    layout (location = 2) in vec2 a_TexCoord;
    layout (location = 3) in vec3 a_Color;

    layout(std140) uniform SceneData {
        mat4 u_ViewMatrix;
        mat4 u_ProjectionMatrix;
        mat4 u_ViewProjectionMatrix;
        vec4 u_CameraPosition;
        vec4 u_LightDirection;
        vec4 u_LightColor;
        float u_Time;
    };

    layout(std140) uniform MaterialData {
        vec4 u_BaseColor;
        vec4 u_EmissiveColor;
        float u_Metallic;
        float u_Roughness;
    };

    out vec3 v_Color;
    out vec3 v_Normal;
    out vec3 v_WorldPos;
    out vec3 v_ViewPos;

    void main()
    {
        v_Color = a_Color;
        v_Normal = a_Normal;
        v_WorldPos = a_Position;
        v_ViewPos = u_CameraPosition.xyz;

        gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);
    }
)";

// Fragment shader source
const std::string fragmentShaderSrc = R"(
    #version 330 core
    out vec4 FragColor;

    in vec3 v_Color;
    in vec3 v_Normal;
    in vec3 v_WorldPos;
    in vec3 v_ViewPos;

    layout(std140) uniform SceneData {
        mat4 u_ViewMatrix;
        mat4 u_ProjectionMatrix;
        mat4 u_ViewProjectionMatrix;
        vec4 u_CameraPosition;
        vec4 u_LightDirection;
        vec4 u_LightColor;
        float u_Time;
    };

    layout(std140) uniform MaterialData {
        vec4 u_BaseColor;
        vec4 u_EmissiveColor;
        float u_Metallic;
        float u_Roughness;
    };

    void main()
    {
        // Normalize vectors
        vec3 normal = normalize(v_Normal);
        vec3 lightDir = normalize(-u_LightDirection.xyz);
        vec3 viewDir = normalize(v_ViewPos - v_WorldPos);

        // Basic lighting calculations
        float NdotL = max(dot(normal, lightDir), 0.0);

        // Diffuse lighting
        vec3 diffuse = u_LightColor.rgb * NdotL;

        // Ambient lighting
        vec3 ambient = u_LightColor.rgb * 0.2;

        // Specular lighting (simplified)
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = u_LightColor.rgb * spec * u_Metallic;

        // Combine lighting with vertex colors and material properties
        vec3 baseColor = v_Color * u_BaseColor.rgb;
        vec3 finalColor = baseColor * (ambient + diffuse) + specular + u_EmissiveColor.rgb;

        FragColor = vec4(finalColor, 1.0);
    }
)";

BasicRendering::BasicRendering()
    : Game(Pyramid::GraphicsAPI::OpenGL)
{
}

void BasicRendering::onCreate()
{
    PYRAMID_LOG_INFO("Basic Rendering Example starting up");

    Game::onCreate();

    auto device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_ERROR("Graphics device is null in BasicRendering::onCreate!");
        return;
    }

    auto* resources = GetResourceRegistry();
    if (!resources)
    {
        PYRAMID_LOG_ERROR("Resource registry is null in BasicRendering::onCreate!");
        return;
    }

    if (!SetupInputActions())
    {
        PYRAMID_LOG_ERROR("Failed to configure BasicRendering input actions");
        quit();
        return;
    }

    // Initialize all components
    InitializeShaders();
    CreateGeometry();

    const std::array<Pyramid::u8, 4> whitePixel = {255, 255, 255, 255};
    Pyramid::TextureResourceSpecification textureSpecification;
    textureSpecification.texture.Width = 1;
    textureSpecification.texture.Height = 1;
    textureSpecification.texture.Format = Pyramid::TextureFormat::RGBA8;
    textureSpecification.texture.GenerateMips = false;
    textureSpecification.texture.MinFilter = Pyramid::TextureFilter::Linear;
    textureSpecification.pixelData = whitePixel.data();
    textureSpecification.pixelDataSize = whitePixel.size();
    textureSpecification.colorSpace = Pyramid::TextureColorSpace::SRGB;
    textureSpecification.assetId =
        Pyramid::TextureAssetId::FromString("examples/basic-rendering/white");
    textureSpecification.name = "BasicRendering White";
    m_debugTexture = resources->Textures().GetOrCreate(textureSpecification);

    Pyramid::MaterialSpecification materialSpecification;
    materialSpecification.shader = m_shader;
    materialSpecification.textures = {
        {"u_AlbedoMap", 0, m_debugTexture}};
    materialSpecification.uniforms = {
        {"u_BaseColor", Pyramid::Math::Vec4(1.0f)},
        {"u_EmissiveColor", Pyramid::Math::Vec4(0.0f)},
        {"u_Metallic", 0.2f},
        {"u_Roughness", 0.65f}};
    materialSpecification.assetId =
        Pyramid::MaterialAssetId::FromString("examples/basic-rendering/material");
    materialSpecification.name = "BasicRendering Material";
    m_material = resources->Materials().GetOrCreate(materialSpecification);
    if (!m_material)
    {
        PYRAMID_LOG_ERROR("Failed to create the BasicRendering material");
        return;
    }

    SetupCamera();
    SetupUniformBuffers();

    PYRAMID_LOG_INFO("Basic Rendering Example initialized successfully!");
}

void BasicRendering::InitializeShaders()
{
    auto device = GetGraphicsDevice();
    if (!device)
        return;

    Pyramid::ShaderProgramSpecification specification;
    specification.vertexSource = vertexShaderSrc;
    specification.fragmentSource = fragmentShaderSrc;
    specification.name = "BasicRendering Scene";
    specification.assetId =
        Pyramid::ShaderAssetId::FromString("examples/basic-rendering/scene");

    auto* resources = GetResourceRegistry();
    if (!resources)
        return;

    m_shader = resources->Shaders().GetOrCreate(specification);
    if (!m_shader)
    {
        PYRAMID_LOG_ERROR("Failed to create or compile shader!");
        return;
    }

    PYRAMID_LOG_INFO("Shader compiled successfully");
}

void BasicRendering::CreateGeometry()
{
    auto device = GetGraphicsDevice();
    if (!device)
        return;

    // Define vertex structure
    struct Vertex
    {
        float Position[3];
        float Normal[3];
        float TexCoord[2];
        float Color[3];
    };

    // Create a colorful cube
    float size = 1.0f;
    Vertex vertices[] = {
        // Front face (Z+) - Red
        {{-size, -size, size}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{size, -size, size}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{size, size, size}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{-size, size, size}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

        // Back face (Z-) - Green
        {{-size, -size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{size, -size, -size}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{size, size, -size}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-size, size, -size}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},

        // Left face (X-) - Blue
        {{-size, -size, -size}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-size, -size, size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-size, size, size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        {{-size, size, -size}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},

        // Right face (X+) - Yellow
        {{size, -size, -size}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{size, -size, size}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{size, size, size}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
        {{size, size, -size}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},

        // Top face (Y+) - Magenta
        {{-size, size, -size}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
        {{-size, size, size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{size, size, size}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
        {{size, size, -size}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},

        // Bottom face (Y-) - Cyan
        {{-size, -size, -size}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
        {{-size, -size, size}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{size, -size, size}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{size, -size, -size}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}}};

    // Cube indices
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

    Pyramid::MeshSpecification specification;
    specification.vertexData = vertices;
    specification.vertexDataSize = sizeof(vertices);
    specification.vertexCount = 24;
    specification.layout = {
        {Pyramid::ShaderDataType::Float3, "a_Position"},
        {Pyramid::ShaderDataType::Float3, "a_Normal"},
        {Pyramid::ShaderDataType::Float2, "a_TexCoord"},
        {Pyramid::ShaderDataType::Float3, "a_Color"}};
    specification.indexData = indices;
    specification.indexCount = 36;
    specification.topology = Pyramid::PrimitiveTopology::Triangles;
    specification.name = "BasicRenderingCube";
    specification.assetId = Pyramid::MeshAssetId::FromString(
        "examples/basic-rendering/cube");

    auto* resources = GetResourceRegistry();
    m_mesh = resources
        ? resources->Meshes().GetOrCreate(specification)
        : nullptr;
    if (!m_mesh)
    {
        PYRAMID_LOG_ERROR("Failed to create cube mesh");
        return;
    }

    PYRAMID_LOG_INFO("Geometry created successfully");
}

void BasicRendering::SetupCamera()
{
    using namespace Pyramid::Math;

    // Create camera with proper parameters
    m_camera = std::make_unique<Pyramid::Camera>(
        Radians(60.0f), // FOV
        16.0f / 9.0f,   // Aspect ratio
        0.1f,           // Near plane
        100.0f          // Far plane
    );

    // Set an initial strategy-camera pose. The controller remains an optional
    // example utility; the Camera class itself has no RTS assumptions.
    m_camera->SetPosition(Vec3(0.0f, 6.0f, 8.0f));
    m_camera->LookAt(Vec3::Zero, Vec3::Up);

    Pyramid::RTSCameraActions controllerActions;
    controllerActions.move = {std::string(kCameraContext), std::string(kMoveAction)};
    controllerActions.orbitDelta = {std::string(kCameraContext), std::string(kOrbitDeltaAction)};
    controllerActions.orbitRate = {std::string(kCameraContext), std::string(kOrbitRateAction)};
    controllerActions.zoomDelta = {std::string(kCameraContext), std::string(kZoomDeltaAction)};
    controllerActions.zoomRate = {std::string(kCameraContext), std::string(kZoomRateAction)};
    controllerActions.boost = {std::string(kCameraContext), std::string(kBoostAction)};
    controllerActions.reset = {std::string(kCameraContext), std::string(kResetAction)};

    Pyramid::RTSCameraSettings controllerSettings;
    controllerSettings.movementSpeed = 4.0f;
    controllerSettings.orbitSensitivity = 0.005f;
    controllerSettings.zoomSensitivity = 1.0f;
    controllerSettings.minimumDistance = 2.0f;
    controllerSettings.maximumDistance = 40.0f;
    controllerSettings.distanceMovementScale = 0.04f;

    m_cameraController = std::make_unique<Pyramid::RTSCameraController>(
        Vec3::Zero,
        std::move(controllerActions),
        controllerSettings);
    m_cameraController->CaptureHome(*m_camera);
    SetActiveCamera(m_camera.get());

    PYRAMID_LOG_INFO("Camera setup completed");
}

void BasicRendering::SetupUniformBuffers()
{
    auto device = GetGraphicsDevice();
    if (!device)
        return;

    // Create uniform buffers
    m_sceneUBO = device->CreateUniformBuffer(sizeof(SceneUniforms));
    m_materialUBO = device->CreateUniformBuffer(sizeof(MaterialUniforms));

    if (!m_sceneUBO || !m_materialUBO)
    {
        PYRAMID_LOG_ERROR("Failed to create uniform buffers!");
        return;
    }

    // Initialize uniform buffer data
    SceneUniforms sceneData = {};
    sceneData.projectionMatrix = Pyramid::Math::Mat4::CreatePerspective(
        Pyramid::Math::Radians(60.0f), 16.0f / 9.0f, 0.1f, 100.0f);
    sceneData.lightDirection = Pyramid::Math::Vec4(0.5f, -1.0f, 0.5f, 0.0f);
    sceneData.lightColor = Pyramid::Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    sceneData.time = 0.0f;

    MaterialUniforms materialData = {};
    materialData.baseColor = Pyramid::Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData.emissiveColor = Pyramid::Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    materialData.metallic = 0.5f;
    materialData.roughness = 0.5f;

    // Upload initial data
    m_sceneUBO->UpdateData(&sceneData, sizeof(SceneUniforms));
    m_materialUBO->UpdateData(&materialData, sizeof(MaterialUniforms));

    // Bind uniform buffers to shader
    if (m_shader)
    {
        m_shader->Bind();
        m_shader->BindUniformBuffer("SceneData", m_sceneUBO.get(), 0);
        m_shader->BindUniformBuffer("MaterialData", m_materialUBO.get(), 1);
        m_shader->Unbind();
    }

    PYRAMID_LOG_INFO("Uniform buffers setup completed");
}

void BasicRendering::onUpdate(float deltaTime)
{
    Game::onUpdate(deltaTime);
    m_time += deltaTime;

    // Update input before uniform state so camera changes are visible during
    // the same rendered frame.
    HandleInput();
    if (m_camera && m_cameraController)
    {
        m_cameraController->Update(*m_camera, GetInputActions(), deltaTime);
    }
    UpdateUniformBuffers(deltaTime);
}

void BasicRendering::UpdateUniformBuffers(float deltaTime)
{
    (void)deltaTime;
    if (!m_sceneUBO || !m_materialUBO || !m_camera)
        return;

    // Update scene data
    SceneUniforms sceneData = {};
    sceneData.viewMatrix = m_camera->GetViewMatrix();
    sceneData.projectionMatrix = m_camera->GetProjectionMatrix();
    sceneData.viewProjectionMatrix = m_camera->GetViewProjectionMatrix();
    sceneData.cameraPosition = Pyramid::Math::Vec4(m_camera->GetPosition(), 1.0f);
    sceneData.lightDirection = Pyramid::Math::Vec4(
        sin(m_time * 0.5f) * 0.5f,
        -0.8f,
        cos(m_time * 0.5f) * 0.5f,
        0.0f);
    sceneData.lightColor = Pyramid::Math::Vec4(1.0f, 0.95f, 0.8f, 1.0f);
    sceneData.time = m_time;

    // Update material data with animated values
    MaterialUniforms materialData = {};
    materialData.baseColor = Pyramid::Math::Vec4(
        0.8f + 0.2f * sin(m_time * 0.7f),
        0.8f + 0.2f * sin(m_time * 0.9f + 2.0f),
        0.8f + 0.2f * sin(m_time * 1.1f + 4.0f),
        1.0f);
    materialData.emissiveColor = Pyramid::Math::Vec4(
        0.1f * sin(m_time * 2.0f),
        0.1f * sin(m_time * 1.5f + 1.0f),
        0.1f * sin(m_time * 1.8f + 3.0f),
        1.0f);
    materialData.metallic = 0.5f + 0.5f * sin(m_time * 0.4f);
    materialData.roughness = 0.5f + 0.4f * sin(m_time * 0.6f + 1.5f);

    // Upload updated data
    m_sceneUBO->UpdateData(&sceneData, sizeof(SceneUniforms));
    m_materialUBO->UpdateData(&materialData, sizeof(MaterialUniforms));
}

bool BasicRendering::SetupInputActions()
{
    auto* context = GetInputActions().CreateContext(
        std::string(kCameraContext),
        0,
        true);
    if (!context)
    {
        return false;
    }

    bool valid = true;
    valid = context->AddAction(std::string(kQuitAction), Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kQuitAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Escape)) && valid;

    valid = context->AddAction(std::string(kResetAction), Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kResetAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::R)) && valid;
    valid = context->AddBinding(
        kResetAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Num1)) && valid;

    valid = context->AddAction(std::string(kMoveAction), Pyramid::InputActionType::Axis2D) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::A, -1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Left, -1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::D, 1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Right, 1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::W, 1.0f, Pyramid::InputAxisComponent::Y)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Up, 1.0f, Pyramid::InputAxisComponent::Y)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::S, -1.0f, Pyramid::InputAxisComponent::Y)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Down, -1.0f, Pyramid::InputAxisComponent::Y)) && valid;

    valid = context->AddAction(std::string(kOrbitRateAction), Pyramid::InputActionType::Axis2D) && valid;
    valid = context->AddBinding(
        kOrbitRateAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Q, -1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kOrbitRateAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::E, 1.0f, Pyramid::InputAxisComponent::X)) && valid;

    valid = context->AddAction(std::string(kOrbitDeltaAction), Pyramid::InputActionType::Axis2D) && valid;
    auto orbitX = Pyramid::InputBinding::MouseDeltaXBinding(
        1.0f,
        Pyramid::InputAxisComponent::X);
    orbitX.RequireMouseButton(Pyramid::MouseButton::Right);
    auto orbitY = Pyramid::InputBinding::MouseDeltaYBinding(
        -1.0f,
        Pyramid::InputAxisComponent::Y);
    orbitY.RequireMouseButton(Pyramid::MouseButton::Right);
    valid = context->AddBinding(kOrbitDeltaAction, orbitX) && valid;
    valid = context->AddBinding(kOrbitDeltaAction, orbitY) && valid;

    valid = context->AddAction(std::string(kZoomDeltaAction), Pyramid::InputActionType::Axis1D) && valid;
    valid = context->AddBinding(
        kZoomDeltaAction,
        Pyramid::InputBinding::MouseWheelBinding(-1.0f)) && valid;

    valid = context->AddAction(std::string(kZoomRateAction), Pyramid::InputActionType::Axis1D) && valid;
    valid = context->AddBinding(
        kZoomRateAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::PageUp, -1.0f)) && valid;
    valid = context->AddBinding(
        kZoomRateAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::PageDown, 1.0f)) && valid;

    valid = context->AddAction(std::string(kBoostAction), Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kBoostAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::LeftShift)) && valid;
    valid = context->AddBinding(
        kBoostAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::RightShift)) && valid;

    return valid;
}

void BasicRendering::HandleInput()
{
    if (GetInputActions().WasActionPressed(kCameraContext, kQuitAction))
    {
        quit();
    }
}

void BasicRendering::onRender()
{
    auto device = GetGraphicsDevice();
    if (!device)
        return;

    // Set clear color and clear buffers
    device->SetClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    device->Clear(Pyramid::Color(0.1f, 0.1f, 0.2f, 1.0f));

    // Enable depth testing
    device->EnableDepthTest(true);
    device->SetDepthFunc(0x0201); // GL_LESS

    // Render the scene
    RenderScene();

    Game::onRender();
}

void BasicRendering::RenderScene()
{
    if (!m_material || !m_mesh || !m_sceneUBO || !m_materialUBO)
        return;

    auto device = GetGraphicsDevice();
    if (!device)
        return;

    // Apply the engine-owned material and bind uniform buffers.
    m_material->Apply(*device);
    m_sceneUBO->Bind(0);
    m_materialUBO->Bind(1);

    // Bind the engine-owned mesh and draw with its immutable metadata.
    m_mesh->Bind();
    if (m_mesh->IsIndexed())
    {
        device->DrawIndexed(m_mesh->GetIndexCount(), m_mesh->GetTopology());
    }
    else
    {
        device->DrawArrays(m_mesh->GetVertexCount(), 0, m_mesh->GetTopology());
    }
    m_mesh->Unbind();

    m_material->GetShader()->Unbind();

    // Log debug info periodically
    static float debugTimer = 0.0f;
    debugTimer += 0.016f; // Approximate frame time
    if (debugTimer >= 5.0f)
    {
        debugTimer = 0.0f;
        if (m_camera)
        {
            auto pos = m_camera->GetPosition();
            PYRAMID_LOG_INFO("Rendering: Camera at (", pos.x, ", ", pos.y, ", ", pos.z, ")");
        }
    }
}