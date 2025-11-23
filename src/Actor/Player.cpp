#include "Player.hpp"
#include "../Game/Game.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Component/PlayerInputComponent.hpp"
#include "../Component/MovementComponent.hpp"
#include "../Component/AnimationComponent.hpp"
#include "../Component/SpriteComponent.hpp"

Player::Player(Game* game)
    : Actor(game)
    , mState(PlayerState::Idle)
    , mInputComponent(nullptr)
    , mMovementComponent(nullptr)
    , mAnimationComponent(nullptr)
    , mSpriteComponent(nullptr)
{
    SetPosition(Vector2(640.0f, 360.0f)); // Center of screen
    
    // Create and add components
    mInputComponent = AddComponent<PlayerInputComponent>();
    mMovementComponent = AddComponent<MovementComponent>();
    mAnimationComponent = AddComponent<AnimationComponent>();
    mSpriteComponent = AddComponent<SpriteComponent>(200); // Higher update order for rendering
    
    // Configure animation component
    mAnimationComponent->SetFrameCount(IDLE_FRAMES);
    mAnimationComponent->SetAnimSpeed(ANIM_SPEED);
    
    // Load sprite sheet
    mSpriteComponent->LoadSpriteSheet("assets/third_party/Cute_Fantasy_Free/Player/Player.png");
    mSpriteComponent->SetSpriteSize(SPRITE_WIDTH, SPRITE_HEIGHT);
    mSpriteComponent->SetRenderSize(80.0f);
}

Player::~Player()
{
}

void Player::OnProcessInput(const Uint8* keyState)
{
    // Input component handles this via Actor::ProcessInput
    // Just need to update state based on input component
    if (mInputComponent)
    {
        mState = mInputComponent->IsMoving() ? PlayerState::Walking : PlayerState::Idle;
        
        // Pass velocity to movement component
        if (mMovementComponent)
        {
            mMovementComponent->SetVelocity(mInputComponent->GetVelocity());
        }
        
        // Update animation frame count based on state
        if (mAnimationComponent)
        {
            int frameCount = (mState == PlayerState::Walking) ? WALK_FRAMES : IDLE_FRAMES;
            mAnimationComponent->SetFrameCount(frameCount);
        }
    }
}

void Player::OnUpdate(float deltaTime)
{
    // Components handle the update automatically via Actor::Update
    // This method can be left empty or used for Player-specific logic
}

void Player::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    if (!spriteRenderer || !mSpriteComponent || !mInputComponent || !mAnimationComponent) return;
    
    // Determine sprite row and column based on state and direction
    int row;
    int col = mAnimationComponent->GetCurrentFrame();
    int direction = mInputComponent->GetDirection();
    
    if (mState == PlayerState::Idle)
    {
        // Idle animations are in rows 0-2 (0-indexed, which is rows 1-3 in 1-indexed)
        switch (direction)
        {
            case 0: row = 0; break; // Down - Row 1 (0-indexed)
            case 1: row = 1; break; // Right - Row 2 (0-indexed)
            case 2: row = 2; break; // Up - Row 3 (0-indexed)
            case 3: row = 1; break; // Left - Use Right row and flip
            default: row = 0; break;
        }
    }
    else // Walking
    {
        // Walking animations are in rows 3-5 (0-indexed, which is rows 4-6 in 1-indexed)
        switch (direction)
        {
            case 0: row = 3; break; // Down - Row 4 (0-indexed)
            case 1: row = 4; break; // Right - Row 5 (0-indexed)
            case 2: row = 5; break; // Up - Row 6 (0-indexed)
            case 3: row = 4; break; // Left - Use Right row and flip
            default: row = 3; break;
        }
    }
    
    // Configure sprite component for rendering
    mSpriteComponent->SetCurrentFrame(row, col);
    mSpriteComponent->SetFlipHorizontal(direction == 3); // Flip for left direction
    
    // Draw the sprite
    mSpriteComponent->Draw(spriteRenderer);
}
