#pragma once
#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/Buffer/UniformBuffer.hpp>
#include <Pyramid/Graphics/Texture.hpp>
#include <Pyramid/Math/Math.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/CameraController.hpp>
#include <memory>

/**
 * @brief Basic Rendering Example for Pyramid Engine
 *
 * This example demonstrates the fundamental rendering capabilities of the Pyramid Engine:
 * - Setting up a basic rendering pipeline
 * - Creating and rendering 3D geometry
 * - Using shaders and uniform buffers
 * - Implementing camera controls
 * - Basic lighting calculations
 *
 * This example is designed to be simple and educational, making it easy for
 * new users to understand the core concepts of the engine.
 */
class BasicRendering : public Pyramid::Game
{
public:
    explicit BasicRendering();
    virtual ~BasicRendering() = default;

protected:
    void onCreate() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;

private:
    // Initialization methods
    void InitializeShaders();
    void CreateGeometry();
    void SetupCamera();
    void SetupUniformBuffers();
    bool SetupInputActions();

    // Update methods
    void UpdateUniformBuffers(float deltaTime);

    // Rendering methods
    void RenderScene();

    // Input handling
    void HandleInput();

    // Core rendering components
    std::shared_ptr<Pyramid::ShaderProgram> m_shader;
    std::shared_ptr<Pyramid::Mesh> m_mesh;
    std::shared_ptr<Pyramid::Material> m_material;
    std::shared_ptr<Pyramid::IUniformBuffer> m_sceneUBO;

    // Camera system
    std::unique_ptr<Pyramid::Camera> m_camera;
    std::unique_ptr<Pyramid::RTSCameraController> m_cameraController;

    // Timing and animation
    float m_time = 0.0f;

    // Uniform data structures
    struct SceneUniforms
    {
        Pyramid::Math::Mat4 viewMatrix;
        Pyramid::Math::Mat4 projectionMatrix;
        Pyramid::Math::Mat4 viewProjectionMatrix;
        Pyramid::Math::Vec4 cameraPosition;
        Pyramid::Math::Vec4 lightDirection;
        Pyramid::Math::Vec4 lightColor;
        float time;
        float padding[3]; // Ensure 16-byte alignment
    };

};