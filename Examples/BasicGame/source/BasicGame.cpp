#include "BasicGame.hpp"

#include <Pyramid/Graphics/Buffer/BufferLayout.hpp>
#include <Pyramid/Graphics/Geometry/Vertex.hpp>
#include <Pyramid/Graphics/GraphicsDevice.hpp>
#include <Pyramid/Graphics/Shader/Shader.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/UI/UIRenderer.hpp>
#include <Pyramid/Input/InputActions.hpp>
#include <Pyramid/Graphics/CameraController.hpp>
#include <Pyramid/Examples/RTSReference/RTSInteractionController.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Util/Log.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>
#include <string_view>
#include <utility>
#include <iomanip>
#include <sstream>
#include <string>

namespace
{
    constexpr std::string_view kInputContext = "basic-game";
    constexpr std::string_view kQuitAction = "Quit";
    constexpr std::string_view kToggleAnimationAction = "ToggleAnimation";
    constexpr std::string_view kResetCameraAction = "ResetCamera";
    constexpr std::string_view kMoveAction = "Move";
    constexpr std::string_view kOrbitDeltaAction = "OrbitDelta";
    constexpr std::string_view kOrbitRateAction = "OrbitRate";
    constexpr std::string_view kZoomDeltaAction = "ZoomDelta";
    constexpr std::string_view kBoostAction = "Boost";
    constexpr std::string_view kSelectAction = "Select";
    constexpr std::string_view kCommandAction = "Command";
    constexpr std::string_view kToggleDebugUIAction = "ToggleDebugUI";

    std::string FormatFloat(float value, int precision = 2)
    {
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(precision) << value;
        return stream.str();
    }

    std::string FormatCount(Pyramid::u64 value)
    {
        return std::to_string(value);
    }

    constexpr const char* kForwardVertexShader = R"(
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

uniform mat4 u_Model;
uniform mat4 u_ViewProjection;
uniform mat4 u_NormalMatrix;

out vec4 v_Color;
out vec3 v_Normal;

void main()
{
    vec4 worldPosition = u_Model * vec4(a_Position, 1.0);
    vec3 localNormal = normalize(a_Position);

    v_Color = a_Color;
    v_Normal = normalize((u_NormalMatrix * vec4(localNormal, 0.0)).xyz);

    gl_Position = u_ViewProjection * worldPosition;
}
)";

    constexpr const char* kForwardFragmentShader = R"(
#version 330 core

in vec4 v_Color;
in vec3 v_Normal;

uniform vec4 u_AlbedoColor;

out vec4 FragColor;

void main()
{
    vec3 lightDirection = normalize(vec3(0.5, 1.0, 0.25));
    float ndotl = max(dot(normalize(v_Normal), lightDirection), 0.12);
    vec3 litColor = v_Color.rgb * u_AlbedoColor.rgb * ndotl;

    FragColor = vec4(litColor, 1.0);
}
)";
}

BasicGame::BasicGame()
    : Game(Pyramid::GraphicsAPI::OpenGL)
{
}

BasicGame::~BasicGame()
{
    UnregisterUIContext(&m_debugUI);
}

void BasicGame::onCreate()
{
    Game::onCreate();
    if (!IsInitialized())
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: engine initialization failed.");
        quit();
        return;
    }

    if (!SetupInputActions())
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: input action setup failed.");
        quit();
        return;
    }

    auto* device = GetGraphicsDevice();
    if (!device)
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: graphics device is null.");
        quit();
        return;
    }

    auto* resources = GetResourceRegistry();
    if (!resources)
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: resource registry is null.");
        quit();
        return;
    }

    m_renderSystem = std::make_unique<Pyramid::Renderer::RenderSystem>();
    if (!m_renderSystem->Initialize(device))
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: render system initialization failed.");
        quit();
        return;
    }
    SetRenderSystem(m_renderSystem.get());

    m_uiRenderer = std::make_unique<Pyramid::UIRenderer>();
    if (!m_uiRenderer->Initialize(*device, *resources, m_debugUI.GetDebugFont()))
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: UI renderer initialization failed.");
        quit();
        return;
    }
    RegisterUIContext(&m_debugUI);

    m_scene = std::make_shared<Pyramid::Scene>("BasicGame Scene");
    m_sceneManager = std::make_unique<Pyramid::SceneManagement::SceneManager>();
    m_sceneManager->SetActiveScene(m_scene);

    m_camera = std::make_unique<Pyramid::Camera>(
        Pyramid::Math::Radians(60.0f),
        1280.0f / 720.0f,
        0.1f,
        200.0f);

    m_camera->SetPosition(Pyramid::Math::Vec3(0.0f, 7.0710678f, 7.0710678f));
    m_camera->LookAt(Pyramid::Math::Vec3::Zero);

    Pyramid::RTSCameraActions cameraActions;
    cameraActions.move = {std::string(kInputContext), std::string(kMoveAction)};
    cameraActions.orbitDelta = {std::string(kInputContext), std::string(kOrbitDeltaAction)};
    cameraActions.orbitRate = {std::string(kInputContext), std::string(kOrbitRateAction)};
    cameraActions.zoomDelta = {std::string(kInputContext), std::string(kZoomDeltaAction)};
    cameraActions.zoomRate = {};
    cameraActions.boost = {std::string(kInputContext), std::string(kBoostAction)};
    cameraActions.reset = {std::string(kInputContext), std::string(kResetCameraAction)};

    Pyramid::RTSCameraSettings cameraSettings;
    cameraSettings.movementSpeed = 4.0f;
    cameraSettings.orbitSensitivity = 0.005f;
    cameraSettings.zoomSensitivity = 0.75f;
    cameraSettings.minimumDistance = 3.0f;
    cameraSettings.maximumDistance = 30.0f;
    cameraSettings.distanceMovementScale = 0.04f;
    m_cameraController = std::make_unique<Pyramid::RTSCameraController>(
        Pyramid::Math::Vec3::Zero,
        std::move(cameraActions),
        cameraSettings);
    m_cameraController->CaptureHome(*m_camera);

    Pyramid::Examples::RTSReference::RTSInteractionActions interactionActions;
    interactionActions.select = {std::string(kInputContext), std::string(kSelectAction)};
    interactionActions.command = {std::string(kInputContext), std::string(kCommandAction)};

    Pyramid::Examples::RTSReference::RTSInteractionSettings interactionSettings;
    interactionSettings.edgeMarginPixels = 18.0f;
    interactionSettings.edgeScrollSpeed = 4.0f;
    interactionSettings.edgeScrollDistanceScale = 0.04f;
    interactionSettings.maximumRayDistance = 200.0f;
    m_interactionController =
        std::make_unique<Pyramid::Examples::RTSReference::RTSInteractionController>(
            std::move(interactionActions),
            interactionSettings);
    m_interactionController->SetViewportSize(1280, 720);

    SetActiveCamera(m_camera.get());

    Pyramid::ShaderProgramSpecification shaderSpecification;
    shaderSpecification.vertexSource = kForwardVertexShader;
    shaderSpecification.fragmentSource = kForwardFragmentShader;
    shaderSpecification.name = "BasicGame Forward";
    shaderSpecification.assetId =
        Pyramid::ShaderAssetId::FromString("examples/basic-game/forward");

    m_shader = resources->Shaders().GetOrCreate(shaderSpecification);
    if (!m_shader)
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: failed to compile scene shader.");
        quit();
        return;
    }

    const std::array<Pyramid::u8, 16> checkerPixels = {
        255, 255, 255, 255, 40, 40, 40, 255,
        40, 40, 40, 255, 255, 255, 255, 255};
    Pyramid::TextureResourceSpecification textureSpecification;
    textureSpecification.texture.Width = 2;
    textureSpecification.texture.Height = 2;
    textureSpecification.texture.Format = Pyramid::TextureFormat::RGBA8;
    textureSpecification.texture.GenerateMips = false;
    textureSpecification.texture.MinFilter = Pyramid::TextureFilter::Nearest;
    textureSpecification.texture.MagFilter = Pyramid::TextureFilter::Nearest;
    textureSpecification.pixelData = checkerPixels.data();
    textureSpecification.pixelDataSize = checkerPixels.size();
    textureSpecification.colorSpace = Pyramid::TextureColorSpace::SRGB;
    textureSpecification.assetId =
        Pyramid::TextureAssetId::FromString("examples/basic-game/checker");
    textureSpecification.name = "BasicGame Checker";
    m_debugTexture = resources->Textures().GetOrCreate(textureSpecification);
    if (!m_debugTexture)
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: failed to create cached checker texture.");
        quit();
        return;
    }

    if (!SetupScene())
    {
        PYRAMID_LOG_CRITICAL("BasicGame aborted: failed to create scene objects.");
        quit();
        return;
    }

    const Pyramid::EntityId selectableUnit = m_cubeEntity.GetId();
    m_interactionController->SetSelectablePredicate(
        [selectableUnit](const Pyramid::Entity& entity)
        {
            return entity.GetId() == selectableUnit;
        });

    Pyramid::Entity sun = m_scene->CreateEntity("Sun");
    Pyramid::LightComponent sunLight;
    sunLight.type = Pyramid::LightType::Directional;
    sunLight.localDirection = Pyramid::Math::Vec3(-0.5f, -1.0f, -0.25f);
    sunLight.intensity = 1.5f;
    sun.SetLight(sunLight);
    m_scene->SetPrimaryLight(sun);

    auto& environment = m_scene->GetEnvironment();
    environment.skyColor = Pyramid::Math::Vec3(0.09f, 0.12f, 0.18f);
    environment.ambientColor = Pyramid::Math::Vec3(0.12f, 0.12f, 0.14f);

    PYRAMID_LOG_INFO(
        "BasicGame ready: RTS reference input active (edge scroll, selection, commands).");
}

void BasicGame::onUpdate(float deltaTime)
{
    Game::onUpdate(deltaTime);

    const auto& actions = GetInputActions();
    if (actions.WasActionPressed(kInputContext, kQuitAction))
    {
        quit();
        return;
    }

    if (actions.WasActionPressed(kInputContext, kToggleDebugUIAction))
    {
        m_debugUIVisible = !m_debugUIVisible;
        m_debugUI.SetEnabled(m_debugUIVisible);
    }

    if (actions.WasActionPressed(kInputContext, kToggleAnimationAction))
    {
        m_animationPaused = !m_animationPaused;
        PYRAMID_LOG_INFO(
            "Cube animation ",
            m_animationPaused ? "paused" : "resumed");
    }

    if (!m_animationPaused)
    {
        m_elapsedTime += deltaTime;
    }

    if (m_cubeEntity)
    {
        m_cubeEntity.SetLocalRotation(Pyramid::Math::Quat::FromEuler(
            0.35f * m_elapsedTime,
            0.8f * m_elapsedTime,
            0.0f));
    }

    if (m_camera && m_cameraController && m_interactionController && m_sceneManager)
    {
        m_sceneManager->Update(deltaTime);
        m_interactionController->Update(
            GetInput(),
            GetInputActions(),
            *m_camera,
            *m_sceneManager,
            *m_cameraController,
            deltaTime);

        const Pyramid::EntityId selected =
            m_interactionController->GetSelectedEntityId();
        if (selected != m_reportedSelection)
        {
            if (selected)
            {
                PYRAMID_LOG_INFO("Selected entity ", selected.GetValue());
            }
            else
            {
                PYRAMID_LOG_INFO("Selection cleared");
            }
            m_reportedSelection = selected;
        }

        if (const auto command = m_interactionController->ConsumeCommand())
        {
            PYRAMID_LOG_INFO(
                "Command requested for entity ", command->entity.GetValue(),
                " at (", command->target.x, ", ", command->target.y, ", ",
                command->target.z, ")");
        }
    }

    BuildDebugUI(deltaTime);
}

void BasicGame::onRender()
{
    if (!m_renderSystem || !m_scene || !m_camera)
    {
        Game::onRender();
        return;
    }

    m_renderSystem->BeginFrame();
    m_renderSystem->Render(*m_scene, *m_camera);
    if (m_uiRenderer && m_debugUIVisible)
    {
        (void)m_uiRenderer->Render(m_debugUI.GetDrawList(), m_uiFrame);
    }
    m_renderSystem->EndFrame();
}

void BasicGame::BuildDebugUI(float deltaTime)
{
    if (!m_debugUIVisible)
    {
        return;
    }

    m_smoothedFrameTime += (deltaTime - m_smoothedFrameTime) * 0.1f;
    m_uiFrame.deltaTime = deltaTime;
    if (!m_debugUI.BeginFrame(m_uiFrame, GetInput()))
    {
        return;
    }

    Pyramid::UI::PanelOptions diagnostics;
    diagnostics.position = Pyramid::Math::Vec2(12.0f, 12.0f);
    diagnostics.size = Pyramid::Math::Vec2(
        350.0f,
        (std::min)(470.0f, (std::max)(200.0f, m_uiFrame.height - 24.0f)));
    if (m_debugUI.BeginPanel("PYRAMID DEBUG  [F1]", diagnostics))
    {
        const float fps = m_smoothedFrameTime > 0.000001f
            ? 1.0f / m_smoothedFrameTime
            : 0.0f;
        m_debugUI.LabelValue("FPS", FormatFloat(fps, 1));
        m_debugUI.LabelValue("FRAME MS", FormatFloat(m_smoothedFrameTime * 1000.0f, 2));
        m_debugUI.ProgressBar(
            "16.67 MS BUDGET",
            m_smoothedFrameTime / (1.0f / 60.0f));

        m_debugUI.Separator();
        if (m_renderSystem)
        {
            const auto& renderStats = m_renderSystem->GetStats();
            m_debugUI.LabelValue("DRAW CALLS", FormatCount(renderStats.drawCalls));
            m_debugUI.LabelValue("TRIANGLES", FormatCount(renderStats.triangles));
            m_debugUI.LabelValue("RENDER MS", FormatFloat(renderStats.frameTime, 2));
        }

        if (auto* resources = GetResourceRegistry())
        {
            const auto stats = resources->GetStats();
            m_debugUI.Separator();
            m_debugUI.Label("RESOURCES");
            m_debugUI.LabelValue("MESHES", FormatCount(stats.meshes.residentMeshes));
            m_debugUI.LabelValue("TEXTURES", FormatCount(stats.textures.residentTextures));
            m_debugUI.LabelValue("SHADERS", FormatCount(stats.shaders.residentPrograms));
            m_debugUI.LabelValue("MATERIALS", FormatCount(stats.materials.residentMaterials));
        }

        m_debugUI.Separator();
        const auto mouse = GetInput().GetMousePosition();
        m_debugUI.LabelValue(
            "MOUSE",
            FormatFloat(mouse.x, 0) + ", " + FormatFloat(mouse.y, 0));
        m_debugUI.LabelValue("WHEEL", FormatFloat(GetInput().GetMouseWheelDelta(), 1));
        m_debugUI.LabelValue(
            "UI CAPTURE",
            m_debugUI.WantsPointerInput() ? "POINTER" : "NONE");
        m_debugUI.EndPanel();
    }

    Pyramid::UI::PanelOptions controls;
    controls.position = Pyramid::Math::Vec2(374.0f, 12.0f);
    controls.size = Pyramid::Math::Vec2(
        300.0f,
        (std::min)(260.0f, (std::max)(180.0f, m_uiFrame.height - 24.0f)));
    if (m_debugUI.BeginPanel("RUNTIME CONTROLS", controls))
    {
        (void)m_debugUI.Checkbox("PAUSE ANIMATION", m_animationPaused);
        if (m_cameraController)
        {
            auto settings = m_cameraController->GetSettings();
            float movementSpeed = settings.movementSpeed;
            if (m_debugUI.SliderFloat(
                    "CAMERA SPEED",
                    movementSpeed,
                    1.0f,
                    20.0f))
            {
                settings.movementSpeed = movementSpeed;
                (void)m_cameraController->SetSettings(settings);
            }
            if (m_debugUI.Button("RESET CAMERA") && m_camera)
            {
                m_cameraController->Reset(*m_camera);
            }
        }

        const auto uiStats = m_debugUI.GetStats();
        m_debugUI.Separator();
        m_debugUI.LabelValue("ELEMENTS", FormatCount(uiStats.retainedElements));
        m_debugUI.LabelValue("VERTICES", FormatCount(uiStats.vertices));
        m_debugUI.LabelValue("BATCHES", FormatCount(uiStats.batches));
        m_debugUI.EndPanel();
    }
    (void)m_debugUI.EndFrame();
}

std::shared_ptr<Pyramid::Mesh> BasicGame::CreateColoredCube(float size)
{
    auto* device = GetGraphicsDevice();
    if (!device)
    {
        return nullptr;
    }

    const float s = size * 0.5f;
    const std::vector<Pyramid::Vertex> vertices = {
        {-s, -s, -s, 1.0f, 0.2f, 0.2f, 1.0f},
        {s, -s, -s, 0.2f, 1.0f, 0.2f, 1.0f},
        {s, s, -s, 0.2f, 0.2f, 1.0f, 1.0f},
        {-s, s, -s, 1.0f, 1.0f, 0.2f, 1.0f},
        {-s, -s, s, 1.0f, 0.2f, 1.0f, 1.0f},
        {s, -s, s, 0.2f, 1.0f, 1.0f, 1.0f},
        {s, s, s, 1.0f, 1.0f, 1.0f, 1.0f},
        {-s, s, s, 1.0f, 0.6f, 0.2f, 1.0f},
    };

    const std::vector<Pyramid::u32> indices = {
        0, 2, 1, 0, 3, 2,
        4, 5, 6, 4, 6, 7,
        0, 7, 3, 0, 4, 7,
        1, 2, 6, 1, 6, 5,
        3, 7, 6, 3, 6, 2,
        0, 1, 5, 0, 5, 4,
    };

    Pyramid::MeshSpecification specification;
    specification.vertexData = vertices.data();
    specification.vertexDataSize =
        static_cast<Pyramid::u32>(vertices.size() * sizeof(Pyramid::Vertex));
    specification.vertexCount = static_cast<Pyramid::u32>(vertices.size());
    specification.layout = {
        {Pyramid::ShaderDataType::Float3, "a_Position"},
        {Pyramid::ShaderDataType::Float4, "a_Color"},
    };
    specification.indexData = indices.data();
    specification.indexCount = static_cast<Pyramid::u32>(indices.size());
    specification.topology = Pyramid::PrimitiveTopology::Triangles;
    specification.name = "ColoredCube";

    auto* resources = GetResourceRegistry();
    return resources
        ? resources->Meshes().GetOrCreate(specification)
        : nullptr;
}

bool BasicGame::SetupScene()
{
    if (!m_scene || !m_shader)
    {
        return false;
    }

    auto cubeGeometry = CreateColoredCube(1.5f);
    auto floorGeometry = CreateColoredCube(1.5f);
    if (!cubeGeometry || !floorGeometry || cubeGeometry != floorGeometry)
    {
        return false;
    }

    Pyramid::MaterialSpecification cubeMaterialSpecification;
    cubeMaterialSpecification.shader = m_shader;
    cubeMaterialSpecification.uniforms = {
        {"u_AlbedoColor", Pyramid::Math::Vec4(1.0f, 1.0f, 1.0f, 1.0f)}};
    cubeMaterialSpecification.assetId =
        Pyramid::MaterialAssetId::FromString("examples/basic-game/cube-material");
    cubeMaterialSpecification.name = "BasicGame Cube Material";
    auto* resources = GetResourceRegistry();
    if (!resources)
    {
        return false;
    }

    m_cubeMaterial = resources->Materials().GetOrCreate(cubeMaterialSpecification);

    Pyramid::MaterialSpecification floorMaterialSpecification = cubeMaterialSpecification;
    floorMaterialSpecification.uniforms = {
        {"u_AlbedoColor", Pyramid::Math::Vec4(0.35f, 0.42f, 0.55f, 1.0f)}};
    floorMaterialSpecification.assetId =
        Pyramid::MaterialAssetId::FromString("examples/basic-game/floor-material");
    floorMaterialSpecification.name = "BasicGame Floor Material";
    m_floorMaterial = resources->Materials().GetOrCreate(floorMaterialSpecification);

    if (!m_cubeMaterial || !m_floorMaterial)
    {
        return false;
    }

    const auto cubeMeshHandle = resources->GetHandle(cubeGeometry->GetAssetId());
    const auto floorMeshHandle = resources->GetHandle(floorGeometry->GetAssetId());
    const auto cubeMaterialHandle = resources->GetHandle(m_cubeMaterial->GetAssetId());
    const auto floorMaterialHandle = resources->GetHandle(m_floorMaterial->GetAssetId());

    m_cubeEntity = m_scene->CreateEntity("DemoCube");
    Pyramid::MeshRendererComponent cubeRenderer;
    cubeRenderer.mesh = cubeMeshHandle;
    cubeRenderer.material = cubeMaterialHandle;
    if (!m_cubeEntity.SetMeshRenderer(cubeRenderer, resources))
    {
        return false;
    }

    Pyramid::Entity floorEntity = m_scene->CreateEntity("Floor");
    floorEntity.SetLocalTransform(
        Pyramid::Math::Vec3(0.0f, -1.2f, 0.0f),
        Pyramid::Math::Quat::Identity,
        Pyramid::Math::Vec3(6.0f, 0.15f, 6.0f));
    Pyramid::MeshRendererComponent floorRenderer;
    floorRenderer.mesh = floorMeshHandle;
    floorRenderer.material = floorMaterialHandle;
    if (!floorEntity.SetMeshRenderer(floorRenderer, resources))
    {
        return false;
    }

    return true;
}

bool BasicGame::SetupInputActions()
{
    auto* context = GetInputActions().CreateContext(std::string(kInputContext), 0, true);
    if (!context)
    {
        return false;
    }

    bool valid = true;
    valid = context->AddAction(std::string(kQuitAction), Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kQuitAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Escape)) && valid;
    valid = context->AddAction(
        std::string(kToggleDebugUIAction),
        Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kToggleDebugUIAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::F1)) && valid;
    valid = context->AddAction(
        std::string(kToggleAnimationAction),
        Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kToggleAnimationAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Space)) && valid;
    valid = context->AddAction(
        std::string(kResetCameraAction),
        Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kResetCameraAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::R)) && valid;

    valid = context->AddAction(std::string(kMoveAction), Pyramid::InputActionType::Axis2D) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::A, -1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::D, 1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::W, 1.0f, Pyramid::InputAxisComponent::Y)) && valid;
    valid = context->AddBinding(
        kMoveAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::S, -1.0f, Pyramid::InputAxisComponent::Y)) && valid;

    valid = context->AddAction(std::string(kOrbitDeltaAction), Pyramid::InputActionType::Axis2D) && valid;
    auto orbitX = Pyramid::InputBinding::MouseDeltaXBinding(
        1.0f,
        Pyramid::InputAxisComponent::X);
    orbitX.RequireMouseButton(Pyramid::MouseButton::Middle);
    auto orbitY = Pyramid::InputBinding::MouseDeltaYBinding(
        -1.0f,
        Pyramid::InputAxisComponent::Y);
    orbitY.RequireMouseButton(Pyramid::MouseButton::Middle);
    valid = context->AddBinding(kOrbitDeltaAction, orbitX) && valid;
    valid = context->AddBinding(kOrbitDeltaAction, orbitY) && valid;

    valid = context->AddAction(std::string(kOrbitRateAction), Pyramid::InputActionType::Axis2D) && valid;
    valid = context->AddBinding(
        kOrbitRateAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::Q, -1.0f, Pyramid::InputAxisComponent::X)) && valid;
    valid = context->AddBinding(
        kOrbitRateAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::E, 1.0f, Pyramid::InputAxisComponent::X)) && valid;

    valid = context->AddAction(std::string(kZoomDeltaAction), Pyramid::InputActionType::Axis1D) && valid;
    valid = context->AddBinding(
        kZoomDeltaAction,
        Pyramid::InputBinding::MouseWheelBinding(-1.0f)) && valid;

    valid = context->AddAction(std::string(kBoostAction), Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kBoostAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::LeftShift)) && valid;
    valid = context->AddBinding(
        kBoostAction,
        Pyramid::InputBinding::KeyBinding(Pyramid::Key::RightShift)) && valid;

    valid = context->AddAction(std::string(kSelectAction), Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kSelectAction,
        Pyramid::InputBinding::MouseButtonBinding(Pyramid::MouseButton::Left)) && valid;

    valid = context->AddAction(std::string(kCommandAction), Pyramid::InputActionType::Button) && valid;
    valid = context->AddBinding(
        kCommandAction,
        Pyramid::InputBinding::MouseButtonBinding(Pyramid::MouseButton::Right)) && valid;

    return valid;
}

void BasicGame::onWindowResize(const Pyramid::WindowResizeEvent& event)
{
    Game::onWindowResize(event);
    if (m_interactionController && event.HasRenderableArea())
    {
        m_interactionController->SetViewportSize(
            static_cast<Pyramid::u32>(event.width),
            static_cast<Pyramid::u32>(event.height));
    }
    if (event.HasRenderableArea())
    {
        m_uiFrame.width = static_cast<float>(event.width);
        m_uiFrame.height = static_cast<float>(event.height);
    }
}
