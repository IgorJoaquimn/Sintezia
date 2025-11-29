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
    // Calculate which "screen" the player is in
    // We want the camera to be at (col * width, row * height)
    // We use the center of the player to determine the screen
    
    int col = static_cast<int>(playerPos.x / mWidth);
    int row = static_cast<int>(playerPos.y / mHeight);
    
    float targetX = col * mWidth;
    float targetY = row * mHeight;
    
    // Clamp to map bounds
    // The max camera position is mapSize - screenSize
    // We use max(0.0f, ...) to handle cases where map is smaller than screen
    float maxCamX = std::max(0.0f, static_cast<float>(mapWidthPixels) - mWidth);
    float maxCamY = std::max(0.0f, static_cast<float>(mapHeightPixels) - mHeight);
    
    // Ensure we don't show area outside the map
    targetX = std::clamp(targetX, 0.0f, maxCamX);
    targetY = std::clamp(targetY, 0.0f, maxCamY);
    
    mTargetPosition = Vector2(targetX, targetY);
    
    // Smoothly interpolate to target
    // Simple lerp: pos += (target - pos) * speed * dt
    Vector2 diff = mTargetPosition - mPosition;
    float distance = diff.Length();
    
    // If close enough, snap
    if (distance < 1.0f)
    {
        mPosition = mTargetPosition;
    }
    else
    {
        // Use a faster transition when distance is large, or constant speed?
        // User asked for "shift smoothly".
        // Lerp is good.
        mPosition = mPosition + diff * mTransitionSpeed * deltaTime;
    }
}
