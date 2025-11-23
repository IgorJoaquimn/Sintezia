#include "MovementComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Game/Game.hpp"
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
    // Update position based on velocity
    Vector2 pos = mOwner->GetPosition();
    pos += mVelocity * deltaTime;
    
    // Clamp to bounds if enabled
    if (mUseBounds)
    {
        pos.x = std::max(mMinX, std::min(pos.x, mMaxX));
        pos.y = std::max(mMinY, std::min(pos.y, mMaxY));
    }
    
    mOwner->SetPosition(pos);
}

void MovementComponent::SetBounds(float minX, float minY, float maxX, float maxY)
{
    mMinX = minX;
    mMinY = minY;
    mMaxX = maxX;
    mMaxY = maxY;
}
