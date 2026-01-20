#include "stdafx.h"
#include "Engine/Camera.h"

void Camera::Initialize(float fovDegrees, float aspectRatio, float nearPlane, float farPlane)
{
    m_fov = fovDegrees;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;

    m_viewDirty = true;
    m_projectionDirty = true;

    UpdateViewMatrix();
    UpdateProjectionMatrix();
}

void Camera::MoveForward(float distance)
{
    XMFLOAT3 direction = GetDirection();
    m_position.x += direction.x * distance;
    m_position.y += direction.y * distance;
    m_position.z += direction.z * distance;
    m_target.x += direction.x * distance;
    m_target.y += direction.y * distance;
    m_target.z += direction.z * distance;

    m_viewDirty = true;
}

void Camera::MoveBackward(float distance)
{
    MoveForward(-distance);
}

void Camera::MoveRight(float distance)
{
    XMFLOAT3 right = GetRight();
    m_position.x += right.x * distance;
    m_position.y += right.y * distance;
    m_position.z += right.z * distance;
    m_target.x += right.x * distance;
    m_target.y += right.y * distance;
    m_target.z += right.z * distance;

    m_viewDirty = true;
}

void Camera::MoveLeft(float distance)
{
    MoveRight(-distance);
}

void Camera::MoveUp(float distance)
{
    m_position.y += distance;
    m_target.y += distance;
    m_viewDirty = true;
}

void Camera::MoveDown(float distance)
{
    MoveUp(-distance);
}

void Camera::Rotate(float yawDegrees, float pitchDegrees)
{
    m_yaw += yawDegrees;
    m_pitch += pitchDegrees;

    // Clamp pitch to prevent camera flip
    const float maxPitch = 89.0f;
    if (m_pitch > maxPitch)
        m_pitch = maxPitch;
    if (m_pitch < -maxPitch)
        m_pitch = -maxPitch;

    m_viewDirty = true;
}

void Camera::SetRotation(float yawDegrees, float pitchDegrees)
{
    m_yaw = yawDegrees;
    m_pitch = pitchDegrees;

    // Clamp pitch to prevent camera flip
    const float maxPitch = 89.0f;
    if (m_pitch > maxPitch)
        m_pitch = maxPitch;
    if (m_pitch < -maxPitch)
        m_pitch = -maxPitch;

    m_viewDirty = true;
}

void Camera::SetPosition(float x, float y, float z)
{
    SetPosition(XMFLOAT3(x, y, z));
}

void Camera::SetPosition(const XMFLOAT3& position)
{
    XMFLOAT3 offset;
    offset.x = position.x - m_position.x;
    offset.y = position.y - m_position.y;
    offset.z = position.z - m_position.z;

    m_position = position;
    m_target.x += offset.x;
    m_target.y += offset.y;
    m_target.z += offset.z;

    m_viewDirty = true;
}

void Camera::SetTarget(float x, float y, float z)
{
    SetTarget(XMFLOAT3(x, y, z));
}

void Camera::SetTarget(const XMFLOAT3& target)
{
    m_target = target;
    m_viewDirty = true;
}

void Camera::SetUpVector(float x, float y, float z)
{
    SetUpVector(XMFLOAT3(x, y, z));
}

void Camera::SetUpVector(const XMFLOAT3& upVector)
{
    m_upVector = upVector;
    m_viewDirty = true;
}

void Camera::SetFOV(float fovDegrees)
{
    m_fov = fovDegrees;
    m_projectionDirty = true;
}

void Camera::SetAspectRatio(float aspectRatio)
{
    m_aspectRatio = aspectRatio;
    m_projectionDirty = true;
}

void Camera::SetClipPlanes(float nearPlane, float farPlane)
{
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionDirty = true;
}

XMMATRIX Camera::GetViewMatrix() const
{
    if (m_viewDirty)
    {
        const_cast<Camera*>(this)->UpdateViewMatrix();
    }
    return m_viewMatrix;
}

XMMATRIX Camera::GetProjectionMatrix() const
{
    if (m_projectionDirty)
    {
        const_cast<Camera*>(this)->UpdateProjectionMatrix();
    }
    return m_projectionMatrix;
}

XMMATRIX Camera::GetViewProjectionMatrix() const
{
    return XMMatrixMultiply(GetViewMatrix(), GetProjectionMatrix());
}

XMFLOAT3 Camera::GetForward() const
{
    XMFLOAT3 direction = GetDirection();
    return direction;
}

XMFLOAT3 Camera::GetRight() const
{
    XMFLOAT3 forward = GetDirection();
    XMVECTOR forwardVec = XMLoadFloat3(&forward);
    XMVECTOR upVec = XMLoadFloat3(&m_upVector);
    XMVECTOR rightVec = XMVector3Cross(upVec, forwardVec);
    rightVec = XMVector3Normalize(rightVec);

    XMFLOAT3 right;
    XMStoreFloat3(&right, rightVec);
    return right;
}

void Camera::UpdateViewMatrix()
{
    XMVECTOR posVec = XMLoadFloat3(&m_position);
    XMVECTOR targetVec = XMLoadFloat3(&m_target);
    XMVECTOR upVec = XMLoadFloat3(&m_upVector);

    m_viewMatrix = XMMatrixLookAtLH(posVec, targetVec, upVec);
    m_viewDirty = false;
}

void Camera::UpdateProjectionMatrix()
{
    m_projectionMatrix = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(m_fov),
        m_aspectRatio,
        m_nearPlane,
        m_farPlane
    );
    m_projectionDirty = false;
}

XMFLOAT3 Camera::GetDirection() const
{
    // Convert yaw and pitch to direction vector
    float yawRad = XMConvertToRadians(m_yaw);
    float pitchRad = XMConvertToRadians(m_pitch);

    float cosYaw = cosf(yawRad);
    float sinYaw = sinf(yawRad);
    float cosPitch = cosf(pitchRad);
    float sinPitch = sinf(pitchRad);

    XMFLOAT3 direction;
    direction.x = sinYaw * cosPitch;
    direction.y = sinPitch;
    direction.z = cosYaw * cosPitch;

    return direction;
}
