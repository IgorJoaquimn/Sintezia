#include "ThirstComponent.hpp"
#include "../Actor/Actor.hpp"
#include <algorithm>

ThirstComponent::ThirstComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mCurrentThirst(0.0f)
    , mMaxThirst(100.0f)
    , mDehydrationCallback(nullptr)
    , mTimeAccumulator(0.0f)
    , mDamageTimer(0.0f)
{
}

void ThirstComponent::IncreaseThirst(float amount)
{
    mCurrentThirst += amount;
    mCurrentThirst = std::min(mMaxThirst, mCurrentThirst);

    // Check if we are dehydrated
    if (IsDehydrated() && mDehydrationCallback)
    {
        mDehydrationCallback();
    }
}

void ThirstComponent::DecreaseThirst(float amount)
{
    mCurrentThirst -= amount;
    mCurrentThirst = std::max(0.0f, mCurrentThirst);
}

void ThirstComponent::SetMaxThirst(float maxThirst)
{
    mMaxThirst = maxThirst;
}

void ThirstComponent::SetCurrentThirst(float thirst)
{
    mCurrentThirst = thirst;
}

void ThirstComponent::Update(float deltaTime)
{
    mTimeAccumulator += deltaTime;
    if (mTimeAccumulator >= 60.0f) // 1 minute
    {
        IncreaseThirst(10.0f);
        mTimeAccumulator = 0.0f;
    }

    if (IsDehydrated())
    {
        mDamageTimer += deltaTime;
        if (mDamageTimer >= 2.0f) // Damage every 2 seconds
        {
            if (mDehydrationCallback)
            {
                mDehydrationCallback();
            }
            mDamageTimer = 0.0f;
        }
    }
    else
    {
        mDamageTimer = 0.0f;
    }
}