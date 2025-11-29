#pragma once
#include "../MathUtils.h"

class Camera
{
public:
    Camera(float width, float height);
    
    void Update(float deltaTime, const Vector2& playerPos, int mapWidth, int mapHeight);
    
    const Vector2& GetPosition() const { return mPosition; }
    void SetPosition(const Vector2& pos) { mPosition = pos; mTargetPosition = pos; }
    
private:
    Vector2 mPosition;
    float mWidth;
    float mHeight;
    
    // For smooth transition
    Vector2 mTargetPosition;
    float mTransitionSpeed;
};
