#include "AnimationComponent.hpp"
#include "../Actor/Actor.hpp"

AnimationComponent::AnimationComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mAnimTime(0.0f)
    , mAnimFrame(0)
    , mMaxFrames(6)
    , mAnimSpeed(8.0f)
{
}

void AnimationComponent::Update(float deltaTime)
{
    mAnimTime += deltaTime;
    float frameTime = 1.0f / mAnimSpeed;
    
    if (mAnimTime >= frameTime)
    {
        mAnimTime -= frameTime;
        mAnimFrame = (mAnimFrame + 1) % mMaxFrames;
    }
}
