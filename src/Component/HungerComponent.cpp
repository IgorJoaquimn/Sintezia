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
    if (mTimeAccumulator >= 10.0f)
    {
        IncreaseHunger(1.0f);
        mTimeAccumulator = 0.0f;
    }
}