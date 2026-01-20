#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
    Camera() = default;
    ~Camera() = default;

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    // Initialization
    void Initialize(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
    
    // Camera controls
    void MoveForward(float distance);
    void MoveBackward(float distance);
    void MoveRight(float distance);
    void MoveLeft(float distance);
    void MoveUp(float distance);
    void MoveDown(float distance);

    // Camera rotation
    void Rotate(float yawDegrees, float pitchDegrees);
    void SetRotation(float yawDegrees, float pitchDegrees);

    // Camera position and orientation
    void SetPosition(float x, float y, float z);
    void SetPosition(const XMFLOAT3& position);
    void SetTarget(float x, float y, float z);
    void SetTarget(const XMFLOAT3& target);
    void SetUpVector(float x, float y, float z);
    void SetUpVector(const XMFLOAT3& upVector);

    // Projection settings
    void SetFOV(float fovDegrees);
    void SetAspectRatio(float aspectRatio);
    void SetClipPlanes(float nearPlane, float farPlane);

    // Matrix accessors
    XMMATRIX GetViewMatrix() const;
    XMMATRIX GetProjectionMatrix() const;
    XMMATRIX GetViewProjectionMatrix() const;

    // Position and orientation accessors
    XMFLOAT3 GetPosition() const { return m_position; }
    XMFLOAT3 GetTarget() const { return m_target; }
    XMFLOAT3 GetForward() const;
    XMFLOAT3 GetRight() const;
    XMFLOAT3 GetUpVector() const { return m_upVector; }

    // Camera properties
    float GetFOV() const { return m_fov; }
    float GetAspectRatio() const { return m_aspectRatio; }
    float GetNearPlane() const { return m_nearPlane; }
    float GetFarPlane() const { return m_farPlane; }

private:
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
    XMFLOAT3 GetDirection() const;

    // Camera properties
    XMFLOAT3 m_position = { 0.0f, 1.0f, -5.0f };
    XMFLOAT3 m_target = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_upVector = { 0.0f, 1.0f, 0.0f };

    // Rotation in degrees
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    // Projection parameters
    float m_fov = 70.0f;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;

    // Cached matrices
    mutable XMMATRIX m_viewMatrix = XMMatrixIdentity();
    mutable XMMATRIX m_projectionMatrix = XMMatrixIdentity();
    mutable bool m_viewDirty = true;
    mutable bool m_projectionDirty = true;
};
