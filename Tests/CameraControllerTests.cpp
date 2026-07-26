#include <Pyramid/Graphics/CameraController.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "CameraController test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    bool NearlyEqual(Pyramid::f32 left, Pyramid::f32 right, Pyramid::f32 tolerance = 0.001f)
    {
        return std::fabs(left - right) <= tolerance;
    }

    bool NearlyEqual(
        const Pyramid::Math::Vec3& left,
        const Pyramid::Math::Vec3& right,
        Pyramid::f32 tolerance = 0.001f)
    {
        return NearlyEqual(left.x, right.x, tolerance) &&
            NearlyEqual(left.y, right.y, tolerance) &&
            NearlyEqual(left.z, right.z, tolerance);
    }

    void ReleaseAll(Pyramid::InputState& input)
    {
        using Pyramid::Key;
        input.ProcessKey(Key::W, false);
        input.ProcessKey(Key::D, false);
        input.ProcessKey(Key::E, false);
        input.ProcessKey(Key::R, false);
        input.ProcessKey(Key::Right, false);
        input.ProcessKey(Key::Up, false);
        input.ProcessKey(Key::LeftShift, false);
    }
}

int main()
{
    using namespace Pyramid;

    InputState input;
    input.SetFocused(true);
    InputActionSystem actions;

    InputContext* fly = actions.CreateContext("fly");
    Require(fly != nullptr, "free-fly context creation failed");
    Require(fly->AddAction("Move", InputActionType::Axis2D), "free-fly Move creation failed");
    Require(fly->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::D, 1.0f, InputAxisComponent::X)),
        "free-fly D binding failed");
    Require(fly->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::W, 1.0f, InputAxisComponent::Y)),
        "free-fly W binding failed");
    Require(fly->AddAction("Elevate", InputActionType::Axis1D), "Elevate creation failed");
    Require(fly->AddBinding("Elevate", InputBinding::KeyBinding(Key::E)),
        "Elevate binding failed");
    Require(fly->AddAction("LookRate", InputActionType::Axis2D), "LookRate creation failed");
    Require(fly->AddBinding(
        "LookRate",
        InputBinding::KeyBinding(Key::Right, 1.0f, InputAxisComponent::X)),
        "LookRate X binding failed");
    Require(fly->AddBinding(
        "LookRate",
        InputBinding::KeyBinding(Key::Up, 1.0f, InputAxisComponent::Y)),
        "LookRate Y binding failed");
    Require(fly->AddAction("Boost", InputActionType::Button), "Boost creation failed");
    Require(fly->AddBinding("Boost", InputBinding::KeyBinding(Key::LeftShift)),
        "Boost binding failed");
    Require(fly->AddAction("Reset", InputActionType::Button), "fly Reset creation failed");
    Require(fly->AddBinding("Reset", InputBinding::KeyBinding(Key::R)),
        "fly Reset binding failed");

    FreeFlyCameraActions flyActions;
    flyActions.move = {"fly", "Move"};
    flyActions.elevate = {"fly", "Elevate"};
    flyActions.lookDelta = {};
    flyActions.lookRate = {"fly", "LookRate"};
    flyActions.boost = {"fly", "Boost"};
    flyActions.reset = {"fly", "Reset"};

    FreeFlyCameraSettings flySettings;
    flySettings.movementSpeed = 2.0f;
    flySettings.verticalSpeed = 2.0f;
    flySettings.boostMultiplier = 2.0f;
    flySettings.turnSpeed = 1.0f;

    Camera flyCamera;
    flyCamera.SetPosition(Math::Vec3::Zero);
    flyCamera.SetRotation(0.0f, 0.0f, 0.0f);
    FreeFlyCameraController flyController(flyActions, flySettings);
    CameraController* polymorphic = &flyController;
    Require(polymorphic->IsEnabled(), "controller interface should start enabled");
    flyController.CaptureHome(flyCamera);

    input.BeginFrame();
    input.ProcessKey(Key::W, true);
    input.ProcessKey(Key::LeftShift, true);
    actions.Update(input);
    flyController.Update(flyCamera, actions, 0.5f);
    Require(NearlyEqual(flyCamera.GetPosition(), Math::Vec3(0.0f, 0.0f, -2.0f)),
        "boosted free-fly movement mismatch");

    input.BeginFrame();
    ReleaseAll(input);
    input.ProcessKey(Key::E, true);
    actions.Update(input);
    flyController.Update(flyCamera, actions, 0.5f);
    Require(NearlyEqual(flyCamera.GetPosition().y, 1.0f),
        "free-fly world-up elevation mismatch");

    input.BeginFrame();
    ReleaseAll(input);
    input.ProcessKey(Key::Right, true);
    actions.Update(input);
    flyController.Update(flyCamera, actions, 0.5f);
    Require(flyCamera.GetForward().x < -0.4f,
        "free-fly rate look should rotate yaw");

    input.BeginFrame();
    ReleaseAll(input);
    input.ProcessKey(Key::R, true);
    input.ProcessKey(Key::R, false);
    actions.Update(input);
    flyController.Update(flyCamera, actions, 0.5f);
    Require(NearlyEqual(flyCamera.GetPosition(), Math::Vec3::Zero),
        "free-fly reset should restore home position");
    Require(NearlyEqual(flyCamera.GetForward(), Math::Vec3(0.0f, 0.0f, -1.0f)),
        "free-fly reset should restore home rotation");

    flyController.SetEnabled(false);
    input.BeginFrame();
    input.ProcessKey(Key::W, true);
    actions.Update(input);
    flyController.Update(flyCamera, actions, 1.0f);
    Require(NearlyEqual(flyCamera.GetPosition(), Math::Vec3::Zero),
        "disabled controller should not move the camera");
    flyController.SetEnabled(true);

    FreeFlyCameraSettings invalidFly = flySettings;
    invalidFly.maximumPitch = invalidFly.minimumPitch;
    Require(!flyController.SetSettings(invalidFly), "invalid free-fly settings should fail");
    Require(NearlyEqual(flyController.GetSettings().movementSpeed, 2.0f),
        "failed free-fly settings update should preserve the prior settings");

    InputContext* orbit = actions.CreateContext("orbit", 10, true);
    Require(orbit != nullptr, "orbit context creation failed");
    Require(orbit->AddAction("OrbitRate", InputActionType::Axis2D),
        "orbit rate creation failed");
    Require(orbit->AddBinding(
        "OrbitRate",
        InputBinding::KeyBinding(Key::Right, 1.0f, InputAxisComponent::X)),
        "orbit rate binding failed");
    Require(orbit->AddAction("PanRate", InputActionType::Axis2D),
        "pan rate creation failed");
    Require(orbit->AddBinding(
        "PanRate",
        InputBinding::KeyBinding(Key::D, 1.0f, InputAxisComponent::X)),
        "pan rate binding failed");
    Require(orbit->AddAction("ZoomDelta", InputActionType::Axis1D),
        "orbit zoom creation failed");
    Require(orbit->AddBinding("ZoomDelta", InputBinding::MouseWheelBinding()),
        "orbit zoom binding failed");
    Require(orbit->AddAction("Reset", InputActionType::Button),
        "orbit reset creation failed");
    Require(orbit->AddBinding("Reset", InputBinding::KeyBinding(Key::R)),
        "orbit reset binding failed");

    OrbitCameraActions orbitActions;
    orbitActions.panDelta = {};
    orbitActions.panRate = {"orbit", "PanRate"};
    orbitActions.orbitDelta = {};
    orbitActions.orbitRate = {"orbit", "OrbitRate"};
    orbitActions.zoomDelta = {"orbit", "ZoomDelta"};
    orbitActions.zoomRate = {};
    orbitActions.reset = {"orbit", "Reset"};

    OrbitCameraSettings orbitSettings;
    orbitSettings.panSpeed = 2.0f;
    orbitSettings.orbitSpeed = Math::HALF_PI;
    orbitSettings.zoomSensitivity = 1.0f;

    Camera orbitCamera;
    orbitCamera.SetPosition(Math::Vec3(0.0f, 0.0f, 5.0f));
    orbitCamera.LookAt(Math::Vec3::Zero);
    OrbitCameraController orbitController(Math::Vec3::Zero, orbitActions, orbitSettings);
    orbitController.CaptureHome(orbitCamera);

    input.BeginFrame();
    ReleaseAll(input);
    input.ProcessMouseWheel(-2.0f);
    actions.Update(input);
    orbitController.Update(orbitCamera, actions, 0.0f);
    Require(NearlyEqual(orbitController.GetDistance(), 3.0f),
        "orbit wheel zoom mismatch");
    Require(NearlyEqual(orbitCamera.GetPosition().Length(), 3.0f),
        "orbit camera distance mismatch");

    input.BeginFrame();
    input.ProcessKey(Key::Right, true);
    actions.Update(input);
    orbitController.Update(orbitCamera, actions, 1.0f);
    Require(orbitCamera.GetPosition().x > 2.9f,
        "orbit rate should rotate around the target");

    input.BeginFrame();
    input.ProcessKey(Key::Right, false);
    input.ProcessKey(Key::D, true);
    actions.Update(input);
    orbitController.Update(orbitCamera, actions, 1.0f);
    Require(orbitController.GetTarget().LengthSquared() > 1.0f,
        "orbit panning should move the target");

    input.BeginFrame();
    input.ProcessKey(Key::D, false);
    input.ProcessKey(Key::R, true);
    input.ProcessKey(Key::R, false);
    actions.Update(input);
    orbitController.Update(orbitCamera, actions, 0.0f);
    Require(NearlyEqual(orbitController.GetTarget(), Math::Vec3::Zero),
        "orbit reset should restore target");
    Require(NearlyEqual(orbitController.GetDistance(), 5.0f),
        "orbit reset should restore distance");

    InputContext* rts = actions.CreateContext("rts", 20, true);
    Require(rts != nullptr, "RTS context creation failed");
    Require(rts->AddAction("Move", InputActionType::Axis2D), "RTS Move creation failed");
    Require(rts->AddBinding(
        "Move",
        InputBinding::KeyBinding(Key::W, 1.0f, InputAxisComponent::Y)),
        "RTS Move binding failed");
    Require(rts->AddAction("OrbitRate", InputActionType::Axis2D),
        "RTS orbit rate creation failed");
    Require(rts->AddBinding(
        "OrbitRate",
        InputBinding::KeyBinding(Key::Right, 1.0f, InputAxisComponent::X)),
        "RTS orbit binding failed");
    Require(rts->AddAction("ZoomDelta", InputActionType::Axis1D),
        "RTS zoom creation failed");
    Require(rts->AddBinding("ZoomDelta", InputBinding::MouseWheelBinding()),
        "RTS zoom binding failed");
    Require(rts->AddAction("Boost", InputActionType::Button), "RTS boost creation failed");
    Require(rts->AddBinding("Boost", InputBinding::KeyBinding(Key::LeftShift)),
        "RTS boost binding failed");
    Require(rts->AddAction("Reset", InputActionType::Button), "RTS reset creation failed");
    Require(rts->AddBinding("Reset", InputBinding::KeyBinding(Key::R)),
        "RTS reset binding failed");

    RTSCameraActions rtsActions;
    rtsActions.move = {"rts", "Move"};
    rtsActions.orbitDelta = {};
    rtsActions.orbitRate = {"rts", "OrbitRate"};
    rtsActions.zoomDelta = {"rts", "ZoomDelta"};
    rtsActions.zoomRate = {};
    rtsActions.boost = {"rts", "Boost"};
    rtsActions.reset = {"rts", "Reset"};

    RTSCameraSettings rtsSettings;
    rtsSettings.movementSpeed = 2.0f;
    rtsSettings.boostMultiplier = 2.0f;
    rtsSettings.orbitSpeed = Math::HALF_PI;
    rtsSettings.zoomSensitivity = 1.0f;
    rtsSettings.distanceMovementScale = 0.0f;

    Camera rtsCamera;
    rtsCamera.SetPosition(Math::Vec3(0.0f, 7.0710678f, 7.0710678f));
    rtsCamera.LookAt(Math::Vec3::Zero);
    RTSCameraController rtsController(Math::Vec3::Zero, rtsActions, rtsSettings);
    rtsController.CaptureHome(rtsCamera);

    input.BeginFrame();
    ReleaseAll(input);
    input.ProcessKey(Key::W, true);
    input.ProcessKey(Key::LeftShift, true);
    actions.Update(input);
    rtsController.Update(rtsCamera, actions, 0.5f);
    Require(NearlyEqual(rtsController.GetFocusPoint().z, -2.0f),
        "RTS boosted ground movement mismatch");
    Require(NearlyEqual(rtsCamera.GetForward().Dot(
        (rtsController.GetFocusPoint() - rtsCamera.GetPosition()).Normalized()), 1.0f),
        "RTS camera should continue looking at the focus point");

    input.BeginFrame();
    ReleaseAll(input);
    input.ProcessMouseWheel(-2.0f);
    actions.Update(input);
    rtsController.Update(rtsCamera, actions, 0.0f);
    Require(NearlyEqual(rtsController.GetDistance(), 8.0f),
        "RTS wheel zoom mismatch");

    input.BeginFrame();
    input.ProcessKey(Key::Right, true);
    actions.Update(input);
    rtsController.Update(rtsCamera, actions, 1.0f);
    Require(rtsController.GetYaw() > 1.5f,
        "RTS rate orbit should change yaw");

    input.BeginFrame();
    input.ProcessKey(Key::Right, false);
    input.ProcessKey(Key::R, true);
    input.ProcessKey(Key::R, false);
    actions.Update(input);
    rtsController.Update(rtsCamera, actions, 0.0f);
    Require(NearlyEqual(rtsController.GetFocusPoint(), Math::Vec3::Zero),
        "RTS reset should restore focus point");
    Require(NearlyEqual(rtsController.GetDistance(), 10.0f),
        "RTS reset should restore distance");
    Require(NearlyEqual(rtsController.GetElevation(), Math::QUARTER_PI),
        "RTS reset should restore elevation");

    RTSCameraSettings invalidRts = rtsSettings;
    invalidRts.minimumDistance = 0.0f;
    Require(!rtsController.SetSettings(invalidRts), "invalid RTS settings should fail");

    std::cout << "Camera controller tests passed\n";
    return EXIT_SUCCESS;
}
