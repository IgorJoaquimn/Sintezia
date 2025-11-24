#include "MovementComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Game/Game.hpp"
#include "../Map/TileMap.hpp"
#include <algorithm>

MovementComponent::MovementComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mVelocity(Vector2::Zero)
    , mImpulseVelocity(Vector2::Zero)
    , mImpulseDecay(0.92f)  // Decay 8% per frame (smooth deceleration)
    , mUseBounds(true)
    , mMinX(32.0f)
    , mMinY(32.0f)
    , mMaxX(Game::WINDOW_WIDTH - 32.0f)
    , mMaxY(Game::WINDOW_HEIGHT - 32.0f)
{
}

void MovementComponent::Update(float deltaTime)
{
    // Calculate new position based on velocity + impulse velocity
    Vector2 currentPos = mOwner->GetPosition();
    Vector2 totalVelocity = mVelocity + mImpulseVelocity;
    Vector2 newPos = currentPos + totalVelocity * deltaTime;

    // Get tilemap for collision checking
    Game* game = GetGame();
    TileMap* tilemap = game ? game->GetTileMap() : nullptr;
    
    if (tilemap)
    {
        // Check collision at new position (using radius for player size)
        const float collisionRadius = 16.0f; // Half of player sprite width
        
        // Try X movement first
        Vector2 testPosX(newPos.x, currentPos.y);
        if (!tilemap->CheckCollision(testPosX, collisionRadius))
        {
            currentPos.x = newPos.x;
        }
        else
        {
            // Collision on X axis, stop X impulse
            mImpulseVelocity.x = 0.0f;
        }

        // Try Y movement
        Vector2 testPosY(currentPos.x, newPos.y);
        if (!tilemap->CheckCollision(testPosY, collisionRadius))
        {
            currentPos.y = newPos.y;
        }
        else
        {
            // Collision on Y axis, stop Y impulse
            mImpulseVelocity.y = 0.0f;
        }

        newPos = currentPos;
    }
    else
    {
        // No tilemap, just update position
        newPos = currentPos + totalVelocity * deltaTime;
    }
    
    // Clamp to bounds if enabled
    if (mUseBounds)
    {
        newPos.x = std::max(mMinX, std::min(newPos.x, mMaxX));
        newPos.y = std::max(mMinY, std::min(newPos.y, mMaxY));
    }
    
    // Apply decay to impulse velocity (friction/deceleration)
    mImpulseVelocity *= mImpulseDecay;

    // Stop impulse when it's very small
    if (mImpulseVelocity.LengthSq() < 1.0f)
    {
        mImpulseVelocity = Vector2::Zero;
    }

    mOwner->SetPosition(newPos);
}

void MovementComponent::ApplyImpulse(const Vector2& impulse)
{
    mImpulseVelocity += impulse;
}

void MovementComponent::SetBounds(float minX, float minY, float maxX, float maxY)
{
    mMinX = minX;
    mMinY = minY;
    mMaxX = maxX;
    mMaxY = maxY;
}
