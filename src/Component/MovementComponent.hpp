#pragma once
#include "Component.hpp"
#include "../MathUtils.h"

// Component that handles physics/movement updates
class MovementComponent : public Component
{
public:
    MovementComponent(class Actor* owner, int updateOrder = 100);
    
    void Update(float deltaTime) override;
    
    // Set the velocity for this frame
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }
    const Vector2& GetVelocity() const { return mVelocity; }
    
    // Apply an impulse (for knockback, etc.)
    void ApplyImpulse(const Vector2& impulse);

    // Enable/disable bounds checking
    void SetBoundsChecking(bool enabled) { mUseBounds = enabled; }
    void SetBounds(float minX, float minY, float maxX, float maxY);
    
private:
    Vector2 mVelocity;
    Vector2 mImpulseVelocity;  // Additional velocity from impulses (knockback)
    float mImpulseDecay;        // How fast impulse decays (friction)
    bool mUseBounds;
    float mMinX, mMinY, mMaxX, mMaxY;
};
