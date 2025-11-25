#include "CatNPC.hpp"
#include "../../../Game/Game.hpp"
#include "../../../Actor/Player.hpp"
#include "../../../Component/SpriteComponent.hpp"
#include "../../../Component/AnimationComponent.hpp"
#include "../../../Component/MovementComponent.hpp"

CatNPC::CatNPC(Game* game)
    : DialogNPC(game)
    , mCurrentFrame(0)
    , mHomePosition(600.0f, 300.0f)
    , mIsFleeing(false)
    , mFleeTimer(0.0f)
{
    // Set position (adjust as needed)
    SetPosition(mHomePosition);

    // Configure sprite to use Cat sprite sheet from tsx
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Cat.tsx");

    // Set greeting
    SetGreeting("Meow! *purrs softly* I'm a friendly cat. Want to chat?");

    // Add dialog options
    AddDialogOption(
        "Pet the cat",
        "Purrrr... *nuzzles your hand* That feels nice! You're very kind."
    );

    AddDialogOption(
        "Ask about the island",
        "Meow! I've explored every corner of this island. There are many secrets hidden here. "
        "The old ruins to the east hold ancient knowledge, if you're brave enough to explore them."
    );

    AddDialogOption(
        "Ask for help",
        "Meow meow! I may be small, but I know many things. If you bring me fish, "
        "I might share some of my treasures with you!"
    );

    // Add a simple trade offer: Fish for a special item
    // Assuming fish item exists or using placeholder IDs
    TradeOffer fishTrade("Cat's Gift: Trade for Fish", 7, 1); // Gives item #7 (placeholder)
    fishTrade.AddRequirement(4, 2); // Requires 2 of item #4 (assuming fish or similar)
    AddTradeOffer(fishTrade);
}

CatNPC::~CatNPC()
{
}

void CatNPC::OnUpdate(float deltaTime)
{
    // Don't flee if currently interacting with player
    if (IsInteracting())
    {
        if (mMovementComponent)
        {
            mMovementComponent->SetVelocity(Vector2::Zero);
        }
        mIsFleeing = false;
        mFleeTimer = 0.0f;
        
        // Call parent update for dialog handling
        DialogNPC::OnUpdate(deltaTime);
        return;
    }
    
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
}
