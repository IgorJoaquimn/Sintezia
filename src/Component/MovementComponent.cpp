#include "MovementComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Game/Game.hpp"
#include "../Map/TileMap.hpp"
#include <algorithm>

MovementComponent::MovementComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mVelocity(Vector2::Zero)
    , mUseBounds(true)
    , mMinX(32.0f)
    , mMinY(32.0f)
    , mMaxX(Game::WINDOW_WIDTH - 32.0f)
    , mMaxY(Game::WINDOW_HEIGHT - 32.0f)
{
}

void MovementComponent::Update(float deltaTime)
{
    // Calculate new position based on velocity
    Vector2 currentPos = mOwner->GetPosition();
    Vector2 newPos = currentPos + mVelocity * deltaTime;
    
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
        
        // Try Y movement
        Vector2 testPosY(currentPos.x, newPos.y);
        if (!tilemap->CheckCollision(testPosY, collisionRadius))
        {
            currentPos.y = newPos.y;
        }
        
        newPos = currentPos;
    }
    else
    {
        // No tilemap, just update position
        newPos = currentPos + mVelocity * deltaTime;
    }
    
    // Clamp to bounds if enabled
    if (mUseBounds)
    {
        newPos.x = std::max(mMinX, std::min(newPos.x, mMaxX));
        newPos.y = std::max(mMinY, std::min(newPos.y, mMaxY));
    }
    
    mOwner->SetPosition(newPos);
}

void MovementComponent::SetBounds(float minX, float minY, float maxX, float maxY)
{
    mMinX = minX;
    mMinY = minY;
    mMaxX = maxX;
    mMaxY = maxY;
}
