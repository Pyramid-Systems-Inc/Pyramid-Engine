#pragma once

#include <Pyramid/Core/Game.hpp>
#include <Pyramid/Examples/RTSReference/RTSInteractionController.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Graphics/CameraController.hpp>
#include <Pyramid/Graphics/Material/Material.hpp>
#include <Pyramid/Graphics/Resources/ResourceRegistry.hpp>
#include <Pyramid/Graphics/Renderer/RenderSystem.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Scene/SceneManager.hpp>
#include <Pyramid/Graphics/UI/UIRenderer.hpp>
#include <Pyramid/UI/UI.hpp>
#include <Pyramid/UI/GameUI.hpp>

#include <memory>

namespace Pyramid
{
    class Mesh;
    class ShaderProgram;
    class TextureResource;
    class Material;
}

class BasicGame final : public Pyramid::Game
{
public:
    BasicGame();
    ~BasicGame() override;

protected:
    void onCreate() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onWindowResize(const Pyramid::WindowResizeEvent& event) override;

private:
    std::shared_ptr<Pyramid::Mesh> CreateColoredCube(float size);
    bool SetupScene();
    bool SetupInputActions();
    void BuildDebugUI(float deltaTime);
    void BuildGameUI(float deltaTime);
    void ShowMainMenu();
    void StartGameplay();
    void OpenPauseMenu();
    void ResumeGameplay();
    void ReturnToMainMenu();

    std::unique_ptr<Pyramid::Renderer::RenderSystem> m_renderSystem;
    std::unique_ptr<Pyramid::UIRenderer> m_uiRenderer;
    Pyramid::UI::Context m_gameUI;
    Pyramid::UI::Context m_debugUI;
    Pyramid::UI::ScreenStack m_gameScreens;
    std::shared_ptr<Pyramid::Scene> m_scene;
    std::unique_ptr<Pyramid::SceneManagement::SceneManager> m_sceneManager;
    std::unique_ptr<Pyramid::Camera> m_camera;
    std::unique_ptr<Pyramid::RTSCameraController> m_cameraController;
    std::unique_ptr<Pyramid::Examples::RTSReference::RTSInteractionController>
        m_interactionController;
    Pyramid::Entity m_cubeEntity;
    Pyramid::EntityId m_reportedSelection;
    std::shared_ptr<Pyramid::ShaderProgram> m_shader;
    std::shared_ptr<Pyramid::TextureResource> m_debugTexture;
    std::shared_ptr<Pyramid::Material> m_cubeMaterial;
    std::shared_ptr<Pyramid::Material> m_floorMaterial;

    Pyramid::UI::FrameInfo m_uiFrame{1280.0f, 720.0f, 1.0f, 0.0f};
    float m_elapsedTime = 0.0f;
    float m_smoothedFrameTime = 1.0f / 60.0f;
    bool m_debugUIVisible = false;
    bool m_gameplayStarted = false;
    bool m_animationPaused = false;
    bool m_performanceSectionOpen = true;
    bool m_resourcesSectionOpen = true;
    bool m_inputSectionOpen = true;
    bool m_uiSectionOpen = true;
};
