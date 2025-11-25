#pragma once
#include "Component.hpp"
#include "../MathUtils.h"
#include <functional>
#include <vector>

// Structure to store attack configuration
struct AttackConfig
{
    float damage;
    float cooldown;
    float range;
    float knockback;

    // Animation configuration
    int attackDownRow;     // Row for downward attack animation
    int attackRightRow;    // Row for right attack animation
    int attackUpRow;       // Row for upward attack animation
    int attackFrameCount;  // Number of frames in attack animation
    float attackDuration;  // Duration of attack animation

    AttackConfig()
        : damage(10.0f)
        , cooldown(1.0f)
        , range(50.0f)
        , knockback(100.0f)
        , attackDownRow(6)
        , attackRightRow(7)
        , attackUpRow(8)
        , attackFrameCount(6)
        , attackDuration(0.3f)
    {}
};

// Component that manages an actor's attack capabilities
class AttackComponent : public Component
{
public:
    AttackComponent(class Actor* owner, int updateOrder = 100);

    void Update(float deltaTime) override;

    // Attack control
    void StartAttack(int direction); // direction: 0=down, 1=right, 2=up, 3=left
    bool IsAttacking() const { return mIsAttacking; }
    bool CanAttack() const { return !mIsAttacking && mCooldownTimer <= 0.0f; }

    // Configuration
    void SetAttackConfig(const AttackConfig& config) { mConfig = config; }
    const AttackConfig& GetAttackConfig() const { return mConfig; }

    void SetDamage(float damage) { mConfig.damage = damage; }
    void SetCooldown(float cooldown) { mConfig.cooldown = cooldown; }
    void SetRange(float range) { mConfig.range = range; }
    void SetKnockback(float knockback) { mConfig.knockback = knockback; }

    // Animation configuration
    void SetAttackAnimationRows(int down, int right, int up);
    void SetAttackFrameCount(int frames) { mConfig.attackFrameCount = frames; }
    void SetAttackDuration(float duration) { mConfig.attackDuration = duration; }

    // Get current attack direction and animation info
    int GetAttackDirection() const { return mAttackDirection; }
    int GetAttackAnimationRow() const;
    float GetAttackProgress() const; // Returns 0.0 to 1.0

    // Attack callback - called when attack is triggered to start animation
    void SetAttackStartCallback(std::function<void(int direction)> callback) { mAttackStartCallback = callback; }

    // Attack end callback - called when attack animation finishes
    void SetAttackEndCallback(std::function<void()> callback) { mAttackEndCallback = callback; }

private:
    void PerformAttack();
    void FindTargetsInRange(std::vector<class Actor*>& targets);
    void ApplyDamageAndKnockback(class Actor* target, const Vector2& direction);

    AttackConfig mConfig;

    bool mIsAttacking;
    float mAttackTimer;
    float mCooldownTimer;
    int mAttackDirection;  // 0=down, 1=left, 2=right, 3=up

    std::function<void(int)> mAttackStartCallback;
    std::function<void()> mAttackEndCallback;
};

