#include "Player.hpp"
#include "../Game/Game.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Component/PlayerInputComponent.hpp"
#include "../Component/MovementComponent.hpp"
#include "../Component/AnimationComponent.hpp"
#include "../Component/SpriteComponent.hpp"
#include "../Component/HealthComponent.hpp"
#include "../Component/AttackComponent.hpp"
#include "../Core/Texture/Texture.hpp"

Player::Player(Game* game)
    : Actor(game)
    , mState(PlayerState::Idle)
    , mInputComponent(nullptr)
    , mMovementComponent(nullptr)
    , mAnimationComponent(nullptr)
    , mSpriteComponent(nullptr)
    , mHealthComponent(nullptr)
    , mAttackComponent(nullptr)
    , mAttackTimer(0.0f)
    , mLastDirection(0)
{
    SetPosition(Vector2(640.0f, 360.0f)); // Center of screen
    
    // Create and add components
    mInputComponent = AddComponent<PlayerInputComponent>();
    mMovementComponent = AddComponent<MovementComponent>();
    mAnimationComponent = AddComponent<AnimationComponent>();
    mSpriteComponent = AddComponent<SpriteComponent>(200); // Higher update order for rendering
    mHealthComponent = AddComponent<HealthComponent>();
    mAttackComponent = AddComponent<AttackComponent>();

    // Configure health component
    mHealthComponent->SetMaxHealth(100.0f);
    mHealthComponent->SetCurrentHealth(100.0f);
    mHealthComponent->SetDeathCallback([this]() {
        // Player death - quit game
        mGame->Quit();
    });

    // Configure attack component
    AttackConfig attackConfig;
    attackConfig.damage = 20.0f;
    attackConfig.cooldown = 0.5f;
    attackConfig.range = 100.0f;
    attackConfig.knockback = 280.0f;
    attackConfig.attackDuration = ATTACK_DURATION;
    // Player uses individual textures, not sprite sheet rows, so these won't be used
    mAttackComponent->SetAttackConfig(attackConfig);

    // Set attack callbacks
    mAttackComponent->SetAttackStartCallback([this](int direction) {
        mState = PlayerState::Attacking;
        mAttackTimer = ATTACK_DURATION;
        if (mMovementComponent) mMovementComponent->SetVelocity(Vector2::Zero);
    });

    mAttackComponent->SetAttackEndCallback([this]() {
        if (mState == PlayerState::Attacking) {
            mState = PlayerState::Idle;
        }
    });

    // Configure animation component
    mAnimationComponent->SetFrameCount(1); // Default to 1 frame, updated in OnProcessInput
    mAnimationComponent->SetAnimSpeed(ANIM_SPEED);
    
    // Load all textures
    LoadTextures();
    
    // Set initial texture
    mSpriteComponent->SetTexture(mTextures["idle_front"]);
    mSpriteComponent->SetSpriteSize(0, 0); // Use full texture size
    mSpriteComponent->SetRenderSize(120.0f); // Adjusted size for high-res sprites
}

Player::~Player()
{
}

void Player::LoadTextures()
{
    auto load = [&](const std::string& key, const std::string& file) {
        auto tex = std::make_shared<Texture>();
        std::string subPath = "third_party/Cute_Fantasy_Free/Player/" + file;
        
        // List of paths to try
        std::vector<std::string> paths = {
            "assets/" + subPath,           // Standard path (deployed)
            "../assets/" + subPath,        // From build directory (development)
            "../../assets/" + subPath      // Deeper build directory
        };

        for (const auto& path : paths) {
            if (tex->Load(path)) {
                mTextures[key] = tex;
                return;
            }
        }

        SDL_Log("Failed to load texture: %s (tried multiple paths)", subPath.c_str());
    };

    // Idle
    load("idle_front", "idle.png");
    load("idle_back", "idle-back.png");
    load("idle_left", "idle-left.png");
    load("idle_right", "idle-right.png");

        // Walk
    load("walk_back_1", "run-back-1.png");
    load("walk_back_2", "run-back-2.png");
    load("walk_right_1", "run-right-1.png");
    load("walk_right_2", "run-right-2.png");
    load("walk_left_1", "run-left-1.png");
    load("walk_left_2", "run-left-2.png");
    load("walk_down_1", "run-1.png");
    load("walk_down_2", "run-2.png");

    // Jump
    load("jump_back", "character-12.png");
    load("jump_left", "jump-left.png");
    load("jump_right", "jump-right.png");

    // Crouch (Abaixar)
    load("crouch_back", "down-back.png");
    load("crouch_left", "down-left.png");  
    load("crouch_right", "down-right.png");
    load("crouch_front", "down.png");

    // Attack
    load("attack_left", "character-4.png");
    load("attack_right", "character-5.png");
}

void Player::OnProcessInput(const Uint8* keyState)
{
    // Input component handles this via Actor::ProcessInput
    // Just need to update state based on input component
    if (mInputComponent)
    {
        // Update last direction when moving
        if (mInputComponent->IsMoving())
        {
            mLastDirection = mInputComponent->GetDirection();
        }

        // Handle Attack State (Priority)
        if (mState == PlayerState::Attacking)
        {
            // Attack is handled by AttackComponent
            return;
        }

        if (mInputComponent->IsAttacking() && mAttackComponent && mAttackComponent->CanAttack())
        {
            // Start attack in the current facing direction
            mAttackComponent->StartAttack(mLastDirection);
            return;
        }

        // Handle other states
        if (mInputComponent->IsCrouching())
        {
            mState = PlayerState::Crouching;
        }
        else if (mInputComponent->IsJumping())
        {
            mState = PlayerState::Jumping;
        }
        else if (mInputComponent->IsMoving())
        {
            mState = PlayerState::Walking;
        }
        else
        {
            mState = PlayerState::Idle;
        }
        
        // Pass velocity to movement component
        if (mMovementComponent)
        {
            mMovementComponent->SetVelocity(mInputComponent->GetVelocity());
        }
        
        // Update animation frame count based on state
        if (mAnimationComponent)
        {
            // For walking, we have 2 frames (alternating textures)
            // For others, we usually have 1 frame
            int frameCount = (mState == PlayerState::Walking) ? 2 : 1;
            mAnimationComponent->SetFrameCount(frameCount);
        }
    }
}

void Player::OnUpdate(float deltaTime)
{
    // Handle attack timer
    if (mState == PlayerState::Attacking)
    {
        mAttackTimer -= deltaTime;
        if (mAttackTimer <= 0.0f)
        {
            mState = PlayerState::Idle;
        }
    }
}

std::shared_ptr<Texture> Player::GetTextureForState(PlayerState state, int direction, int frame)
{
    // Direction: 0=Down, 1=Right, 2=Up, 3=Left
    
    if (state == PlayerState::Attacking)
    {
        // Attack only has Left and Right
        if (direction == 3 || direction == 0) // Left or Down -> Attack Left
            return mTextures["attack_left"];
        else // Right or Up -> Attack Right
            return mTextures["attack_right"];
    }
    else if (state == PlayerState::Jumping)
    {
        switch (direction)
        {
            case 0: return mTextures["idle_front"]; // No jump front, use idle
            case 1: return mTextures["jump_right"];
            case 2: return mTextures["jump_back"];
            case 3: return mTextures["jump_left"];
        }
    }
    else if (state == PlayerState::Crouching)
    {
        switch (direction)
        {
            case 0: return mTextures["crouch_front"];
            case 1: return mTextures["crouch_right"];
            case 2: return mTextures["crouch_back"];
            case 3: return mTextures["crouch_left"];
        }
    }
    else if (state == PlayerState::Idle)
    {
        switch (direction)
        {
            case 0: return mTextures["idle_front"];
            case 1: return mTextures["idle_right"];
            case 2: return mTextures["idle_back"];
            case 3: return mTextures["idle_left"];
        }
    }
    else // Walking
    {
        int animFrame = frame % 2; // 0 or 1
        
        switch (direction)
        {
            case 0: // Down
                // Alternate between Run Front 1 and Run Front 2
                return (animFrame == 0) ? mTextures["walk_down_1"] : mTextures["walk_down_2"];
            case 1: // Right
                return (animFrame == 0) ? mTextures["walk_right_1"] : mTextures["walk_right_2"];
            case 2: // Up
                return (animFrame == 0) ? mTextures["walk_back_1"] : mTextures["walk_back_2"];
            case 3: // Left - Use Right textures and flip
                return (animFrame == 0) ? mTextures["walk_left_1"] : mTextures["walk_left_2"];
        }
    }
    
    return mTextures["idle_front"];
}



void Player::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    if (!spriteRenderer || !mSpriteComponent || !mInputComponent || !mAnimationComponent) return;
    
    int direction = mInputComponent->GetDirection();
    int frame = mAnimationComponent->GetCurrentFrame();
    
    // Get the correct texture
    auto texture = GetTextureForState(mState, direction, frame);
    
    // Fallback to idle_front if texture is missing (e.g. failed to load)
    if (!texture)
    {
        texture = mTextures["idle_front"];
    }

    if (texture)
    {
        mSpriteComponent->SetTexture(texture);
        // Reset sprite size to 0 to use full texture size
        mSpriteComponent->SetSpriteSize(0, 0);
        mSpriteComponent->SetCurrentFrame(0, 0);
        
        // Flip horizontal only for Left Walking
        // Idle Left has its own texture (character-10), so no flip needed for Idle Left
        // Walk Left uses Walk Right textures, so flip needed
        // Attack Left has its own texture
        // Jump Left has its own texture
        // Crouch Left uses Down Left (character-7) which is its own texture
        // bool flip = (mState == PlayerState::Walking && direction == 3);
        // mSpriteComponent->SetFlipHorizontal(flip);
        
        mSpriteComponent->Draw(spriteRenderer);
    }
}
