#pragma once

#include <Pyramid/Core/Prerequisites.hpp>
#include <Pyramid/Graphics/Camera.hpp>
#include <Pyramid/Input/InputActions.hpp>
#include <Pyramid/Math/Math.hpp>

#include <string>
#include <utility>

namespace Pyramid
{
    /**
     * @brief Named input action consumed by a camera controller.
     *
     * Empty references are treated as unbound. This keeps controllers independent
     * from any particular game's action names or physical bindings.
     */
    struct CameraActionReference
    {
        std::string context;
        std::string action;

        [[nodiscard]] bool IsBound() const
        {
            return !context.empty() && !action.empty();
        }
    };

    /**
     * @brief Common non-owning camera-controller interface.
     */
    class CameraController
    {
    public:
        virtual ~CameraController() = default;

        void SetEnabled(bool enabled) { m_enabled = enabled; }
        [[nodiscard]] bool IsEnabled() const { return m_enabled; }

        virtual void Update(Camera& camera, const InputActionSystem& actions, f32 deltaTime) = 0;
        virtual void Synchronize(const Camera& camera) = 0;
        virtual void CaptureHome(const Camera& camera) = 0;
        virtual void Reset(Camera& camera) = 0;

    protected:
        [[nodiscard]] bool CanUpdate(f32 deltaTime) const;

    private:
        bool m_enabled = true;
    };

    struct FreeFlyCameraActions
    {
        CameraActionReference move{"camera", "Move"};
        CameraActionReference elevate{"camera", "Elevate"};
        CameraActionReference lookDelta{"camera", "LookDelta"};
        CameraActionReference lookRate{"camera", "LookRate"};
        CameraActionReference boost{"camera", "Boost"};
        CameraActionReference reset{"camera", "Reset"};
    };

    struct FreeFlyCameraSettings
    {
        f32 movementSpeed = 5.0f;
        f32 verticalSpeed = 5.0f;
        f32 boostMultiplier = 3.0f;
        f32 lookSensitivity = 0.0025f;
        f32 turnSpeed = Math::Radians(90.0f);
        f32 minimumPitch = Math::Radians(-89.0f);
        f32 maximumPitch = Math::Radians(89.0f);
        bool useWorldUp = true;
    };

    /**
     * @brief Six-degree camera movement driven by generic named actions.
     */
    class FreeFlyCameraController final : public CameraController
    {
    public:
        explicit FreeFlyCameraController(
            FreeFlyCameraActions actions = {},
            FreeFlyCameraSettings settings = {});

        [[nodiscard]] bool SetSettings(const FreeFlyCameraSettings& settings);
        [[nodiscard]] const FreeFlyCameraSettings& GetSettings() const { return m_settings; }
        void SetActions(FreeFlyCameraActions actions) { m_actions = std::move(actions); }
        [[nodiscard]] const FreeFlyCameraActions& GetActions() const { return m_actions; }

        void Update(Camera& camera, const InputActionSystem& actions, f32 deltaTime) override;
        void Synchronize(const Camera& camera) override;
        void CaptureHome(const Camera& camera) override;
        void Reset(Camera& camera) override;

    private:
        void ApplyRotation(Camera& camera) const;

        FreeFlyCameraActions m_actions;
        FreeFlyCameraSettings m_settings;
        Math::Vec3 m_homePosition = Math::Vec3::Zero;
        f32 m_pitch = 0.0f;
        f32 m_yaw = 0.0f;
        f32 m_homePitch = 0.0f;
        f32 m_homeYaw = 0.0f;
        bool m_initialized = false;
        bool m_hasHome = false;
    };

    struct OrbitCameraActions
    {
        CameraActionReference panDelta{"camera", "PanDelta"};
        CameraActionReference panRate{"camera", "PanRate"};
        CameraActionReference orbitDelta{"camera", "OrbitDelta"};
        CameraActionReference orbitRate{"camera", "OrbitRate"};
        CameraActionReference zoomDelta{"camera", "ZoomDelta"};
        CameraActionReference zoomRate{"camera", "ZoomRate"};
        CameraActionReference reset{"camera", "Reset"};
    };

    struct OrbitCameraSettings
    {
        f32 panSensitivity = 0.01f;
        f32 panSpeed = 5.0f;
        f32 orbitSensitivity = 0.0025f;
        f32 orbitSpeed = Math::Radians(90.0f);
        f32 zoomSensitivity = 1.0f;
        f32 zoomSpeed = 5.0f;
        f32 minimumDistance = 0.1f;
        f32 maximumDistance = 1000.0f;
        f32 minimumPitch = Math::Radians(-89.0f);
        f32 maximumPitch = Math::Radians(89.0f);
    };

    /**
     * @brief Target-centered orbit camera with optional screen-plane panning.
     */
    class OrbitCameraController final : public CameraController
    {
    public:
        explicit OrbitCameraController(
            Math::Vec3 target = Math::Vec3::Zero,
            OrbitCameraActions actions = {},
            OrbitCameraSettings settings = {});

        [[nodiscard]] bool SetSettings(const OrbitCameraSettings& settings);
        [[nodiscard]] const OrbitCameraSettings& GetSettings() const { return m_settings; }
        void SetActions(OrbitCameraActions actions) { m_actions = std::move(actions); }
        [[nodiscard]] const OrbitCameraActions& GetActions() const { return m_actions; }

        void SetTarget(const Math::Vec3& target);
        [[nodiscard]] const Math::Vec3& GetTarget() const { return m_target; }
        void SetDistance(f32 distance);
        [[nodiscard]] f32 GetDistance() const { return m_distance; }

        void Update(Camera& camera, const InputActionSystem& actions, f32 deltaTime) override;
        void Synchronize(const Camera& camera) override;
        void CaptureHome(const Camera& camera) override;
        void Reset(Camera& camera) override;

    private:
        void Apply(Camera& camera) const;

        OrbitCameraActions m_actions;
        OrbitCameraSettings m_settings;
        Math::Vec3 m_target = Math::Vec3::Zero;
        Math::Vec3 m_homeTarget = Math::Vec3::Zero;
        f32 m_distance = 5.0f;
        f32 m_pitch = 0.0f;
        f32 m_yaw = 0.0f;
        f32 m_homeDistance = 5.0f;
        f32 m_homePitch = 0.0f;
        f32 m_homeYaw = 0.0f;
        bool m_initialized = false;
        bool m_hasHome = false;
    };

    struct RTSCameraActions
    {
        CameraActionReference move{"camera", "Move"};
        CameraActionReference orbitDelta{"camera", "OrbitDelta"};
        CameraActionReference orbitRate{"camera", "OrbitRate"};
        CameraActionReference zoomDelta{"camera", "ZoomDelta"};
        CameraActionReference zoomRate{"camera", "ZoomRate"};
        CameraActionReference boost{"camera", "Boost"};
        CameraActionReference reset{"camera", "Reset"};
    };

    struct RTSCameraSettings
    {
        f32 movementSpeed = 8.0f;
        f32 boostMultiplier = 2.5f;
        f32 orbitSensitivity = 0.0025f;
        f32 orbitSpeed = Math::Radians(90.0f);
        f32 zoomSensitivity = 1.0f;
        f32 zoomSpeed = 8.0f;
        f32 minimumDistance = 2.0f;
        f32 maximumDistance = 200.0f;
        f32 minimumElevation = Math::Radians(15.0f);
        f32 maximumElevation = Math::Radians(80.0f);
        f32 distanceMovementScale = 0.05f;
    };

    /**
     * @brief XZ-ground-plane strategy camera driven by generic named actions.
     *
     * The controller is an optional engine utility, not an engine-wide camera
     * assumption. Other games can use FreeFlyCameraController,
     * OrbitCameraController, or their own CameraController implementation.
     */
    class RTSCameraController final : public CameraController
    {
    public:
        explicit RTSCameraController(
            Math::Vec3 focusPoint = Math::Vec3::Zero,
            RTSCameraActions actions = {},
            RTSCameraSettings settings = {});

        [[nodiscard]] bool SetSettings(const RTSCameraSettings& settings);
        [[nodiscard]] const RTSCameraSettings& GetSettings() const { return m_settings; }
        void SetActions(RTSCameraActions actions) { m_actions = std::move(actions); }
        [[nodiscard]] const RTSCameraActions& GetActions() const { return m_actions; }

        void SetFocusPoint(const Math::Vec3& focusPoint);
        [[nodiscard]] const Math::Vec3& GetFocusPoint() const { return m_focusPoint; }
        void SetDistance(f32 distance);
        [[nodiscard]] f32 GetDistance() const { return m_distance; }
        void SetYaw(f32 yaw) { m_yaw = yaw; }
        [[nodiscard]] f32 GetYaw() const { return m_yaw; }
        void SetElevation(f32 elevation);
        [[nodiscard]] f32 GetElevation() const { return m_elevation; }

        void Update(Camera& camera, const InputActionSystem& actions, f32 deltaTime) override;
        void Synchronize(const Camera& camera) override;
        void CaptureHome(const Camera& camera) override;
        void Reset(Camera& camera) override;

    private:
        void Apply(Camera& camera) const;

        RTSCameraActions m_actions;
        RTSCameraSettings m_settings;
        Math::Vec3 m_focusPoint = Math::Vec3::Zero;
        Math::Vec3 m_homeFocusPoint = Math::Vec3::Zero;
        f32 m_distance = 10.0f;
        f32 m_yaw = 0.0f;
        f32 m_elevation = Math::Radians(45.0f);
        f32 m_homeDistance = 10.0f;
        f32 m_homeYaw = 0.0f;
        f32 m_homeElevation = Math::Radians(45.0f);
        bool m_initialized = false;
        bool m_hasHome = false;
    };
} // namespace Pyramid
