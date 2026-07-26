#include <Pyramid/Graphics/CameraController.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace Pyramid
{
    namespace
    {
        constexpr f32 ControllerEpsilon = 0.000001f;

        [[nodiscard]] bool IsFinite(f32 value)
        {
            return std::isfinite(value);
        }

        [[nodiscard]] bool IsFinite(const Math::Vec3& value)
        {
            return IsFinite(value.x) && IsFinite(value.y) && IsFinite(value.z);
        }

        [[nodiscard]] f32 Clamp(f32 value, f32 minimum, f32 maximum)
        {
            return std::clamp(value, minimum, maximum);
        }

        [[nodiscard]] f32 GetValue(
            const InputActionSystem& actions,
            const CameraActionReference& reference)
        {
            return reference.IsBound()
                ? actions.GetActionValue(reference.context, reference.action)
                : 0.0f;
        }

        [[nodiscard]] InputActionVector2 GetValue2D(
            const InputActionSystem& actions,
            const CameraActionReference& reference)
        {
            return reference.IsBound()
                ? actions.GetActionValue2D(reference.context, reference.action)
                : InputActionVector2{};
        }

        [[nodiscard]] bool IsDown(
            const InputActionSystem& actions,
            const CameraActionReference& reference)
        {
            return reference.IsBound() &&
                actions.IsActionDown(reference.context, reference.action);
        }

        [[nodiscard]] bool WasPressed(
            const InputActionSystem& actions,
            const CameraActionReference& reference)
        {
            return reference.IsBound() &&
                actions.WasActionPressed(reference.context, reference.action);
        }

        void ExtractPitchYaw(const Camera& camera, f32& pitch, f32& yaw)
        {
            const Math::Vec3 forward = camera.GetForward();
            pitch = std::asin(Clamp(forward.y, -1.0f, 1.0f));
            yaw = std::atan2(forward.x, -forward.z);
        }

        [[nodiscard]] Math::Vec3 ForwardFromPitchYaw(f32 pitch, f32 yaw)
        {
            return Math::Quat::FromEuler(pitch, yaw, 0.0f)
                .RotateVector(-Math::Vec3::Forward)
                .Normalized();
        }

        [[nodiscard]] bool ValidPitchRange(f32 minimum, f32 maximum)
        {
            return IsFinite(minimum) && IsFinite(maximum) && minimum < maximum &&
                minimum > -Math::PI && maximum < Math::PI;
        }
    }

    bool CameraController::CanUpdate(f32 deltaTime) const
    {
        return IsEnabled() && IsFinite(deltaTime) && deltaTime >= 0.0f;
    }

    FreeFlyCameraController::FreeFlyCameraController(
        FreeFlyCameraActions actions,
        FreeFlyCameraSettings settings)
        : m_actions(std::move(actions))
    {
        if (!SetSettings(settings))
        {
            m_settings = {};
        }
    }

    bool FreeFlyCameraController::SetSettings(const FreeFlyCameraSettings& settings)
    {
        if (!IsFinite(settings.movementSpeed) || settings.movementSpeed < 0.0f ||
            !IsFinite(settings.verticalSpeed) || settings.verticalSpeed < 0.0f ||
            !IsFinite(settings.boostMultiplier) || settings.boostMultiplier < 1.0f ||
            !IsFinite(settings.lookSensitivity) || settings.lookSensitivity < 0.0f ||
            !IsFinite(settings.turnSpeed) || settings.turnSpeed < 0.0f ||
            !ValidPitchRange(settings.minimumPitch, settings.maximumPitch))
        {
            return false;
        }

        m_settings = settings;
        m_pitch = Clamp(m_pitch, m_settings.minimumPitch, m_settings.maximumPitch);
        return true;
    }

    void FreeFlyCameraController::Update(
        Camera& camera,
        const InputActionSystem& actions,
        f32 deltaTime)
    {
        if (!CanUpdate(deltaTime))
        {
            return;
        }
        if (!m_initialized)
        {
            Synchronize(camera);
        }
        if (!m_hasHome)
        {
            CaptureHome(camera);
        }
        if (WasPressed(actions, m_actions.reset))
        {
            Reset(camera);
            return;
        }

        const InputActionVector2 lookDelta = GetValue2D(actions, m_actions.lookDelta);
        const InputActionVector2 lookRate = GetValue2D(actions, m_actions.lookRate);

        m_yaw += lookDelta.x * m_settings.lookSensitivity +
            lookRate.x * m_settings.turnSpeed * deltaTime;
        m_pitch = Clamp(
            m_pitch + lookDelta.y * m_settings.lookSensitivity +
                lookRate.y * m_settings.turnSpeed * deltaTime,
            m_settings.minimumPitch,
            m_settings.maximumPitch);
        ApplyRotation(camera);

        const InputActionVector2 move = GetValue2D(actions, m_actions.move);
        const f32 elevate = GetValue(actions, m_actions.elevate);
        const f32 boost = IsDown(actions, m_actions.boost) ? m_settings.boostMultiplier : 1.0f;

        Math::Vec3 direction = camera.GetRight() * move.x + camera.GetForward() * move.y;
        if (direction.LengthSquared() > 1.0f)
        {
            direction.Normalize();
        }
        if (direction.LengthSquared() > ControllerEpsilon)
        {
            camera.SetPosition(
                camera.GetPosition() + direction * (m_settings.movementSpeed * boost * deltaTime));
        }

        const Math::Vec3 vertical = m_settings.useWorldUp ? Math::Vec3::Up : camera.GetUp();
        if (std::fabs(elevate) > ControllerEpsilon)
        {
            camera.SetPosition(
                camera.GetPosition() + vertical *
                    (elevate * m_settings.verticalSpeed * boost * deltaTime));
        }
    }

    void FreeFlyCameraController::Synchronize(const Camera& camera)
    {
        ExtractPitchYaw(camera, m_pitch, m_yaw);
        m_pitch = Clamp(m_pitch, m_settings.minimumPitch, m_settings.maximumPitch);
        m_initialized = true;
    }

    void FreeFlyCameraController::CaptureHome(const Camera& camera)
    {
        Synchronize(camera);
        m_homePosition = camera.GetPosition();
        m_homePitch = m_pitch;
        m_homeYaw = m_yaw;
        m_hasHome = true;
    }

    void FreeFlyCameraController::Reset(Camera& camera)
    {
        if (!m_hasHome)
        {
            CaptureHome(camera);
        }
        m_pitch = m_homePitch;
        m_yaw = m_homeYaw;
        camera.SetPosition(m_homePosition);
        ApplyRotation(camera);
    }

    void FreeFlyCameraController::ApplyRotation(Camera& camera) const
    {
        camera.SetRotation(m_pitch, m_yaw, 0.0f);
    }

    OrbitCameraController::OrbitCameraController(
        Math::Vec3 target,
        OrbitCameraActions actions,
        OrbitCameraSettings settings)
        : m_actions(std::move(actions)), m_target(target)
    {
        if (!SetSettings(settings))
        {
            m_settings = {};
        }
        if (!IsFinite(m_target))
        {
            m_target = Math::Vec3::Zero;
        }
    }

    bool OrbitCameraController::SetSettings(const OrbitCameraSettings& settings)
    {
        if (!IsFinite(settings.panSensitivity) || settings.panSensitivity < 0.0f ||
            !IsFinite(settings.panSpeed) || settings.panSpeed < 0.0f ||
            !IsFinite(settings.orbitSensitivity) || settings.orbitSensitivity < 0.0f ||
            !IsFinite(settings.orbitSpeed) || settings.orbitSpeed < 0.0f ||
            !IsFinite(settings.zoomSensitivity) || settings.zoomSensitivity < 0.0f ||
            !IsFinite(settings.zoomSpeed) || settings.zoomSpeed < 0.0f ||
            !IsFinite(settings.minimumDistance) || settings.minimumDistance <= 0.0f ||
            !IsFinite(settings.maximumDistance) ||
            settings.maximumDistance < settings.minimumDistance ||
            !ValidPitchRange(settings.minimumPitch, settings.maximumPitch))
        {
            return false;
        }

        m_settings = settings;
        m_distance = Clamp(m_distance, m_settings.minimumDistance, m_settings.maximumDistance);
        m_pitch = Clamp(m_pitch, m_settings.minimumPitch, m_settings.maximumPitch);
        return true;
    }

    void OrbitCameraController::SetTarget(const Math::Vec3& target)
    {
        if (IsFinite(target))
        {
            m_target = target;
        }
    }

    void OrbitCameraController::SetDistance(f32 distance)
    {
        if (IsFinite(distance))
        {
            m_distance = Clamp(distance, m_settings.minimumDistance, m_settings.maximumDistance);
        }
    }

    void OrbitCameraController::Update(
        Camera& camera,
        const InputActionSystem& actions,
        f32 deltaTime)
    {
        if (!CanUpdate(deltaTime))
        {
            return;
        }
        if (!m_initialized)
        {
            Synchronize(camera);
        }
        if (!m_hasHome)
        {
            CaptureHome(camera);
        }
        if (WasPressed(actions, m_actions.reset))
        {
            Reset(camera);
            return;
        }

        const InputActionVector2 orbitDelta = GetValue2D(actions, m_actions.orbitDelta);
        const InputActionVector2 orbitRate = GetValue2D(actions, m_actions.orbitRate);
        m_yaw += orbitDelta.x * m_settings.orbitSensitivity +
            orbitRate.x * m_settings.orbitSpeed * deltaTime;
        m_pitch = Clamp(
            m_pitch + orbitDelta.y * m_settings.orbitSensitivity +
                orbitRate.y * m_settings.orbitSpeed * deltaTime,
            m_settings.minimumPitch,
            m_settings.maximumPitch);

        m_distance = Clamp(
            m_distance + GetValue(actions, m_actions.zoomDelta) * m_settings.zoomSensitivity +
                GetValue(actions, m_actions.zoomRate) * m_settings.zoomSpeed * deltaTime,
            m_settings.minimumDistance,
            m_settings.maximumDistance);

        const Math::Quat rotation = Math::Quat::FromEuler(m_pitch, m_yaw, 0.0f);
        const InputActionVector2 panDelta = GetValue2D(actions, m_actions.panDelta);
        const InputActionVector2 panRate = GetValue2D(actions, m_actions.panRate);
        const f32 panX = panDelta.x * m_settings.panSensitivity +
            panRate.x * m_settings.panSpeed * deltaTime;
        const f32 panY = panDelta.y * m_settings.panSensitivity +
            panRate.y * m_settings.panSpeed * deltaTime;
        m_target += rotation.RotateVector(Math::Vec3::Right) * panX +
            rotation.RotateVector(Math::Vec3::Up) * panY;

        Apply(camera);
    }

    void OrbitCameraController::Synchronize(const Camera& camera)
    {
        const Math::Vec3 offset = camera.GetPosition() - m_target;
        const f32 distance = offset.Length();
        if (distance > ControllerEpsilon)
        {
            m_distance = Clamp(distance, m_settings.minimumDistance, m_settings.maximumDistance);
            const Math::Vec3 forward = (-offset).Normalized();
            m_pitch = std::asin(Clamp(forward.y, -1.0f, 1.0f));
            m_yaw = std::atan2(forward.x, -forward.z);
        }
        else
        {
            ExtractPitchYaw(camera, m_pitch, m_yaw);
        }
        m_pitch = Clamp(m_pitch, m_settings.minimumPitch, m_settings.maximumPitch);
        m_initialized = true;
    }

    void OrbitCameraController::CaptureHome(const Camera& camera)
    {
        Synchronize(camera);
        m_homeTarget = m_target;
        m_homeDistance = m_distance;
        m_homePitch = m_pitch;
        m_homeYaw = m_yaw;
        m_hasHome = true;
    }

    void OrbitCameraController::Reset(Camera& camera)
    {
        if (!m_hasHome)
        {
            CaptureHome(camera);
        }
        m_target = m_homeTarget;
        m_distance = m_homeDistance;
        m_pitch = m_homePitch;
        m_yaw = m_homeYaw;
        Apply(camera);
    }

    void OrbitCameraController::Apply(Camera& camera) const
    {
        const Math::Vec3 forward = ForwardFromPitchYaw(m_pitch, m_yaw);
        camera.SetPosition(m_target - forward * m_distance);
        camera.SetRotation(m_pitch, m_yaw, 0.0f);
    }

    RTSCameraController::RTSCameraController(
        Math::Vec3 focusPoint,
        RTSCameraActions actions,
        RTSCameraSettings settings)
        : m_actions(std::move(actions)), m_focusPoint(focusPoint)
    {
        if (!SetSettings(settings))
        {
            m_settings = {};
        }
        if (!IsFinite(m_focusPoint))
        {
            m_focusPoint = Math::Vec3::Zero;
        }
    }

    bool RTSCameraController::SetSettings(const RTSCameraSettings& settings)
    {
        if (!IsFinite(settings.movementSpeed) || settings.movementSpeed < 0.0f ||
            !IsFinite(settings.boostMultiplier) || settings.boostMultiplier < 1.0f ||
            !IsFinite(settings.orbitSensitivity) || settings.orbitSensitivity < 0.0f ||
            !IsFinite(settings.orbitSpeed) || settings.orbitSpeed < 0.0f ||
            !IsFinite(settings.zoomSensitivity) || settings.zoomSensitivity < 0.0f ||
            !IsFinite(settings.zoomSpeed) || settings.zoomSpeed < 0.0f ||
            !IsFinite(settings.minimumDistance) || settings.minimumDistance <= 0.0f ||
            !IsFinite(settings.maximumDistance) ||
            settings.maximumDistance < settings.minimumDistance ||
            !IsFinite(settings.minimumElevation) || !IsFinite(settings.maximumElevation) ||
            settings.minimumElevation < 0.0f ||
            settings.maximumElevation <= settings.minimumElevation ||
            settings.maximumElevation >= Math::PI * 0.5f ||
            !IsFinite(settings.distanceMovementScale) || settings.distanceMovementScale < 0.0f)
        {
            return false;
        }

        m_settings = settings;
        m_distance = Clamp(m_distance, m_settings.minimumDistance, m_settings.maximumDistance);
        m_elevation = Clamp(
            m_elevation,
            m_settings.minimumElevation,
            m_settings.maximumElevation);
        return true;
    }

    void RTSCameraController::SetFocusPoint(const Math::Vec3& focusPoint)
    {
        if (IsFinite(focusPoint))
        {
            m_focusPoint = focusPoint;
        }
    }

    void RTSCameraController::SetDistance(f32 distance)
    {
        if (IsFinite(distance))
        {
            m_distance = Clamp(distance, m_settings.minimumDistance, m_settings.maximumDistance);
        }
    }

    void RTSCameraController::SetElevation(f32 elevation)
    {
        if (IsFinite(elevation))
        {
            m_elevation = Clamp(
                elevation,
                m_settings.minimumElevation,
                m_settings.maximumElevation);
        }
    }

    void RTSCameraController::Update(
        Camera& camera,
        const InputActionSystem& actions,
        f32 deltaTime)
    {
        if (!CanUpdate(deltaTime))
        {
            return;
        }
        if (!m_initialized)
        {
            Synchronize(camera);
        }
        if (!m_hasHome)
        {
            CaptureHome(camera);
        }
        if (WasPressed(actions, m_actions.reset))
        {
            Reset(camera);
            return;
        }

        const InputActionVector2 orbitDelta = GetValue2D(actions, m_actions.orbitDelta);
        const InputActionVector2 orbitRate = GetValue2D(actions, m_actions.orbitRate);
        m_yaw += orbitDelta.x * m_settings.orbitSensitivity +
            orbitRate.x * m_settings.orbitSpeed * deltaTime;
        m_elevation = Clamp(
            m_elevation + orbitDelta.y * m_settings.orbitSensitivity +
                orbitRate.y * m_settings.orbitSpeed * deltaTime,
            m_settings.minimumElevation,
            m_settings.maximumElevation);

        m_distance = Clamp(
            m_distance + GetValue(actions, m_actions.zoomDelta) * m_settings.zoomSensitivity +
                GetValue(actions, m_actions.zoomRate) * m_settings.zoomSpeed * deltaTime,
            m_settings.minimumDistance,
            m_settings.maximumDistance);

        const InputActionVector2 move = GetValue2D(actions, m_actions.move);
        const Math::Vec3 groundForward(
            std::sin(m_yaw),
            0.0f,
            -std::cos(m_yaw));
        const Math::Vec3 groundRight(
            std::cos(m_yaw),
            0.0f,
            std::sin(m_yaw));
        Math::Vec3 movement = groundRight * move.x + groundForward * move.y;
        if (movement.LengthSquared() > 1.0f)
        {
            movement.Normalize();
        }
        const f32 boost = IsDown(actions, m_actions.boost) ? m_settings.boostMultiplier : 1.0f;
        const f32 distanceScale = 1.0f + m_distance * m_settings.distanceMovementScale;
        m_focusPoint += movement *
            (m_settings.movementSpeed * boost * distanceScale * deltaTime);

        Apply(camera);
    }

    void RTSCameraController::Synchronize(const Camera& camera)
    {
        const Math::Vec3 offset = camera.GetPosition() - m_focusPoint;
        const f32 distance = offset.Length();
        if (distance > ControllerEpsilon)
        {
            m_distance = Clamp(distance, m_settings.minimumDistance, m_settings.maximumDistance);
            const Math::Vec3 direction = (m_focusPoint - camera.GetPosition()).Normalized();
            m_yaw = std::atan2(direction.x, -direction.z);
            m_elevation = Clamp(
                std::asin(Clamp(-direction.y, -1.0f, 1.0f)),
                m_settings.minimumElevation,
                m_settings.maximumElevation);
        }
        m_initialized = true;
    }

    void RTSCameraController::CaptureHome(const Camera& camera)
    {
        Synchronize(camera);
        m_homeFocusPoint = m_focusPoint;
        m_homeDistance = m_distance;
        m_homeYaw = m_yaw;
        m_homeElevation = m_elevation;
        m_hasHome = true;
    }

    void RTSCameraController::Reset(Camera& camera)
    {
        if (!m_hasHome)
        {
            CaptureHome(camera);
        }
        m_focusPoint = m_homeFocusPoint;
        m_distance = m_homeDistance;
        m_yaw = m_homeYaw;
        m_elevation = m_homeElevation;
        Apply(camera);
    }

    void RTSCameraController::Apply(Camera& camera) const
    {
        const f32 horizontal = std::cos(m_elevation);
        const Math::Vec3 direction(
            std::sin(m_yaw) * horizontal,
            -std::sin(m_elevation),
            -std::cos(m_yaw) * horizontal);
        camera.SetPosition(m_focusPoint - direction * m_distance);
        camera.LookAt(m_focusPoint, Math::Vec3::Up);
    }
} // namespace Pyramid
