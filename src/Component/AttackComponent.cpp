#include "AttackComponent.hpp"
#include "HealthComponent.hpp"
#include "MovementComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Game/Game.hpp"
#include <SDL.h>
#include <cmath>

AttackComponent::AttackComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mConfig()
    , mIsAttacking(false)
    , mAttackTimer(0.0f)
    , mCooldownTimer(0.0f)
    , mAttackDirection(0)
    , mAttackStartCallback(nullptr)
    , mAttackEndCallback(nullptr)
{
}

void AttackComponent::Update(float deltaTime)
{
    // Update cooldown timer
    if (mCooldownTimer > 0.0f)
    {
        mCooldownTimer -= deltaTime;
    }

    // Update attack timer
    if (mIsAttacking)
    {
        mAttackTimer -= deltaTime;

        if (mAttackTimer <= 0.0f)
        {
            mIsAttacking = false;

            // Call attack end callback
            if (mAttackEndCallback)
            {
                mAttackEndCallback();
            }
        }
    }
}

void AttackComponent::StartAttack(int direction)
{
    if (!CanAttack()) return;

    mIsAttacking = true;
    mAttackTimer = mConfig.attackDuration;
    mCooldownTimer = mConfig.cooldown;
    mAttackDirection = direction;

    // Call attack start callback (for animation)
    if (mAttackStartCallback)
    {
        mAttackStartCallback(direction);
    }

    // Perform the attack immediately
    PerformAttack();
}

void AttackComponent::PerformAttack()
{
    std::vector<Actor*> targets;
    FindTargetsInRange(targets);

    Vector2 ownerPos = mOwner->GetPosition();
    Vector2 attackDir;

    // Determine attack direction vector (matches PlayerInputComponent: 0=Down, 1=Right, 2=Up, 3=Left)
    switch (mAttackDirection)
    {
        case 0: attackDir = Vector2(0.0f, 1.0f); break;   // Down
        case 1: attackDir = Vector2(1.0f, 0.0f); break;   // Right
        case 2: attackDir = Vector2(0.0f, -1.0f); break;  // Up
        case 3: attackDir = Vector2(-1.0f, 0.0f); break;  // Left
        default: attackDir = Vector2(0.0f, 1.0f); break;
    }

    SDL_Log("Attack triggered! Direction: %d, Found %zu targets in range %.1f",
            mAttackDirection, targets.size(), mConfig.range);

    // Apply damage and knockback to all targets
    for (Actor* target : targets)
    {
        Vector2 targetPos = target->GetPosition();
        Vector2 toTarget = targetPos - ownerPos;
        float distance = toTarget.Length();
        toTarget.Normalize();

        // Check if target is roughly in the attack direction
        float dot = Vector2::Dot(attackDir, toTarget);

        SDL_Log("  Target at distance %.1f, dot product: %.2f (attackDir=(%.2f,%.2f), toTarget=(%.2f,%.2f))",
                distance, dot, attackDir.x, attackDir.y, toTarget.x, toTarget.y);

        if (dot > 0.0f) // Target is in front (90 degree cone)
        {
            SDL_Log("  -> HIT! Applying damage");
            ApplyDamageAndKnockback(target, toTarget);
        }
        else
        {
            SDL_Log("  -> MISS! Target not in attack cone");
        }
    }
}

void AttackComponent::FindTargetsInRange(std::vector<Actor*>& targets)
{
    Game* game = mOwner->GetGame();
    if (!game) return;

    Vector2 ownerPos = mOwner->GetPosition();

    // Get all actors from the game
    const auto& actors = game->GetActors();

    for (const auto& actorPtr : actors)
    {
        Actor* actor = actorPtr.get();

        // Skip self
        if (actor == mOwner) continue;

        // Check if actor has health component (is damageable)
        HealthComponent* healthComp = actor->GetComponent<HealthComponent>();
        if (!healthComp) continue;

        // Check if in range
        Vector2 targetPos = actor->GetPosition();
        float distance = (targetPos - ownerPos).Length();

        if (distance <= mConfig.range)
        {
            targets.push_back(actor);
        }
    }
}

void AttackComponent::ApplyDamageAndKnockback(Actor* target, const Vector2& direction)
{
    // Apply damage
    HealthComponent* healthComp = target->GetComponent<HealthComponent>();
    if (healthComp)
    {
        float healthBefore = healthComp->GetCurrentHealth();
        healthComp->TakeDamage(mConfig.damage);
        float healthAfter = healthComp->GetCurrentHealth();

        SDL_Log("Attack hit! Damage: %.1f | Target health: %.1f -> %.1f",
                mConfig.damage, healthBefore, healthAfter);
    }

    // Apply knockback
    if (mConfig.knockback > 0.0f)
    {
        MovementComponent* movementComp = target->GetComponent<MovementComponent>();
        if (movementComp)
        {
            Vector2 knockbackImpulse = direction * mConfig.knockback;
            movementComp->ApplyImpulse(knockbackImpulse);
            SDL_Log("Knockback applied: direction=(%.2f, %.2f), force=%.1f",
                    direction.x, direction.y, mConfig.knockback);
        }
    }
}

void AttackComponent::SetAttackAnimationRows(int down, int right, int up)
{
    mConfig.attackDownRow = down;
    mConfig.attackRightRow = right;
    mConfig.attackUpRow = up;
}

int AttackComponent::GetAttackAnimationRow() const
{
    if (!mIsAttacking) return -1;

    switch (mAttackDirection)
    {
        case 0: return mConfig.attackDownRow;   // Down
        case 1: return mConfig.attackRightRow;  // Left (will be flipped)
        case 2: return mConfig.attackRightRow;  // Right
        case 3: return mConfig.attackUpRow;     // Up
        default: return mConfig.attackDownRow;
    }
}

float AttackComponent::GetAttackProgress() const
{
    if (!mIsAttacking || mConfig.attackDuration <= 0.0f) return 0.0f;

    float progress = 1.0f - (mAttackTimer / mConfig.attackDuration);
    return std::max(0.0f, std::min(1.0f, progress));
}

