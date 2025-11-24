#include "HealthComponent.hpp"
#include "../Actor/Actor.hpp"
#include <SDL.h>
#include <algorithm>

HealthComponent::HealthComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mCurrentHealth(100.0f)
    , mMaxHealth(100.0f)
    , mDeathCallback(nullptr)
{
}

void HealthComponent::TakeDamage(float damage)
{
    if (IsDead()) return;

    mCurrentHealth -= damage;
    mCurrentHealth = std::max(0.0f, mCurrentHealth);

    // Check if we just died
    if (IsDead() && mDeathCallback)
    {
        SDL_Log("Actor died! Final health: 0.0");
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

