#pragma once
#include "Component.hpp"
#include <functional>

// Component that manages an actor's health points
class HealthComponent : public Component
{
public:
    HealthComponent(class Actor* owner, int updateOrder = 100);

    // Health management
    void TakeDamage(float damage);
    void Heal(float amount);
    void SetMaxHealth(float maxHealth);
    void SetCurrentHealth(float health);

    // Getters
    float GetCurrentHealth() const { return mCurrentHealth; }
    float GetMaxHealth() const { return mMaxHealth; }
    bool IsDead() const { return mCurrentHealth <= 0.0f; }

    // Death callback - called when health reaches 0 or below
    void SetDeathCallback(std::function<void()> callback) { mDeathCallback = callback; }
    
    // Damage callback - called when taking damage
    void SetOnDamageCallback(std::function<void(float)> callback) { mOnDamageCallback = callback; }

private:
    float mCurrentHealth;
    float mMaxHealth;
    std::function<void()> mDeathCallback;
    std::function<void(float)> mOnDamageCallback;
};

