// ----------------------------------------------------------------
// Component implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "Component.hpp"
#include "../Actor/Actor.hpp"

Component::Component(Actor* owner, int updateOrder)
    : mOwner(owner)
    , mUpdateOrder(updateOrder)
{
    // Component will be added to Actor through smart pointer management
}

Component::~Component()
{
}

void Component::Update(float deltaTime)
{
}

void Component::ProcessInput(const uint8_t* keyState)
{
}

class Game* Component::GetGame() const
{
    return mOwner->GetGame();
}