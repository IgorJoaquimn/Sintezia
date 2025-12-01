#include "CatNPC.hpp"
#include "../../../Game/Game.hpp"
#include "../../../Actor/Player.hpp"
#include "../../../Component/SpriteComponent.hpp"
#include "../../../Component/AnimationComponent.hpp"
#include "../../../Component/MovementComponent.hpp"

CatNPC::CatNPC(Game* game)
    : NPC(game)
    , mCurrentFrame(0)
    , mHomePosition(1800.0f, 2900.0f)
    , mIsFleeing(false)
    , mFleeTimer(0.0f)
{
    // Set position (adjust as needed)
    SetPosition(mHomePosition);

    // Configure sprite to use Cat sprite sheet from tsx
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Cat.tsx");
}

CatNPC::~CatNPC()
{
}

void CatNPC::OnUpdate(float deltaTime)
{
    // Get player position
    Player* player = mGame->GetPlayer();
    if (player)
    {
        Vector2 playerPos = player->GetPosition();
        Vector2 catPos = GetPosition();
        Vector2 toPlayer = playerPos - catPos;
        float distanceToPlayer = toPlayer.Length();
        
        if (distanceToPlayer < FLEE_RANGE && !mIsFleeing)
        {
            // Start fleeing
            mIsFleeing = true;
            mFleeTimer = FLEE_DURATION;
        }
        
        if (mIsFleeing)
        {
            mFleeTimer -= deltaTime;
            
            if (mFleeTimer > 0.0f)
            {
                // Run away from player
                Vector2 fleeDirection = (catPos - playerPos);
                if (fleeDirection.Length() > 0.0f)
                {
                    fleeDirection.Normalize();
                }
                else
                {
                    // If on top of player, flee in random direction
                    fleeDirection = Vector2(1.0f, 0.0f);
                }
                
                if (mMovementComponent)
                {
                    mMovementComponent->SetVelocity(fleeDirection * FLEE_SPEED);
                }
            }
            else
            {
                // Return to home position
                Vector2 toHome = mHomePosition - catPos;
                float distanceToHome = toHome.Length();
                
                if (distanceToHome > 10.0f)
                {
                    toHome.Normalize();
                    if (mMovementComponent)
                    {
                        mMovementComponent->SetVelocity(toHome * RETURN_SPEED);
                    }
                }
                else
                {
                    // Reached home, stop fleeing
                    mIsFleeing = false;
                    if (mMovementComponent)
                    {
                        mMovementComponent->SetVelocity(Vector2::Zero);
                    }
                }
            }
        }
        else
        {
            // Idle at home position
            if (mMovementComponent)
            {
                mMovementComponent->SetVelocity(Vector2::Zero);
            }
        }
    }
    
    // Update animation based on movement
    if (mAnimationComponent && mMovementComponent)
    {
        Vector2 velocity = mMovementComponent->GetVelocity();
        if (velocity.LengthSq() > 0.0f)
        {
            // Moving - use faster animation when fleeing
            float animSpeed = mIsFleeing ? (ANIM_SPEED / 3.0f) : (ANIM_SPEED / 2.0f);
            mAnimationComponent->SetAnimSpeed(1.0f / animSpeed);
        }
        else
        {
            // Idle - slow animation
            mAnimationComponent->SetAnimSpeed(1.0f / ANIM_SPEED);
        }
        
        mAnimationComponent->Update(deltaTime);
        mCurrentFrame = mAnimationComponent->GetCurrentFrame();
    }
    
    // Update sprite component to show current frame
    // Cat.tsx has 2 sprites in 1 row (columns 0 and 1, row 0)
    if (mSpriteComponent)
    {
        mSpriteComponent->SetCurrentFrame(0, mCurrentFrame); // Row 0, alternating columns
    }

    NPC::OnUpdate(deltaTime);
}
