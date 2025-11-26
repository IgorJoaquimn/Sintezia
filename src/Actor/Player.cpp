#include "Player.hpp"
#include "../Game/Game.hpp"
#include "../Game/Inventory.hpp"
#include "../UI/InventoryUI.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Component/PlayerInputComponent.hpp"
#include "../Component/MovementComponent.hpp"
#include "../Component/AnimationComponent.hpp"
#include "../Component/SpriteComponent.hpp"
#include "../Component/HealthComponent.hpp"
#include "../Component/AttackComponent.hpp"
#include "../Core/Texture/Texture.hpp"
#include "../Map/TiledParser.hpp"

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
    , mInventory(std::make_unique<Inventory>(20))  // 20 inventory slots
    , mInventoryUI(nullptr)
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
    mAnimationComponent->SetFrameCount(4); // 4 frames for walk animation
    mAnimationComponent->SetAnimSpeed(ANIM_SPEED);
    
    // Create inventory UI
    mInventoryUI = std::make_unique<InventoryUI>(game, mInventory.get());
    mInventoryUI->SetPosition(Vector2(200.0f, 150.0f));
    
    // Load Boy sprite sheet from TSX
    TilesetInfo tileset;
    if (TiledParser::ParseTSX("assets/tiled/tilesets/Boy.tsx", tileset))
    {
        mSpriteComponent->LoadSpriteSheet(tileset.imagePath);
        mSpriteComponent->SetSpriteSize(tileset.tileWidth, tileset.tileHeight);
        mSpriteComponent->SetRenderSize(64.0f);
    }
    else
    {
        SDL_Log("Failed to load Boy.tsx");
    }
}

Player::~Player()
{
}

void Player::OnProcessInput(const Uint8* keyState)
{
    // Handle inventory UI input first (if visible, it consumes input)
    if (mInventoryUI)
    {
        mInventoryUI->HandleInput(keyState);
        
        // If inventory is visible, don't process game input
        if (mInventoryUI->IsVisible())
        {
            return;
        }
    }

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
    // Update inventory UI
    if (mInventoryUI)
    {
        mInventoryUI->Update(deltaTime);
    }

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

void Player::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    if (!spriteRenderer || !mSpriteComponent || !mInputComponent || !mAnimationComponent) return;

    int direction = mInputComponent->GetDirection();
    int frame = mAnimationComponent->GetCurrentFrame();
    
    // Boy.tsx sprite sheet layout (4 columns, 7 rows):
    // Column 0: Down, Column 1: Up, Column 2: Left, Column 3: Right
    // Row 0: Idle frame 0
    // Rows 1-4: Walk animation frames (4 frames)
    // Rows 5-6: Additional frames
    
    int row = 0;
    int col = 0;
    
    // Map direction (0=Down, 1=Right, 2=Up, 3=Left) to sprite columns
    switch (direction)
    {
        case 0: col = 0; break; // Down
        case 1: col = 3; break; // Right
        case 2: col = 1; break; // Up
        case 3: col = 2; break; // Left
    }
    
    // Set row based on state
    if (mState == PlayerState::Idle)
    {
        row = 0; // Idle frame
    }
    else if (mState == PlayerState::Walking)
    {
        row = 1 + (frame % 4); // Walk frames are rows 1-4
    }
    else
    {
        row = 0; // Default to idle for other states
    }
    
    mSpriteComponent->SetCurrentFrame(row, col);
    mSpriteComponent->SetFlipHorizontal(false);
    mSpriteComponent->Draw(spriteRenderer);

    // Draw inventory UI on top of player
    if (mInventoryUI)
    {
        auto* rectRenderer = mGame->GetRectRenderer();
        mInventoryUI->Draw(textRenderer, rectRenderer);
    }
}

bool Player::PickupItem(const Item& item, int quantity)
{
    if (!mInventory)
        return false;
    
    return mInventory->AddItem(item, quantity);
}

bool Player::UseItem(int itemId)
{
    if (!mInventory)
        return false;
    
    // Check if player has the item
    if (!mInventory->HasItem(itemId, 1))
        return false;
    
    // TODO: Implement item usage logic based on item type
    // For now, just remove one from inventory
    return mInventory->RemoveItem(itemId, 1);
}

void Player::StopMovement()
{
    if (mMovementComponent)
    {
        mMovementComponent->SetVelocity(Vector2::Zero);
    }
    mState = PlayerState::Idle;
}
