#include "PatrolNPC.hpp"
#include "../Game/Game.hpp"
#include "../Actor/Player.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Component/AnimationComponent.hpp"
#include "../Component/SpriteComponent.hpp"
#include "../Component/MovementComponent.hpp"
#include <cmath>
#include <algorithm>

PatrolNPC::PatrolNPC(Game* game, bool isAggressive)
    : Actor(game)
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
    , mAnimationComponent(nullptr)
    , mSpriteComponent(nullptr)
    , mMovementComponent(nullptr)
    , mSpriteWidth(32)
    , mSpriteHeight(32)
    , mIdleFrames(6)
    , mWalkFrames(6)
    , mAnimSpeed(8.0f)
    , mCurrentDirection(0)
    , mIsMoving(false)
{
    // Initialize default animation row mappings (standard 8-row sprite sheet layout)
    // Idle rows: down=0, left=1, right=2, up=3
    mIdleRows[0] = 0; mIdleRows[1] = 1; mIdleRows[2] = 2; mIdleRows[3] = 3;
    // Walk rows: down=4, left=5, right=6, up=7
    mWalkRows[0] = 4; mWalkRows[1] = 5; mWalkRows[2] = 6; mWalkRows[3] = 7;

    // Create and add components
    mAnimationComponent = AddComponent<AnimationComponent>();
    mSpriteComponent = AddComponent<SpriteComponent>(200); // Higher update order for rendering
    mMovementComponent = AddComponent<MovementComponent>();

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
        // Idle - use idle animation
        mIsMoving = false;

        if (mAnimationComponent)
        {
            mAnimationComponent->SetFrameCount(mIdleFrames);
        }
    }
}

int PatrolNPC::GetDirectionRow(const Vector2& velocity) const
{
    // Determine facing direction based on velocity
    // Rows: 0=idle_down, 1=idle_left, 2=idle_right, 3=idle_up,
    //       4=walk_down, 5=walk_left, 6=walk_right, 7=walk_up

    float absX = std::abs(velocity.x);
    float absY = std::abs(velocity.y);

    if (absX > absY)
    {
        // Moving more horizontally
        return velocity.x > 0 ? 2 : 1;  // Right : Left
    }
    else
    {
        // Moving more vertically
        return velocity.y > 0 ? 0 : 3;  // Down : Up
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
    if (mIsMoving)
    {
        row = mWalkRows[mCurrentDirection];
    }
    else
    {
        row = mIdleRows[mCurrentDirection];
    }

    int col = mAnimationComponent->GetCurrentFrame();

    // Configure sprite component for rendering
    mSpriteComponent->SetCurrentFrame(row, col);

    // Flip horizontally when facing left (direction 1)
    // This allows reusing right-facing animations for left direction
    bool shouldFlip = (mCurrentDirection == 1);
    mSpriteComponent->SetFlipHorizontal(shouldFlip);

    // Draw the sprite
    mSpriteComponent->Draw(spriteRenderer);
}

void PatrolNPC::AddWaypoint(const Vector2& position, float waitTime)
{
    mWaypoints.emplace_back(position, waitTime);
}

void PatrolNPC::LoadSpriteSheet(const std::string& filepath)
{
    if (mSpriteComponent)
    {
        mSpriteComponent->LoadSpriteSheet(filepath);
    }
}

void PatrolNPC::SetSpriteConfiguration(int width, int height, int idleFrames, int walkFrames, float animSpeed)
{
    mSpriteWidth = width;
    mSpriteHeight = height;
    mIdleFrames = idleFrames;
    mWalkFrames = walkFrames;
    mAnimSpeed = animSpeed;

    if (mSpriteComponent)
    {
        mSpriteComponent->SetSpriteSize(width, height);
        mSpriteComponent->SetRenderSize(80.0f);
    }

    if (mAnimationComponent)
    {
        mAnimationComponent->SetFrameCount(idleFrames);
        mAnimationComponent->SetAnimSpeed(animSpeed);
    }
}

void PatrolNPC::SetIdleRows(int down, int left, int right, int up)
{
    mIdleRows[0] = down;
    mIdleRows[1] = left;
    mIdleRows[2] = right;
    mIdleRows[3] = up;
}

void PatrolNPC::SetWalkRows(int down, int left, int right, int up)
{
    mWalkRows[0] = down;
    mWalkRows[1] = left;
    mWalkRows[2] = right;
    mWalkRows[3] = up;
}

