#include "HungerComponent.hpp"
#include "../Actor/Actor.hpp"
#include <algorithm>
#include <SDL.h>

HungerComponent::HungerComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mCurrentHunger(0.0f)
    , mMaxHunger(100.0f)
    , mStarvationCallback(nullptr)
    , mTimeAccumulator(0.0f)
    , mDamageTimer(0.0f)
{
}

void HungerComponent::IncreaseHunger(float amount)
{
    mCurrentHunger += amount;
    mCurrentHunger = std::min(mMaxHunger, mCurrentHunger);

    // Check if we are starving
    if (IsStarving() && mStarvationCallback)
    {
        mStarvationCallback();
    }
}

void HungerComponent::DecreaseHunger(float amount)
{
    mCurrentHunger -= amount;
    mCurrentHunger = std::max(0.0f, mCurrentHunger);
}

void HungerComponent::SetMaxHunger(float maxHunger)
{
    mMaxHunger = maxHunger;
}

void HungerComponent::SetCurrentHunger(float hunger)
{
    mCurrentHunger = hunger;
}

void HungerComponent::Update(float deltaTime)
{
    mTimeAccumulator += deltaTime;
    if (mTimeAccumulator >= 120.0f) // 2 minutes
    {
        IncreaseHunger(20.0f);
        mTimeAccumulator = 0.0f;
    }

    if (IsStarving())
    {
        mDamageTimer += deltaTime;
        if (mDamageTimer >= 3.0f) // Damage every 3 seconds (starvation is slower than dehydration)
        {
            if (mStarvationCallback)
            {
                mStarvationCallback();
            }
            mDamageTimer = 0.0f;
        }
    }
    else
    {
        mDamageTimer = 0.0f;
    }
}