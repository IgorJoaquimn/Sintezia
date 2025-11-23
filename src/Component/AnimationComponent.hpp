#pragma once
#include "Component.hpp"

// Component that handles sprite animation frame updates
class AnimationComponent : public Component
{
public:
    AnimationComponent(class Actor* owner, int updateOrder = 100);
    
    void Update(float deltaTime) override;
    
    // Animation control
    void SetFrameCount(int frames) { mMaxFrames = frames; }
    void SetAnimSpeed(float fps) { mAnimSpeed = fps; }
    void ResetAnimation() { mAnimTime = 0.0f; mAnimFrame = 0; }
    
    // Getters
    int GetCurrentFrame() const { return mAnimFrame; }
    
private:
    float mAnimTime;
    int mAnimFrame;
    int mMaxFrames;
    float mAnimSpeed;
};
