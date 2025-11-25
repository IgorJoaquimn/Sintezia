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
    , mHealthComponent(nullptr)
    , mAttackComponent(nullptr)
    , mCurrentDirection(0)
    , mIsMoving(false)
    , mIsAttackAnimPlaying(false)
{
    // Initialize attack animation row mappings
    // Attack rows: down=6, left=7, right=7, up=8 (as specified)
    mAttackRows[0] = 6; mAttackRows[1] = 7; mAttackRows[2] = 7; mAttackRows[3] = 8;

    // Create patrol-specific components
    mHealthComponent = AddComponent<HealthComponent>();

    // Configure health component
    mHealthComponent->SetMaxHealth(50.0f);
    mHealthComponent->SetCurrentHealth(50.0f);
    mHealthComponent->SetDeathCallback([this]() {
        // NPC death - mark for destruction
        SetState(ActorState::Destroy);
    });

    // Only add attack component if aggressive
    if (isAggressive)
    {
        mAttackComponent = AddComponent<AttackComponent>();

        // Configure attack component
        AttackConfig attackConfig;
        attackConfig.damage = 15.0f;
        attackConfig.cooldown = 1.5f;
        attackConfig.range = 50.0f;
        attackConfig.knockback = 100.0f;
        attackConfig.attackDownRow = mAttackRows[0];
        attackConfig.attackRightRow = mAttackRows[2];
        attackConfig.attackUpRow = mAttackRows[3];
        attackConfig.attackFrameCount = 6;
        attackConfig.attackDuration = 0.4f;
        mAttackComponent->SetAttackConfig(attackConfig);

        // Set attack callbacks
        mAttackComponent->SetAttackStartCallback([this](int direction) {
            mIsAttackAnimPlaying = true;
            if (mAnimationComponent) {
                mAnimationComponent->SetFrameCount(mAttackComponent->GetAttackConfig().attackFrameCount);
                mAnimationComponent->ResetAnimation();
            }
        });

        mAttackComponent->SetAttackEndCallback([this]() {
            mIsAttackAnimPlaying = false;
        });
    }

    // Configure animation component with default values
    mAnimationComponent->SetFrameCount(mIdleFrames);
    mAnimationComponent->SetAnimSpeed(mAnimSpeed);

    // Set anchor position to initial position
    mAnchorPosition = GetPosition();
}

PatrolNPC::~PatrolNPC()
{
}

void PatrolNPC::OnUpdate(float deltaTime)
{
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
    // Check if aggressive NPC should start chasing
    if (mIsAggressive && IsPlayerInRange(mAggroRange))
    {
        mState = PatrolNPCState::Chasing;
        return;
    }

    // If no waypoints, stay idle
    if (mWaypoints.empty())
    {
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
        return;
    }

    // If waiting at waypoint
    if (mWaitTimer > 0.0f)
    {
        mWaitTimer -= deltaTime;
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
        return;
    }

    // Move towards current waypoint
    const Waypoint& targetWaypoint = mWaypoints[mCurrentWaypointIndex];
    Vector2 currentPos = GetPosition();
    Vector2 toTarget = targetWaypoint.position - currentPos;
    float distanceToWaypoint = toTarget.Length();

    // Check if reached waypoint
    if (distanceToWaypoint < 5.0f)
    {
        // Start waiting at this waypoint
        mWaitTimer = targetWaypoint.waitTime;

        // Move to next waypoint
        mCurrentWaypointIndex = (mCurrentWaypointIndex + 1) % mWaypoints.size();

        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
    }
    else
    {
        // Move towards waypoint
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

    // Check if player has gone too far from the NPC
    float distanceToPlayer = (playerPos - npcPos).Length();
    if (distanceToPlayer > mMaxChaseDistance)
    {
        mState = PatrolNPCState::Returning;
        return;
    }

    // Check if NPC has left deaggro range from its anchor
    float npcDistanceFromAnchor = (npcPos - mAnchorPosition).Length();
    if (npcDistanceFromAnchor > mDeaggroRange)
    {
        mState = PatrolNPCState::Returning;
        return;
    }

    // Try to attack if in range and has attack component
    if (mAttackComponent && mAttackComponent->CanAttack())
    {
        float attackRange = mAttackComponent->GetAttackConfig().range;
        if (distanceToPlayer <= attackRange)
        {
            // Determine attack direction based on player position
            Vector2 toPlayer = playerPos - npcPos;
            toPlayer.Normalize();
            int attackDir = GetDirectionRow(toPlayer);

            // Trigger attack
            mAttackComponent->StartAttack(attackDir);

            // Stop moving during attack
            mMovementComponent->SetVelocity(Vector2::Zero);
            mIsMoving = false;
            return;
        }
    }

    // Chase the player
    MoveTowards(playerPos, mChaseSpeed, deltaTime);
    mIsMoving = true;
}

void PatrolNPC::UpdateReturning(float deltaTime)
{
    // Check if player comes back into aggro range while returning
    if (mIsAggressive && IsPlayerInRange(mAggroRange))
    {
        mState = PatrolNPCState::Chasing;
        return;
    }

    // Move back to anchor position
    Vector2 currentPos = GetPosition();
    float distanceToAnchor = (mAnchorPosition - currentPos).Length();

    if (distanceToAnchor < 5.0f)
    {
        // Reached anchor, resume patrolling
        SetPosition(mAnchorPosition);
        mState = PatrolNPCState::Patrolling;
        mCurrentWaypointIndex = 0;  // Reset to first waypoint
        mWaitTimer = 0.0f;
        mMovementComponent->SetVelocity(Vector2::Zero);
        mIsMoving = false;
        UpdateAnimation(Vector2::Zero);
    }
    else
    {
        // Move towards anchor
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

        // Set velocity for movement component
        mMovementComponent->SetVelocity(velocity);

        // Update animation based on movement direction
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
        // Moving - use walk animation
        mCurrentDirection = GetDirectionRow(velocity);
        mIsMoving = true;

        if (mAnimationComponent)
        {
            mAnimationComponent->SetFrameCount(mWalkFrames);
        }
    }
    else
    {
        // Idle - use idle animation and reset to first frame
        mIsMoving = false;

        if (mAnimationComponent)
        {
            mAnimationComponent->SetFrameCount(mIdleFrames);
            mAnimationComponent->ResetAnimation(); // Stay on first idle frame
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

    // Get the appropriate row from custom mappings based on state and direction
    int row;
    bool shouldFlip = false;

    if (mIsAttackAnimPlaying && mAttackComponent)
    {
        // Playing attack animation
        int attackDir = mAttackComponent->GetAttackDirection();

        // Map direction to attack row using [down, left, right, up] pattern
        if (attackDir == 0) // Down
            row = mAttackRows[0];
        else if (attackDir == 1) // Left
        {
            row = mAttackRows[1];
            shouldFlip = true; // Flip if left uses different row
        }
        else if (attackDir == 2) // Right
            row = mAttackRows[2];
        else // Up (attackDir == 3)
            row = mAttackRows[3];
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

    // Configure sprite component for rendering
    if (mUseColumnBasedDirection)
    {
        // For column-based sprites: direction is column, animation frame is row
        // Sprite columns: [0=Down, 1=Up, 2=Left, 3=Right]
        // Code direction: [0=down, 1=left, 2=right, 3=up]
        // Remap direction to column: down=0->0, left=1->2, right=2->3, up=3->1
        int directionCol = mCurrentDirection;
        if (directionCol == 1) directionCol = 2;      // left (1) -> column 2
        else if (directionCol == 2) directionCol = 3; // right (2) -> column 3
        else if (directionCol == 3) directionCol = 1; // up (3) -> column 1
        // down (0) stays 0
        
        // row contains the base row (0 for idle, 1 for walk start)
        // col contains the animation frame offset
        int finalRow = row + col;
        
        mSpriteComponent->SetCurrentFrame(finalRow, directionCol);
        mSpriteComponent->SetFlipHorizontal(false);  // Column-based sprites don't need flipping
    }
    else
    {
        // For row-based sprites: direction is row, animation frame is column
        mSpriteComponent->SetCurrentFrame(row, col);
        mSpriteComponent->SetFlipHorizontal(shouldFlip);
    }

    // Draw the sprite
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

    // Update attack component configuration if it exists
    if (mAttackComponent)
    {
        mAttackComponent->SetAttackAnimationRows(down, right, up);
    }
}

