#pragma once
#include "Component.hpp"

class Inventory;

class WeightComponent : public Component
{
public:
    WeightComponent(class Actor* owner, int updateOrder = 100);

    void Update(float deltaTime) override;

    void SetInventory(Inventory* inventory) { mInventory = inventory; }

    void SetMaxWeight(float maxWeight);
    float GetMaxWeight() const { return mMaxWeight; }

    void SetCurrentWeight(float weight);
    float GetCurrentWeight() const { return mCurrentWeight; }

    float GetWeightRatio() const;

private:
    float mMaxWeight;
    float mCurrentWeight;
    Inventory* mInventory;
};
