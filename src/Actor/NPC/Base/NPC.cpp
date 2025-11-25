#include "NPC.hpp"
#include "../../../Game/Game.hpp"
#include "../../../Component/AnimationComponent.hpp"
#include "../../../Component/SpriteComponent.hpp"
#include "../../../Component/MovementComponent.hpp"
#include "../../../Map/TiledParser.hpp"
#include <cmath>

NPC::NPC(Game* game)
    : Actor(game)
    , mAnimationComponent(nullptr)
    , mSpriteComponent(nullptr)
    , mMovementComponent(nullptr)
    , mSpriteWidth(16)
    , mSpriteHeight(16)
    , mIdleFrames(1)
    , mWalkFrames(4)
    , mAnimSpeed(8.0f)
    , mCurrentDirection(0)
    , mIsMoving(false)
    , mUseHorizontalFlip(false)
    , mUseColumnBasedDirection(true)
{
    // Initialize default animation row mappings for column-based sprites
    // For column-based: each direction is a column, idle=row0, walk=rows0-3
    mIdleRows[0] = 0; mIdleRows[1] = 0; mIdleRows[2] = 0; mIdleRows[3] = 0;
    mWalkRows[0] = 0; mWalkRows[1] = 0; mWalkRows[2] = 0; mWalkRows[3] = 0;

    // Create and add components
    mAnimationComponent = AddComponent<AnimationComponent>();
    mSpriteComponent = AddComponent<SpriteComponent>(200); // Higher update order for rendering
    mMovementComponent = AddComponent<MovementComponent>();

    // Configure animation component with default values
    mAnimationComponent->SetFrameCount(mIdleFrames);
    mAnimationComponent->SetAnimSpeed(mAnimSpeed);
}

NPC::~NPC()
{
}

void NPC::LoadSpriteSheetFromTSX(const std::string& tsxPath)
{
    TilesetInfo tileset;
    if (!TiledParser::ParseTSX(tsxPath, tileset))
    {
        SDL_Log("Failed to load TSX file: %s", tsxPath.c_str());
        return;
    }

    // Load the sprite sheet and configure
    if (mSpriteComponent)
    {
        mSpriteComponent->LoadSpriteSheet(tileset.imagePath);
    }
    SetSpriteConfiguration(tileset.tileWidth, tileset.tileHeight, tileset.columns, tileset.columns, 8.0f);
}

void NPC::SetSpriteConfiguration(int width, int height, int idleFrames, int walkFrames, float animSpeed)
{
    mSpriteWidth = width;
    mSpriteHeight = height;
    mIdleFrames = idleFrames;
    mWalkFrames = walkFrames;
    mAnimSpeed = animSpeed;

    if (mSpriteComponent)
    {
        mSpriteComponent->SetSpriteSize(width, height);
        mSpriteComponent->SetRenderSize(64.0f);
    }

    if (mAnimationComponent)
    {
        mAnimationComponent->SetFrameCount(idleFrames);
        mAnimationComponent->SetAnimSpeed(animSpeed);
    }
}

void NPC::SetIdleRows(int down, int left, int right, int up)
{
    mIdleRows[0] = down;
    mIdleRows[1] = left;
    mIdleRows[2] = right;
    mIdleRows[3] = up;
}

void NPC::SetWalkRows(int down, int left, int right, int up)
{
    mWalkRows[0] = down;
    mWalkRows[1] = left;
    mWalkRows[2] = right;
    mWalkRows[3] = up;
}

int NPC::GetDirectionRow(const Vector2& velocity) const
{
    // Determine facing direction based on velocity
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
