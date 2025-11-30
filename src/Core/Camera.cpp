#include "Camera.hpp"
#include <algorithm>
#include <cmath>

Camera::Camera(float width, float height)
    : mPosition(Vector2::Zero)
    , mWidth(width)
    , mHeight(height)
    , mTargetPosition(Vector2::Zero)
    , mTransitionSpeed(5.0f)
{
}

void Camera::Update(float deltaTime, const Vector2& playerPos, int mapWidthPixels, int mapHeightPixels)
{
    // Calculate which screen the player is in
    int screenX = static_cast<int>(playerPos.x / mWidth);
    int screenY = static_cast<int>(playerPos.y / mHeight);

    // Calculate target position (top-left of the screen)
    float targetX = static_cast<float>(screenX) * mWidth;
    float targetY = static_cast<float>(screenY) * mHeight;
    
    // Clamp to map bounds
    float maxCamX = std::max(0.0f, static_cast<float>(mapWidthPixels) - mWidth);
    float maxCamY = std::max(0.0f, static_cast<float>(mapHeightPixels) - mHeight);
    
    targetX = std::clamp(targetX, 0.0f, maxCamX);
    targetY = std::clamp(targetY, 0.0f, maxCamY);
    
    mTargetPosition = Vector2(targetX, targetY);
    
    // Smoothly interpolate to target
    Vector2 diff = mTargetPosition - mPosition;
    float distance = diff.Length();
    
    if (distance > 1.0f)
    {
        mPosition = mPosition + diff * mTransitionSpeed * deltaTime;
    }
    else
    {
        mPosition = mTargetPosition;
    }
}

void Camera::SnapToPlayer(const Vector2& playerPos, int mapWidthPixels, int mapHeightPixels)
{
    int screenX = static_cast<int>(playerPos.x / mWidth);
    int screenY = static_cast<int>(playerPos.y / mHeight);

    float targetX = static_cast<float>(screenX) * mWidth;
    float targetY = static_cast<float>(screenY) * mHeight;
    
    float maxCamX = std::max(0.0f, static_cast<float>(mapWidthPixels) - mWidth);
    float maxCamY = std::max(0.0f, static_cast<float>(mapHeightPixels) - mHeight);
    
    targetX = std::clamp(targetX, 0.0f, maxCamX);
    targetY = std::clamp(targetY, 0.0f, maxCamY);
    
    mPosition = Vector2(targetX, targetY);
    mTargetPosition = mPosition;
}
