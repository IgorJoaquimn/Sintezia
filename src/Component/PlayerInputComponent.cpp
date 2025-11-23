#include "PlayerInputComponent.hpp"
#include "../Actor/Actor.hpp"

PlayerInputComponent::PlayerInputComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mVelocity(Vector2::Zero)
    , mDirection(0)
    , mIsMoving(false)
    , mIsAttacking(false)
    , mIsJumping(false)
    , mIsCrouching(false)
    , mSpeed(DEFAULT_SPEED)
{
}

void PlayerInputComponent::ProcessInput(const uint8_t* keyState)
{
    Vector2 velocity = Vector2::Zero;
    
    // Reset actions
    mIsAttacking = false;
    mIsJumping = false;
    mIsCrouching = false;

    // Actions
    if (keyState[SDL_SCANCODE_SPACE])
    {
        mIsJumping = true;
    }
    if (keyState[SDL_SCANCODE_LCTRL] || keyState[SDL_SCANCODE_C])
    {
        mIsCrouching = true;
    }
    if (keyState[SDL_SCANCODE_Z] || keyState[SDL_SCANCODE_K])
    {
        mIsAttacking = true;
    }
    
    // WASD movement (disable movement if crouching or attacking)
    if (!mIsCrouching && !mIsAttacking)
    {
        if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
        {
            velocity.y -= 1.0f;
            mDirection = 2; // Up
        }
        if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
        {
            velocity.y += 1.0f;
            mDirection = 0; // Down
        }
        if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT])
        {
            velocity.x -= 1.0f;
            mDirection = 3; // Left
        }
        if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT])
        {
            velocity.x += 1.0f;
            mDirection = 1; // Right
        }
    }
    
    // Normalize diagonal movement
    if (velocity.LengthSq() > 0.0f)
    {
        velocity.Normalize();
        velocity *= mSpeed;
        mIsMoving = true;
    }
    else
    {
        mIsMoving = false;
    }
    
    mVelocity = velocity;
}
