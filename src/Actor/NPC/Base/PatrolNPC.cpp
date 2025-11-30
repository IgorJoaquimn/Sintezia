#include "PatrolNPC.hpp"
#include "../../../Game/Game.hpp"
#include "../../Player.hpp"
#include "../../../Map/TiledParser.hpp"
#include "../../../Core/TextRenderer/TextRenderer.hpp"
#include "../../../Core/Texture/SpriteRenderer.hpp"
#include "../../../Component/AnimationComponent.hpp"
#include "../../../Component/SpriteComponent.hpp"
#include "../../../Component/MovementComponent.hpp"
#include "../../../Component/HealthComponent.hpp"
#include "../../../Component/AttackComponent.hpp"
#include <cmath>
#include <algorithm>

PatrolNPC::PatrolNPC(Game* game, bool isAggressive)
    : NPC(game)
    , mState(PatrolNPCState::Patrolling)
    , mIsAggressive(isAggressive)
    , mCurrentWaypointIndex(0)
    , mWaitTimer(0.0f)
    , mMovementSpeed(100.0f)
    , mAnchorPosition(Vector2::Zero)
    , mAggroRange(150.0f)
    , mDeaggroRange(400.0f)
    , mChaseSpeed(150.0f)
    , mMaxChaseDistance(300.0f)
    , mAttackComponent(nullptr)
    , mCurrentDirection(0)
    , mIsMoving(false)
    , mIsAttackAnimPlaying(false)
{
    // Attack rows: down=6, left=7, right=7, up=8
    mAttackRows[0] = 6; mAttackRows[1] = 7; mAttackRows[2] = 7; mAttackRows[3] = 8;

    if (mHealthComponent)
    {
        mHealthComponent->SetMaxHealth(50.0f);
        mHealthComponent->SetCurrentHealth(50.0f);
        mHealthComponent->SetDeathCallback([this]() {
            SetState(ActorState::Destroy);
        });
    }

    if (isAggressive)
    {
        mAttackComponent = AddComponent<AttackComponent>();

        AttackConfig attackConfig;
        attackConfig.damage = 15.0f;
        attackConfig.cooldown = 1.5f;
        attackConfig.range = 50.0f;
        attackConfig.knockback = 100.0f;
        attackConfig.attackDownRow = mAttackRows[0];
        attackConfig.attackRightRow = mAttackRows[2];
        attackConfig.attackUpRow = mAttackRows[3];
        // Single-frame attack stance
        attackConfig.attackFrameCount = 1;
        attackConfig.attackDuration = 0.4f;
        mAttackComponent->SetAttackConfig(attackConfig);

        // When attack starts, force a single-frame animation
        mAttackComponent->SetAttackStartCallback([this](int direction) {
            mIsAttackAnimPlaying = true;
            if (mAnimationComponent) {
                mAnimationComponent->SetFrameCount(1);
                mAnimationComponent->ResetAnimation();
            }
        });

        // Restore animation frames when attack ends
        mAttackComponent->SetAttackEndCallback([this]() {
            mIsAttackAnimPlaying = false;
            if (mAnimationComponent) {
                int frameCount = mIsMoving ? mWalkFrames : mIdleFrames;
                mAnimationComponent->SetFrameCount(frameCount);
                mAnimationComponent->ResetAnimation();
            }
        });
    }

    mAnimationComponent->SetFrameCount(mIdleFrames);
    mAnimationComponent->SetAnimSpeed(mAnimSpeed);

    mAnchorPosition = GetPosition();
}

PatrolNPC::~PatrolNPC()
{
}

void PatrolNPC::OnUpdate(float deltaTime)
{
    NPC::OnUpdate(deltaTime);

    switch (mState)
    {
        case PatrolNPCState::Patrolling:
            UpdatePatrolling(deltaTime);
            break;
        case PatrolNPCState::Chasing:
            UpdateChasing(deltaTime);
            break;
        case PatrolNPCState::Returning:
            UpdateReturning(deltaTime);
            break;
    }
}

void PatrolNPC::UpdatePatrolling(float deltaTime)
{
    if (mIsAggressive && IsPlayerInRange(mAggroRange))
    {
        mState = PatrolNPCState::Chasing;
        return;
    }

    if (mWaypoints.empty())
    {
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
        return;
    }

    if (mWaitTimer > 0.0f)
    {
        mWaitTimer -= deltaTime;
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
        return;
    }

    const Waypoint& targetWaypoint = mWaypoints[mCurrentWaypointIndex];
    Vector2 currentPos = GetPosition();
    Vector2 toTarget = targetWaypoint.position - currentPos;
    float distanceToWaypoint = toTarget.Length();

    if (distanceToWaypoint < 5.0f)
    {
        mWaitTimer = targetWaypoint.waitTime;
        mCurrentWaypointIndex = (mCurrentWaypointIndex + 1) % mWaypoints.size();
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
    }
    else
    {
        MoveTowards(targetWaypoint.position, mMovementSpeed, deltaTime);
        mIsMoving = true;
    }
}

void PatrolNPC::UpdateChasing(float deltaTime)
{
    Player* player = mGame->GetPlayer();
    if (!player)
    {
        mState = PatrolNPCState::Returning;
        return;
    }

    Vector2 playerPos = player->GetPosition();
    Vector2 npcPos = GetPosition();

    float distanceToPlayer = (playerPos - npcPos).Length();
    if (distanceToPlayer > mMaxChaseDistance)
    {
        mState = PatrolNPCState::Returning;
        return;
    }

    float npcDistanceFromAnchor = (npcPos - mAnchorPosition).Length();
    if (npcDistanceFromAnchor > mDeaggroRange)
    {
        mState = PatrolNPCState::Returning;
        return;
    }

    if (mAttackComponent && mAttackComponent->CanAttack())
    {
        float attackRange = mAttackComponent->GetAttackConfig().range;
        if (distanceToPlayer <= attackRange)
        {
            Vector2 toPlayer = playerPos - npcPos;
            toPlayer.Normalize();
            int attackDir = GetDirectionRow(toPlayer);

            mAttackComponent->StartAttack(attackDir);

            mMovementComponent->SetVelocity(Vector2::Zero);
            mIsMoving = false;
            return;
        }
    }

    MoveTowards(playerPos, mChaseSpeed, deltaTime);
    mIsMoving = true;
}

void PatrolNPC::UpdateReturning(float deltaTime)
{
    if (mIsAggressive && IsPlayerInRange(mAggroRange))
    {
        mState = PatrolNPCState::Chasing;
        return;
    }

    Vector2 currentPos = GetPosition();
    float distanceToAnchor = (mAnchorPosition - currentPos).Length();

    if (distanceToAnchor < 5.0f)
    {
        SetPosition(mAnchorPosition);
        mState = PatrolNPCState::Patrolling;
        mCurrentWaypointIndex = 0;
        mWaitTimer = 0.0f;
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
    }
    else
    {
        MoveTowards(mAnchorPosition, mMovementSpeed, deltaTime);
        mIsMoving = true;
    }
}

void PatrolNPC::MoveTowards(const Vector2& target, float speed, float deltaTime)
{
    Vector2 currentPos = GetPosition();
    Vector2 direction = target - currentPos;
    float distance = direction.Length();

    if (distance > 0.0f)
    {
        direction.Normalize();
        Vector2 velocity = direction * speed;
        mMovementComponent->SetVelocity(velocity);
        UpdateAnimation(velocity);
    }
    else
    {
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
    }
}

void PatrolNPC::UpdateAnimation(const Vector2& velocity)
{
    if (velocity.LengthSq() > 0.1f)
    {
        mCurrentDirection = GetDirectionRow(velocity);
        mIsMoving = true;

        if (mAnimationComponent)
        {
            mAnimationComponent->SetFrameCount(mWalkFrames);
        }
    }
    else
    {
        mIsMoving = false;

        if (mAnimationComponent)
        {
            mAnimationComponent->SetFrameCount(mIdleFrames);
            mAnimationComponent->ResetAnimation();
        }
    }
}

bool PatrolNPC::IsPlayerInRange(float range) const
{
    Player* player = mGame->GetPlayer();
    if (!player) return false;

    Vector2 playerPos = player->GetPosition();
    Vector2 npcPos = GetPosition();
    float distance = (playerPos - npcPos).Length();

    return distance <= range;
}

void PatrolNPC::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    if (!spriteRenderer || !mSpriteComponent || !mAnimationComponent) return;

    int row;
    bool shouldFlip = false;

    if (mIsAttackAnimPlaying && mAttackComponent)
    {
        // Attack stances are on row 4 of the shared spritesheet; columns define orientation
        int attackDir = mAttackComponent->GetAttackDirection();
        int attackRow = 4; // 0-based
        int attackCol = 0;

        // Direction encoding: [0=down, 1=left, 2=right, 3=up]
        // Map to sprite sheet columns: down=0, left=2, right=3, up=1
        if (attackDir == 0) attackCol = 0;
        else if (attackDir == 1) attackCol = 2;
        else if (attackDir == 2) attackCol = 3;
        else attackCol = 1;

        row = attackRow;
        shouldFlip = false;

        int col = attackCol;

        if (mUseColumnBasedDirection)
        {
            mSpriteComponent->SetCurrentFrame(row, col);
            mSpriteComponent->SetFlipHorizontal(false);
            mSpriteComponent->Draw(spriteRenderer);
            return;
        }
        else
        {
            mSpriteComponent->SetCurrentFrame(row, col);
            mSpriteComponent->SetFlipHorizontal(false);
            mSpriteComponent->Draw(spriteRenderer);
            return;
        }
    }
    else if (mIsMoving)
    {
        row = mWalkRows[mCurrentDirection];
        shouldFlip = mUseHorizontalFlip && (mCurrentDirection == 1);
    }
    else
    {
        row = mIdleRows[mCurrentDirection];
        shouldFlip = mUseHorizontalFlip && (mCurrentDirection == 1);
    }

    int col = mAnimationComponent->GetCurrentFrame();

    if (mUseColumnBasedDirection)
    {
        int directionCol = mCurrentDirection;
        if (directionCol == 1) directionCol = 2;
        else if (directionCol == 2) directionCol = 3;
        else if (directionCol == 3) directionCol = 1;
        int finalRow = row + col;
        mSpriteComponent->SetCurrentFrame(finalRow, directionCol);
        mSpriteComponent->SetFlipHorizontal(false);
    }
    else
    {
        mSpriteComponent->SetCurrentFrame(row, col);
        mSpriteComponent->SetFlipHorizontal(shouldFlip);
    }

    mSpriteComponent->Draw(spriteRenderer);
}

void PatrolNPC::AddWaypoint(const Vector2& position, float waitTime)
{
    mWaypoints.emplace_back(position, waitTime);
}

void PatrolNPC::SetAttackRows(int down, int left, int right, int up)
{
    mAttackRows[0] = down;
    mAttackRows[1] = left;
    mAttackRows[2] = right;
    mAttackRows[3] = up;

    if (mAttackComponent)
    {
        mAttackComponent->SetAttackAnimationRows(down, right, up);
    }
}
