#pragma once
#include "Component.hpp"
#include "../MathUtils.h"
#include <SDL.h>

// Component that handles player keyboard input
class PlayerInputComponent : public Component
{
public:
    PlayerInputComponent(class Actor* owner, int updateOrder = 100);
    
    void ProcessInput(const uint8_t* keyState) override;
    
    // Getters
    const Vector2& GetVelocity() const { return mVelocity; }
    int GetDirection() const { return mDirection; }
    bool IsMoving() const { return mIsMoving; }
    bool IsAttacking() const { return mIsAttacking; }
    bool IsJumping() const { return mIsJumping; }
    bool IsCrouching() const { return mIsCrouching; }
    
private:
    Vector2 mVelocity;
    int mDirection;  // 0=down, 1=right, 2=up, 3=left
    bool mIsMoving;
    bool mIsAttacking;
    bool mIsJumping;
    bool mIsCrouching;
    float mSpeed;
    
    static constexpr float DEFAULT_SPEED = 200.0f;
};
