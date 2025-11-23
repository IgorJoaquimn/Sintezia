#pragma once
#include "Component.hpp"
#include "../Math.h"

// Component that handles physics/movement updates
class MovementComponent : public Component
{
public:
    MovementComponent(class Actor* owner, int updateOrder = 100);
    
    void Update(float deltaTime) override;
    
    // Set the velocity for this frame
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }
    const Vector2& GetVelocity() const { return mVelocity; }
    
    // Enable/disable bounds checking
    void SetBoundsChecking(bool enabled) { mUseBounds = enabled; }
    void SetBounds(float minX, float minY, float maxX, float maxY);
    
private:
    Vector2 mVelocity;
    bool mUseBounds;
    float mMinX, mMinY, mMaxX, mMaxY;
};
