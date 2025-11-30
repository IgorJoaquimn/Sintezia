#pragma once
#include "Component.hpp"
#include <functional>

// Component that manages an actor's hunger level
class HungerComponent : public Component
{
public:
    HungerComponent(class Actor* owner, int updateOrder = 100);

    // Hunger management
    void IncreaseHunger(float amount);
    void DecreaseHunger(float amount);
    void SetMaxHunger(float maxHunger);
    void SetCurrentHunger(float hunger);

    // Getters
    float GetCurrentHunger() const { return mCurrentHunger; }
    float GetMaxHunger() const { return mMaxHunger; }
    bool IsStarving() const { return mCurrentHunger >= mMaxHunger; }

    // Starvation callback - called when hunger reaches max
    void SetStarvationCallback(std::function<void()> callback) { mStarvationCallback = callback; }

    // Update method for periodic hunger updates
    void Update(float deltaTime);

private:
    float mCurrentHunger;
    float mMaxHunger;
    std::function<void()> mStarvationCallback;
    float mTimeAccumulator;
};