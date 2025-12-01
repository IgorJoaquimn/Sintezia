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
#include "../Actor/ItemActor.hpp"
#include "../UI/HealthBar.hpp" // adicionado include da HealthBar
#include "../Map/TileMap.hpp"

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
    , mHealthBar(nullptr) // inicializa o ponteiro
    , mHasShownHungerWarning(false)
    , mHasShownThirstWarning(false)
    , mHasShownDamageWarning(false)
{
    SetGridPosition(45, 70); // Start at (45, 70) tiles
    
    // Create and add components
    mInputComponent = AddComponent<PlayerInputComponent>();
    mMovementComponent = AddComponent<MovementComponent>();
    mAnimationComponent = AddComponent<AnimationComponent>();
    mSpriteComponent = AddComponent<SpriteComponent>(200); // Higher update order for rendering
    mHealthComponent = AddComponent<HealthComponent>();
    mAttackComponent = AddComponent<AttackComponent>();
    mHungerComponent = AddComponent<HungerComponent>();
    mThirstComponent = AddComponent<ThirstComponent>();

    // Configure health component
    mHealthComponent->SetMaxHealth(100.0f);
    mHealthComponent->SetCurrentHealth(100.0f);
    mHealthComponent->SetDeathCallback([this]() {
        // Player death - quit game
        mGame->Quit();
    });
    
    mHealthComponent->SetOnDamageCallback([this](float damage) {
        if (!mHasShownDamageWarning) {
            mGame->ShowWarning("Ah nao! Voce esta ferido. Procure uma maneira de se curar.");
            mHasShownDamageWarning = true;
        }
    });

    mHungerComponent->SetStarvationCallback([this]() {
        mHealthComponent->TakeDamage(5.0f);
        if (!mHasShownHungerWarning) {
            mGame->ShowWarning("Ah nao! Voce esta sofrendo dano por fome. Busque comida.");
            mHasShownHungerWarning = true;
        }
    });

    mThirstComponent->SetDehydrationCallback([this]() {
        mHealthComponent->TakeDamage(5.0f);
        if (!mHasShownThirstWarning) {
            mGame->ShowWarning("Ah nao! Voce esta sofrendo dano por sede. Busque agua para tomar.");
            mHasShownThirstWarning = true;
        }
    });

    // Debug: Start with high hunger/thirst to test warning
    // mHungerComponent->SetCurrentHunger(95.0f);
    // mThirstComponent->SetCurrentThirst(95.0f);

    // Create health bar UI attached to the player's HealthComponent
    // Initial position is (0,0) — we'll position it relative to the player each frame in OnDraw
    mHealthBar = std::make_unique<HealthBar>(mHealthComponent, mHungerComponent, mThirstComponent, mGame->GetRectRenderer(), 0.0f, 0.0f, 100.0f, 12.0f);

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
    mInventoryUI->SetOnItemUsed([this](const Item& item) {
        bool consumed = false;
        
        if (item.hungerRestoration > 0.0f && mHungerComponent) {
            // Treat restoration as absolute value
            float amount = item.hungerRestoration;
            mHungerComponent->DecreaseHunger(amount);
            consumed = true;
        }
        
        if (item.thirstRestoration > 0.0f && mThirstComponent) {
            // Treat restoration as absolute value
            float amount = item.thirstRestoration;
            mThirstComponent->DecreaseThirst(amount);
            consumed = true;
        }
        
        if (consumed) {
            mInventory->RemoveItem(item.id, 1);
            SDL_Log("Consumed item: %s", item.name.c_str());
        }
    });
    // Center on screen (assuming 1200x800 resolution from Game.hpp)
    mInventoryUI->CenterOnScreen(1200.0f, 800.0f);
    
    LoadTextures();
}

Player::~Player()
{
}

void Player::LoadTextures()
{
    // Load main sprite sheet
    mSpriteSheet = std::make_shared<Texture>();
    // Try loading from assets
    std::string basePath = "assets/third_party/Ninja Adventure - Asset Pack/Actor/Characters/Boy/";
    if (!mSpriteSheet->Load(basePath + "SpriteSheet.png"))
    {
        // Try fallback path for build dir
        basePath = "../assets/third_party/Ninja Adventure - Asset Pack/Actor/Characters/Boy/";
        mSpriteSheet->Load(basePath + "SpriteSheet.png");
    }

    // Load attack texture
    mAttackTexture = std::make_shared<Texture>();
    mAttackTexture->Load(basePath + "SeparateAnim/Attack.png");

    // Set initial texture
    if (mSpriteSheet)
    {
        mSpriteComponent->SetTexture(mSpriteSheet);
        mSpriteComponent->SetSpriteSize(16, 16); // Tile size from TSX
        mSpriteComponent->SetRenderSize(64.0f);
    }
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
            // Stop movement when inventory is open
            if (mMovementComponent)
            {
                mMovementComponent->SetVelocity(Vector2::Zero);
            }
            mState = PlayerState::Idle;
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

        // Handle Environment Interaction (K key)
        static bool kKeyPressed = false;
        if (keyState[SDL_SCANCODE_K])
        {
            if (!kKeyPressed)
            {
                if (CheckForEnvironmentInteraction())
                {
                    kKeyPressed = true;
                    return; // Skip attack if interacted
                }
                kKeyPressed = true;
            }
        }
        else
        {
            kKeyPressed = false;
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
    // Check for hunger/thirst appearance (threshold > 0)
    if (!mHasShownHungerWarning && mHungerComponent && mHungerComponent->GetCurrentHunger() >= 1.0f)
    {
        mGame->ShowWarning("Voce esta com fome! Nao deixe a barra amarela encher.");
        mHasShownHungerWarning = true;
    }

    if (!mHasShownThirstWarning && mThirstComponent && mThirstComponent->GetCurrentThirst() >= 1.0f)
    {
        mGame->ShowWarning("Voce esta com sede! Nao deixe a barra azul encher.");
        mHasShownThirstWarning = true;
    }

    // Check for nearby items to pickup
    // Radius increased to 150px (approx 3-4 tiles) to make pickup easier
    const float PICKUP_RADIUS = 150.0f;
    const float PICKUP_RADIUS_SQ = PICKUP_RADIUS * PICKUP_RADIUS;
    
    Vector2 myPos = GetPosition();
    
    for (const auto& actor : mGame->GetActors())
    {
        // Check if it's an ItemActor
        ItemActor* item = dynamic_cast<ItemActor*>(actor.get());
        if (item && item->GetState() == ActorState::Active && !item->IsBeingPickedUp())
        {
            // Calculate item center (ItemActor position is Left-Center)
            Vector2 itemBounds = item->GetBounds();
            Vector2 itemCenter = item->GetPosition() + Vector2(itemBounds.x / 2.0f, 0.0f);

            // Check distance
            float distSq = (itemCenter - myPos).LengthSq();
            if (distSq < PICKUP_RADIUS_SQ)
            {
                item->StartPickup(this);
            }
        }
    }

    // Update inventory UI
    if (mInventoryUI)
    {
        mInventoryUI->Update(deltaTime);
    }

        // Update health bar UI
    if (mHealthBar)
    {
        mHealthBar->Update(deltaTime);
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

    // Reset recent damage over time
    if (mHealthComponent)
    {
        mHealthComponent->ResetRecentDamage(deltaTime);
    }
}

void Player::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    if (!spriteRenderer || !mSpriteComponent || !mInputComponent || !mAnimationComponent) return;

    int direction = mInputComponent->GetDirection();
    // Use last direction if idle/attacking to keep facing the right way
    if (!mInputComponent->IsMoving())
    {
        direction = mLastDirection;
    }

    int frame = mAnimationComponent->GetCurrentFrame();
    
    int row = 0;
    int col = 0;
    
    // Map direction (0=Down, 1=Right, 2=Up, 3=Left) to sprite columns
    // Assuming Attack.png follows the same column layout as SpriteSheet.png
    switch (direction)
    {
        case 0: col = 0; break; // Down
        case 1: col = 3; break; // Right
        case 2: col = 1; break; // Up
        case 3: col = 2; break; // Left
    }
    
    if (mState == PlayerState::Attacking && mAttackTexture)
    {
        mSpriteComponent->SetTexture(mAttackTexture);
        // Attack animation: Assuming 1 frame (Row 0) for now
        // If Attack.png has multiple rows for animation, we can use 'frame' here
        row = 0; 
    }
    else
    {
        if (mSpriteSheet)
        {
            mSpriteComponent->SetTexture(mSpriteSheet);
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
    }
    
    mSpriteComponent->SetCurrentFrame(row, col);
    mSpriteComponent->SetFlipHorizontal(false);
    mSpriteComponent->Draw(spriteRenderer);

    // Draw health bar (HUD) fixed at top-left of the screen
    if (mHealthBar)
    {
        // Position fixed in screen coordinates (10px margin from top-left)
        mHealthBar->SetPosition(10.0f, 50.0f); // Moved down slightly to make room for icons
        mHealthBar->SetSize(300, 40); // Increased size
        mHealthBar->Draw(textRenderer, mGame->GetRectRenderer());
    }

    // Inventory UI is now drawn in Game::GenerateOutput to ensure it's on top
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

bool Player::CheckForEnvironmentInteraction()
{
    auto* tileMap = mGame->GetTileMap();
    if (!tileMap) return false;

    Vector2 pos = GetPosition();
    int tileSize = tileMap->GetTileSize();
    
    // Calculate tile coordinates of the player
    int playerCol = static_cast<int>(pos.x / tileSize);
    int playerRow = static_cast<int>(pos.y / tileSize);

    // Define generator layers and their corresponding items
    std::map<std::string, std::string> generatorMap = {
        {"gerador_agua", "Água"},
        {"gerador_fogo", "Fogo"},
        {"gerador_madeira", "Madeira"}
    };

    // Check surrounding tiles (including current)
    // 3x3 grid centered on player
    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            int col = playerCol + x;
            int row = playerRow + y;

            // Check bounds
            if (col < 0 || col >= tileMap->GetWidth() || row < 0 || row >= tileMap->GetHeight())
                continue;

            auto* mapData = tileMap->GetMapData();
            if (!mapData) continue;

            // 1. Check for "bloco_deitado_machado" interaction
            for (auto& layer : mapData->layers)
            {
                if (layer.name == "bloco_deitado_machado")
                {
                    int index = row * layer.width + col;
                    if (index >= 0 && index < static_cast<int>(layer.data.size()) && layer.data[index] != 0)
                    {
                        // Found a log!
                        
                        // Check if player has Machado (ID 102)
                        if (mInventory && mInventory->HasItem(102, 1))
                        {
                            // Remove the log (set GID to 0)
                            layer.data[index] = 0;

                            // Remove collision and visual representation from other layers
                            for (auto& otherLayer : mapData->layers)
                            {
                                if (otherLayer.name == "collision" || 
                                    otherLayer.name == "structures" || 
                                    otherLayer.name == "structures 2")
                                {
                                    if (index >= 0 && index < static_cast<int>(otherLayer.data.size()))
                                    {
                                        otherLayer.data[index] = 0;
                                    }
                                }
                            }
                            
                            // Spawn "Madeira" (ID 100)
                            const Item* woodItem = mGame->GetCrafting()->FindItemById(100);
                            if (woodItem)
                            {
                                auto itemActor = std::make_unique<ItemActor>(mGame, *woodItem);
                                Vector2 spawnPos(col * tileSize + tileSize / 2.0f, row * tileSize + tileSize / 2.0f);
                                itemActor->SetPosition(spawnPos);
                                itemActor->SetAutoPickup(this);
                                mGame->AddActor(std::move(itemActor));
                            }
                            
                            return true;
                        }
                        else
                        {
                            // Show warning
                            mGame->ShowWarning("Tente bater nisso apos craftar o machado");
                            return true; // Interaction handled (prevent attack)
                        }
                    }
                }
            }

            // 2. Check each generator layer
            for (const auto& pair : generatorMap)
            {
                std::string layerName = pair.first;
                std::string itemName = pair.second;

                for (const auto& layer : mapData->layers)
                {
                    if (layer.name == layerName)
                    {
                        int index = row * layer.width + col;
                        if (index >= 0 && index < static_cast<int>(layer.data.size()) && layer.data[index] != 0)
                        {
                            // Found a generator tile!
                            
                            // Check cooldown (e.g. 0.5 seconds)
                            if (tileMap->CanHarvestBlock(col, row, mGame->GetGameTime(), 0.5f))
                            {
                                // Find item definition
                                const Item* itemDef = nullptr;
                                if (mGame->GetCrafting())
                                {
                                    for (const auto& item : mGame->GetCrafting()->GetAllItems())
                                    {
                                        if (item.name == itemName)
                                        {
                                            itemDef = &item;
                                            break;
                                        }
                                    }
                                }

                                if (itemDef)
                                {
                                    // Spawn item
                                    auto itemActor = std::make_unique<ItemActor>(mGame, *itemDef);
                                    Vector2 spawnPos(col * tileSize + tileSize / 2.0f, row * tileSize + tileSize / 2.0f);
                                    itemActor->SetPosition(spawnPos);
                                    
                                    // Make it go to user
                                    itemActor->SetAutoPickup(this);
                                    
                                    mGame->AddActor(std::move(itemActor));
                                    
                                    // Set cooldown
                                    tileMap->SetBlockHarvestTime(col, row, mGame->GetGameTime());
                                    
                                    // Only harvest one block per press
                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}
