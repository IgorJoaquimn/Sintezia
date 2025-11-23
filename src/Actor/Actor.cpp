// ----------------------------------------------------------------
// Actor implementation following the asteroids game architecture
// ----------------------------------------------------------------

#include "Actor.hpp"
#include "../Game/Game.hpp"
#include "../Component/Component.hpp"
#include <algorithm>
#include <SDL_stdinc.h>

Actor::Actor(Game* game)
    : mState(ActorState::Active)
    , mPosition(Vector2::Zero)
    , mScale(Vector2(1.0f, 1.0f))
    , mRotation(0.0f)
    , mGame(game)
{
    // Game now manages Actor lifetime through smart pointers
}

Actor::~Actor()
{
    // Smart pointers will automatically clean up components
    mComponents.clear();
}

void Actor::Update(float deltaTime)
{
    if (mState == ActorState::Active)
    {
        // Update all components first
        for (auto& comp : mComponents)
        {
            comp->Update(deltaTime);
        }
        // Then call the actor's own update
        OnUpdate(deltaTime);
    }
}

void Actor::OnUpdate(float deltaTime)
{
    // Base implementation does nothing
}

void Actor::ProcessInput(const uint8_t* keyState)
{
    if (mState == ActorState::Active)
    {
        // Process input for all components first
        for (auto& comp : mComponents)
        {
            comp->ProcessInput(keyState);
        }
        // Then call the actor's own process input
        OnProcessInput(keyState);
    }
}

void Actor::OnProcessInput(const uint8_t* keyState)
{
    // Base implementation does nothing
}

void Actor::OnDraw(class TextRenderer* textRenderer)
{
    // Base implementation does nothing
}

void Actor::AddComponent(std::unique_ptr<Component> c)
{
    mComponents.push_back(std::move(c));
    std::sort(mComponents.begin(), mComponents.end(), 
        [](const std::unique_ptr<Component>& a, const std::unique_ptr<Component>& b) {
            return a->GetUpdateOrder() < b->GetUpdateOrder();
        });
}

Matrix4 Actor::GetModelMatrix() const
{
    Matrix4 scaleMat = Matrix4::CreateScale(mScale.x, mScale.y, 1.0f);
    Matrix4 rotMat = Matrix4::CreateRotationZ(mRotation);
    Matrix4 transMat = Matrix4::CreateTranslation(Vector3(mPosition.x, mPosition.y, 0.0f));
    return scaleMat * rotMat * transMat;
}