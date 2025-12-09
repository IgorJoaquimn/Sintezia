#include "WeightComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Game/Inventory.hpp"
#include <algorithm>

WeightComponent::WeightComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mMaxWeight(100.0f)
    , mCurrentWeight(0.0f)
    , mInventory(nullptr)
{
}

void WeightComponent::Update(float deltaTime)
{
    if (mInventory)
    {
        float totalWeight = 0.0f;
        for (const auto& slot : mInventory->GetAllSlots())
        {
            totalWeight += (float)slot.quantity * 50.0f;
        }
        SetCurrentWeight(totalWeight);
        // Assume max stack size of 64 for weight calculation purposes
        SetMaxWeight((float)mInventory->GetMaxSlots() * 64.0f);
    }
}

void WeightComponent::SetMaxWeight(float maxWeight)
{
    mMaxWeight = maxWeight;
}

void WeightComponent::SetCurrentWeight(float weight)
{
    mCurrentWeight = weight;
}

float WeightComponent::GetWeightRatio() const
{
    if (mMaxWeight <= 0.0f) return 0.0f;
    return std::min(1.0f, std::max(0.0f, mCurrentWeight / mMaxWeight));
}
