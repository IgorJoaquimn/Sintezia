#pragma once
#include "NPC.hpp"
#include "../../../MathUtils.h"
#include <vector>
#include <SDL.h>

// Forward declarations
class HealthComponent;
class AttackComponent;

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

class PatrolNPC : public NPC
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

    // Animation row mapping for attack animations
    void SetAttackRows(int down, int left, int right, int up);

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
    HealthComponent* mHealthComponent;
    AttackComponent* mAttackComponent;

    // Animation state
    int mCurrentDirection;  // 0=down, 1=left, 2=right, 3=up
    bool mIsMoving;
    bool mIsAttackAnimPlaying;

    // Attack animation row mappings
    int mAttackRows[4]; // [down, left, right, up]
};

