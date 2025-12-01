#include "HealthComponent.hpp"
#include "../Actor/Actor.hpp"
#include <SDL.h>
#include <algorithm>
#include <typeinfo>

HealthComponent::HealthComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mCurrentHealth(100.0f)
    , mMaxHealth(100.0f)
    , mDeathCallback(nullptr)
    , mOnDamageCallback(nullptr)
{
}

void HealthComponent::TakeDamage(float damage)
{
    if (IsDead()) return;

    mCurrentHealth -= damage;
    mCurrentHealth = std::max(0.0f, mCurrentHealth);

    // Update recent damage
    mRecentDamage = damage;

    // Notify damage
    if (mOnDamageCallback)
    {
        mOnDamageCallback(damage);
    }

    // Check if we just died
    if (IsDead() && mDeathCallback)
    {
        const char* actorType = typeid(*mOwner).name();
        SDL_Log("Actor died! Type: %s, Final health: 0.0", actorType);
        mDeathCallback();
    }
}

void HealthComponent::Heal(float amount)
{
    if (IsDead()) return;

    mCurrentHealth += amount;
    mCurrentHealth = std::min(mCurrentHealth, mMaxHealth);
}

void HealthComponent::SetMaxHealth(float maxHealth)
{
    mMaxHealth = maxHealth;
    mCurrentHealth = std::min(mCurrentHealth, mMaxHealth);
}

void HealthComponent::SetCurrentHealth(float health)
{
    mCurrentHealth = std::max(0.0f, std::min(health, mMaxHealth));

    // Check if we just died
    if (IsDead() && mDeathCallback)
    {
        mDeathCallback();
    }
}

void HealthComponent::UpdateVitalityBar(float hunger, float thirst)
{
    // Logic to update the vitality bar based on health, hunger, and thirst
}

