#include <Pyramid/Examples/RTSReference/RTSInteractionController.hpp>
#include <Pyramid/Graphics/Scene.hpp>
#include <Pyramid/Graphics/Scene/SceneManager.hpp>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>

namespace
{
    void Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "RTS interaction test failed: " << message << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    bool NearlyEqual(Pyramid::f32 left, Pyramid::f32 right, Pyramid::f32 epsilon = 0.001f)
    {
        return std::fabs(left - right) <= epsilon;
    }

    bool NearlyEqual(
        const Pyramid::Math::Vec3& left,
        const Pyramid::Math::Vec3& right,
        Pyramid::f32 epsilon = 0.001f)
    {
        return NearlyEqual(left.x, right.x, epsilon) &&
            NearlyEqual(left.y, right.y, epsilon) &&
            NearlyEqual(left.z, right.z, epsilon);
    }

    void ReleaseMouse(Pyramid::InputState& input)
    {
        input.ProcessMouseButton(Pyramid::MouseButton::Left, false);
        input.ProcessMouseButton(Pyramid::MouseButton::Right, false);
        input.ProcessMouseButton(Pyramid::MouseButton::Middle, false);
    }
}

int main()
{
    using namespace Pyramid;
    using namespace Pyramid::Examples::RTSReference;

    InputState input;
    InputActionSystem actions;
    input.SetFocused(true);

    InputContext* context = actions.CreateContext("reference-rts", 0, true);
    Require(context != nullptr, "input context creation failed");
    Require(context->AddAction("Select", InputActionType::Button),
        "select action creation failed");
    Require(context->AddBinding(
        "Select",
        InputBinding::MouseButtonBinding(MouseButton::Left)),
        "select binding creation failed");
    Require(context->AddAction("Command", InputActionType::Button),
        "command action creation failed");
    Require(context->AddBinding(
        "Command",
        InputBinding::MouseButtonBinding(MouseButton::Right)),
        "command binding creation failed");

    RTSInteractionActions interactionActions;
    interactionActions.select = {"reference-rts", "Select"};
    interactionActions.command = {"reference-rts", "Command"};

    RTSInteractionSettings settings;
    settings.edgeMarginPixels = 20.0f;
    settings.edgeScrollSpeed = 4.0f;
    settings.edgeScrollDistanceScale = 0.0f;
    settings.maximumRayDistance = 100.0f;

    RTSInteractionController interaction(interactionActions, settings);
    interaction.SetViewportSize(1000, 800);

    InputState noPointer;
    noPointer.SetFocused(true);
    const InputActionVector2 noPointerAxis = interaction.GetEdgeScrollAxis(noPointer);
    Require(NearlyEqual(noPointerAxis.x, 0.0f) && NearlyEqual(noPointerAxis.y, 0.0f),
        "edge scrolling should wait for the first pointer sample");

    input.ProcessMouseMove(0.0f, 0.0f);
    const InputActionVector2 cornerAxis = interaction.GetEdgeScrollAxis(input);
    Require(NearlyEqual(cornerAxis.x, -1.0f) && NearlyEqual(cornerAxis.y, 1.0f),
        "top-left corner should produce a diagonal edge axis");

    input.ProcessMouseMove(500.0f, 400.0f);
    const InputActionVector2 centerAxis = interaction.GetEdgeScrollAxis(input);
    Require(NearlyEqual(centerAxis.x, 0.0f) && NearlyEqual(centerAxis.y, 0.0f),
        "center pointer should not edge scroll");

    auto scene = std::make_shared<Scene>("RTS Interaction Test");

    auto blockerObject = std::make_shared<RenderObject>();
    blockerObject->name = "DecorativeBlocker";
    blockerObject->position = Math::Vec3(0.0f, 2.5f, 5.0f);
    blockerObject->SetLocalBounds(Math::Vec3(-0.75f), Math::Vec3(0.75f));
    const Entity blocker = scene->AddRenderObject(blockerObject);
    Require(static_cast<bool>(blocker), "blocker entity creation failed");

    auto unitObject = std::make_shared<RenderObject>();
    unitObject->name = "SelectableUnit";
    unitObject->position = Math::Vec3::Zero;
    unitObject->SetLocalBounds(Math::Vec3(-1.0f), Math::Vec3(1.0f));
    const Entity unit = scene->AddRenderObject(unitObject);
    Require(static_cast<bool>(unit), "unit entity creation failed");

    SceneManagement::SceneManager sceneManager;
    sceneManager.SetActiveScene(scene);

    Camera camera(
        Math::Radians(60.0f),
        1000.0f / 800.0f,
        0.1f,
        100.0f);
    camera.SetPosition(Math::Vec3(0.0f, 5.0f, 10.0f));
    camera.LookAt(Math::Vec3::Zero);

    RTSCameraActions cameraActions;
    cameraActions.move = {};
    cameraActions.orbitDelta = {};
    cameraActions.orbitRate = {};
    cameraActions.zoomDelta = {};
    cameraActions.zoomRate = {};
    cameraActions.boost = {};
    cameraActions.reset = {};

    RTSCameraSettings cameraSettings;
    cameraSettings.distanceMovementScale = 0.0f;
    RTSCameraController cameraController(Math::Vec3::Zero, cameraActions, cameraSettings);
    cameraController.CaptureHome(camera);

    input.BeginFrame();
    ReleaseMouse(input);
    input.ProcessMouseMove(0.0f, 400.0f);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 1.0f);
    Require(NearlyEqual(cameraController.GetFocusPoint(), Math::Vec3(-4.0f, 0.0f, 0.0f)),
        "left-edge scrolling should move the strategy focus left at a time-scaled speed");
    Require(NearlyEqual(
        camera.GetForward().Dot(
            (cameraController.GetFocusPoint() - camera.GetPosition()).Normalized()),
        1.0f),
        "edge scrolling should preserve the camera focus relationship");

    cameraController.Reset(camera);
    Require(NearlyEqual(cameraController.GetFocusPoint(), Math::Vec3::Zero),
        "camera reset should restore the interaction test focus");

    interaction.SetSelectablePredicate(
        [unitId = unit.GetId()](const Entity& entity)
        {
            return entity.GetId() == unitId;
        });

    input.BeginFrame();
    ReleaseMouse(input);
    input.ProcessMouseMove(500.0f, 400.0f);
    input.ProcessMouseButton(MouseButton::Left, true);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 0.0f);
    Require(interaction.GetSelectedEntityId() == unit.GetId(),
        "selection should skip a nearer non-selectable renderable and pick the unit behind it");
    Require(interaction.GetSelectedEntity(sceneManager) == unit,
        "selected entity lookup should resolve through the active scene");

    input.BeginFrame();
    ReleaseMouse(input);
    input.ProcessMouseMove(500.0f, 400.0f);
    input.ProcessMouseButton(MouseButton::Right, true);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 0.0f);
    Require(interaction.HasPendingCommand(),
        "a command press with a selection should emit a command request");
    const auto command = interaction.ConsumeCommand();
    Require(command.has_value(), "command consumption should return the pending request");
    Require(command->entity == unit.GetId(), "command should target the selected entity");
    Require(NearlyEqual(command->target, Math::Vec3::Zero, 0.01f),
        "center-screen command should project to the ground-plane origin");
    Require(!interaction.ConsumeCommand().has_value(),
        "command requests should be one-shot after consumption");

    input.BeginFrame();
    ReleaseMouse(input);
    input.ProcessMouseMove(850.0f, 100.0f);
    input.ProcessMouseButton(MouseButton::Left, true);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 0.0f);
    Require(!interaction.GetSelectedEntityId(),
        "an empty selection click should clear the previous selection");

    input.BeginFrame();
    ReleaseMouse(input);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 0.0f);

    input.BeginFrame();
    input.ProcessMouseMove(500.0f, 400.0f);
    input.ProcessMouseButton(MouseButton::Left, true);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 0.0f);
    Require(interaction.GetSelectedEntityId() == unit.GetId(),
        "unit should be selectable again before stale-entity validation");

    Require(scene->DestroyEntity(unit), "selected unit destruction failed");
    input.BeginFrame();
    ReleaseMouse(input);
    input.ProcessMouseButton(MouseButton::Right, true);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 0.0f);
    Require(!interaction.GetSelectedEntityId(),
        "destroyed selections should be cleared before command processing");
    Require(!interaction.HasPendingCommand(),
        "destroyed selections must not emit commands");

    RTSInteractionSettings invalid = interaction.GetSettings();
    invalid.commandPlaneNormal = Math::Vec3::Zero;
    Require(!interaction.SetSettings(invalid),
        "zero command-plane normals should be rejected");
    Require(NearlyEqual(interaction.GetSettings().edgeScrollSpeed, 4.0f),
        "failed settings updates should preserve the previous settings");

    interaction.SetEnabled(false);
    const Math::Vec3 disabledFocus = cameraController.GetFocusPoint();
    input.BeginFrame();
    ReleaseMouse(input);
    input.ProcessMouseMove(0.0f, 400.0f);
    actions.Update(input);
    interaction.Update(input, actions, camera, sceneManager, cameraController, 1.0f);
    Require(NearlyEqual(cameraController.GetFocusPoint(), disabledFocus),
        "disabled RTS interaction should not move the camera focus");

    std::cout << "RTS interaction tests passed\n";
    return EXIT_SUCCESS;
}
