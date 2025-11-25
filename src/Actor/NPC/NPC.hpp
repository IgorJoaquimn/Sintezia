#pragma once
#include "../Actor.hpp"
#include <string>

// Forward declarations
class AnimationComponent;
class SpriteComponent;
class MovementComponent;
class Game;
struct TilesetInfo;

// Base class for all NPCs with common sprite/animation functionality
class NPC : public Actor
{
public:
    NPC(Game* game);
    virtual ~NPC();

    // Sprite configuration
    void LoadSpriteSheetFromTSX(const std::string& tsxPath);
    void SetSpriteConfiguration(int width, int height, int idleFrames, int walkFrames, float animSpeed);
    
    // Animation row mapping (for custom sprite sheet layouts)
    void SetIdleRows(int down, int left, int right, int up);
    void SetWalkRows(int down, int left, int right, int up);
    
    // Control horizontal flipping and column-based direction
    void SetUseHorizontalFlip(bool useFlip) { mUseHorizontalFlip = useFlip; }
    void SetUseColumnBasedDirection(bool useColumn) { mUseColumnBasedDirection = useColumn; }

protected:
    // Helper to get direction from velocity
    int GetDirectionRow(const Vector2& velocity) const;
    
    // Components shared by all NPCs
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
    
    // Custom animation row mappings
    int mIdleRows[4];  // [down, left, right, up]
    int mWalkRows[4];  // [down, left, right, up]
    bool mUseHorizontalFlip;  // Whether to flip sprite for left direction
    bool mUseColumnBasedDirection;  // Whether direction is in columns (true) or rows (false)
};
