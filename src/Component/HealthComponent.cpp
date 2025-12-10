#include "HealthComponent.hpp"
#include "HungerComponent.hpp"
#include "ThirstComponent.hpp"
#include "WeightComponent.hpp"
#include "../Actor/Actor.hpp"
#include <SDL.h>
#include <algorithm>
#include <typeinfo>

HealthComponent::HealthComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mCurrentHealth(100.0f)
    , mMaxHealth(100.0f)
    , mDeathTriggered(false)
    , mDeathCallback(nullptr)
    , mOnDamageCallback(nullptr)
{
}

void HealthComponent::TakeDamage(float damage)
{
    if (IsDead()) return;

    float oldHealth = mCurrentHealth;
    mCurrentHealth -= damage;
    mCurrentHealth = std::max(0.0f, mCurrentHealth);
    
    SDL_Log("TakeDamage: %.2f -> %.2f (Damage: %.2f)", oldHealth, mCurrentHealth, damage);

    // Update recent damage
    mRecentDamage = damage;

    // Notify damage
    if (mOnDamageCallback)
    {
        mOnDamageCallback(damage);
    }

    // Check if we just died
    if (IsDead() && !mDeathTriggered)
    {
        mDeathTriggered = true;
        SDL_Log("Health reached 0. Checking death callback...");
        if (mDeathCallback)
        {
            const char* actorType = typeid(*mOwner).name();
            SDL_Log("Actor died! Type: %s, Final health: 0.0", actorType);
            mDeathCallback();
        }
        else
        {
            SDL_Log("Death callback is NULL!");
        }
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
    if (IsDead() && !mDeathTriggered)
    {
        mDeathTriggered = true;
        if (mDeathCallback)
        {
            mDeathCallback();
        }
    }
}


void HealthComponent::UpdateVitalityBar(float hunger, float thirst)
{
    // This function updates the vitality bar based on hunger and thirst ratios
    // Can be extended later to implement specific UI logic
}

void HealthComponent::Update(float deltaTime)
{
    // Calculate total strain/damage from various sources
    float healthDamage = mMaxHealth - mCurrentHealth;
    float hungerDamage = 0.0f;
    float thirstDamage = 0.0f;
    float weightDamage = 0.0f;

    auto* hungerComp = mOwner->GetComponent<HungerComponent>();
    if (hungerComp)
    {
        hungerDamage = hungerComp->GetCurrentHunger();
    }

    auto* thirstComp = mOwner->GetComponent<ThirstComponent>();
    if (thirstComp)
    {
        thirstDamage = thirstComp->GetCurrentThirst();
    }

    auto* weightComp = mOwner->GetComponent<WeightComponent>();
    if (weightComp)
    {
        weightDamage = weightComp->GetWeightRatio() * 100.0f;
    }

    float totalStrain = healthDamage + hungerDamage + thirstDamage + weightDamage;

    // If total strain reaches 100, the character dies
    if (totalStrain >= 100.0f && !mDeathTriggered)
    {
        mDeathTriggered = true;
        mCurrentHealth = 0.0f; // Force health to 0
        
        SDL_Log("Death by Strain! HealthDmg: %.1f, Hunger: %.1f, Thirst: %.1f, Weight: %.1f (Total: %.1f)", 
                healthDamage, hungerDamage, thirstDamage, weightDamage, totalStrain);

        if (mDeathCallback)
        {
            mDeathCallback();
        }
    }
    else if (IsDead() && !mDeathTriggered)
    {
        mDeathTriggered = true;
        SDL_Log("HealthComponent::Update - Health is 0, triggering death callback");
        if (mDeathCallback)
        {
            mDeathCallback();
        }
    }
}

