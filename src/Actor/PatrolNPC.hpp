#pragma once
#include "Actor.hpp"
#include "../MathUtils.h"
#include <vector>
#include <SDL.h>

// Forward declarations
class AnimationComponent;
class SpriteComponent;
class MovementComponent;

// Represents a waypoint in the NPC's patrol loop
struct Waypoint
{
    Vector2 position;
    float waitTime;  // Time to wait at this waypoint before moving to the next

    Waypoint(const Vector2& pos, float wait = 0.0f)
        : position(pos), waitTime(wait) {}
};

enum class PatrolNPCState
{
    Patrolling,     // Following the patrol loop
    Chasing,        // Pursuing the player (aggressive NPCs only)
    Returning       // Returning to anchor position after losing aggro
};

class PatrolNPC : public Actor
{
public:
    PatrolNPC(class Game* game, bool isAggressive = false);
    virtual ~PatrolNPC();

    void OnUpdate(float deltaTime) override;
    void OnDraw(class TextRenderer* textRenderer) override;

    // Movement configuration
    void AddWaypoint(const Vector2& position, float waitTime = 0.0f);
    void SetMovementSpeed(float speed) { mMovementSpeed = speed; }
    void SetAnchorPosition(const Vector2& anchor) { mAnchorPosition = anchor; }

    // Aggressive NPC configuration
    void SetAggressive(bool aggressive) { mIsAggressive = aggressive; }
    void SetAggroRange(float range) { mAggroRange = range; }
    void SetDeaggroRange(float range) { mDeaggroRange = range; }
    void SetChaseSpeed(float speed) { mChaseSpeed = speed; }
    void SetMaxChaseDistance(float distance) { mMaxChaseDistance = distance; }

    // Sprite configuration
    void LoadSpriteSheet(const std::string& filepath);
    void SetSpriteConfiguration(int width, int height, int idleFrames, int walkFrames, float animSpeed);

    // Animation row mapping (for custom sprite sheet layouts)
    // Set which rows correspond to idle animations for each direction
    void SetIdleRows(int down, int left, int right, int up);
    // Set which rows correspond to walk animations for each direction
    void SetWalkRows(int down, int left, int right, int up);

    // State
    PatrolNPCState GetState() const { return mState; }

protected:
    // Update methods for different states
    void UpdatePatrolling(float deltaTime);
    void UpdateChasing(float deltaTime);
    void UpdateReturning(float deltaTime);

    // Helper methods
    void MoveTowards(const Vector2& target, float speed, float deltaTime);
    void UpdateAnimation(const Vector2& velocity);
    int GetDirectionRow(const Vector2& velocity) const;
    bool IsPlayerInRange(float range) const;

    // NPC state
    PatrolNPCState mState;
    bool mIsAggressive;

    // Movement loop
    std::vector<Waypoint> mWaypoints;
    int mCurrentWaypointIndex;
    float mWaitTimer;
    float mMovementSpeed;

    // Aggressive behavior
    Vector2 mAnchorPosition;  // Fixed position to return to after deaggro
    float mAggroRange;         // Distance to start chasing player
    float mDeaggroRange;       // Distance from anchor to deaggro
    float mChaseSpeed;         // Speed when chasing player
    float mMaxChaseDistance;   // Max distance from NPC to chase player before giving up

    // Components
    AnimationComponent* mAnimationComponent;
    SpriteComponent* mSpriteComponent;
    MovementComponent* mMovementComponent;

    // Sprite configuration
    int mSpriteWidth;
    int mSpriteHeight;
    int mIdleFrames;
    int mWalkFrames;
    float mAnimSpeed;

    // Animation state
    int mCurrentDirection;  // 0=down, 1=left, 2=right, 3=up
    bool mIsMoving;

    // Custom animation row mappings (default assumes standard 8-row layout)
    int mIdleRows[4];  // [down, left, right, up]
    int mWalkRows[4];  // [down, left, right, up]
};

