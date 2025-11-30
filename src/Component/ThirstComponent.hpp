#pragma once
#include "Component.hpp"
#include <functional>

// Component that manages an actor's thirst level
class ThirstComponent : public Component
{
public:
    ThirstComponent(class Actor* owner, int updateOrder = 100);

    // Thirst management
    void IncreaseThirst(float amount);
    void DecreaseThirst(float amount);
    void SetMaxThirst(float maxThirst);
    void SetCurrentThirst(float thirst);

    // Getters
    float GetCurrentThirst() const { return mCurrentThirst; }
    float GetMaxThirst() const { return mMaxThirst; }
    bool IsDehydrated() const { return mCurrentThirst >= mMaxThirst; }

    // Dehydration callback - called when thirst reaches max
    void SetDehydrationCallback(std::function<void()> callback) { mDehydrationCallback = callback; }

    // Update method for periodic thirst updates
    void Update(float deltaTime);

private:
    float mCurrentThirst;
    float mMaxThirst;
    std::function<void()> mDehydrationCallback;
    float mTimeAccumulator;
};