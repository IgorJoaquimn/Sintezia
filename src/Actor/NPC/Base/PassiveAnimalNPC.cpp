#include "PassiveAnimalNPC.hpp"
#include "../../../Game/Game.hpp"
#include "../../../Component/SpriteComponent.hpp"
#include "../../../Component/AnimationComponent.hpp"
#include "../../../Component/MovementComponent.hpp"
#include "../../../Actor/ItemActor.hpp"
#include "../../../Crafting/Crafting.hpp"
#include "../../../Crafting/Item.hpp"
#include <cmath>

PassiveAnimalNPC::PassiveAnimalNPC(Game* game, const std::string& spriteSheetPath)
    : NPC(game)
    , mMoveSpeed(40.0f)
    , mWanderRadius(100.0f)
    , mAnimSpeedRun(0.2f)
    , mHomePosition(Vector2::Zero)
    , mWanderTimer(0.0f)
    , mIsMoving(false)
    , mCurrentFrame(0)
{
    LoadSpriteSheetFromTSX(spriteSheetPath);
    
    // Configure animation component for 2 frames (Idle + Walk)
    if (mAnimationComponent)
    {
        mAnimationComponent->SetFrameCount(2);
    }
}

PassiveAnimalNPC::~PassiveAnimalNPC()
{
}

void PassiveAnimalNPC::OnDeath()
{
    // Drop Meat (ID 103)
    if (mGame && mGame->GetCrafting())
    {
        const Item* meat = mGame->GetCrafting()->FindItemById(103);
        if (meat)
        {
            auto itemActor = std::make_unique<ItemActor>(mGame, *meat);
            itemActor->SetPosition(GetPosition());
            mGame->AddActor(std::move(itemActor));
        }
    }
    
    NPC::OnDeath();
}

void PassiveAnimalNPC::OnUpdate(float deltaTime)
{
    // Initialize home position on first update if not set
    if (Math::NearlyEqual(mHomePosition, Vector2::Zero))
    {
        mHomePosition = GetPosition();
    }

    // Wander logic
    mWanderTimer -= deltaTime;
    if (mWanderTimer <= 0.0f)
    {
        mIsMoving = !mIsMoving;
        mWanderTimer = 2.0f + (rand() % 4); // Change state every 2-5 seconds
        
        if (mIsMoving)
        {
            // Pick a random point around home
            float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
            float dist = (float)(rand() % (int)mWanderRadius);
            mWanderTarget = mHomePosition + Vector2(cos(angle) * dist, sin(angle) * dist);
            
            Vector2 dir = mWanderTarget - GetPosition();
            if (dir.Length() > 0) dir.Normalize();
            if (mMovementComponent) mMovementComponent->SetVelocity(dir * mMoveSpeed);
        }
        else
        {
            if (mMovementComponent) mMovementComponent->SetVelocity(Vector2::Zero);
        }
    }

    // Animation logic
    if (mAnimationComponent && mMovementComponent)
    {
        Vector2 velocity = mMovementComponent->GetVelocity();
        if (velocity.LengthSq() > 0.1f)
        {
            // Moving: Cycle frames
            mAnimationComponent->SetAnimSpeed(1.0f / mAnimSpeedRun);
            mAnimationComponent->Update(deltaTime);
            mCurrentFrame = mAnimationComponent->GetCurrentFrame();
        }
        else
        {
            // Idle: Frame 0
            mCurrentFrame = 0;
        }
    }
    
    if (mSpriteComponent)
    {
        mSpriteComponent->SetCurrentFrame(0, mCurrentFrame);
    }

    NPC::OnUpdate(deltaTime);
}
