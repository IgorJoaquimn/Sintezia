#include "Player.hpp"
#include "../Game/Game.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <algorithm>
#include <cmath>

Player::Player(Game* game)
    : Actor(game)
    , mVelocity(Vector2::Zero)
    , mState(PlayerState::Idle)
    , mSpeed(200.0f)
    , mDirection(0)
    , mSpriteSheet(std::make_unique<Texture>())
    , mAnimTime(0.0f)
    , mAnimFrame(0)
{
    SetPosition(Vector2(640.0f, 360.0f)); // Center of screen
    
    // Load Cute Fantasy Player sprite sheet (contains all animations)
    if (!mSpriteSheet->Load("assets/third_party/Cute_Fantasy_Free/Player/Player.png"))
    {
        SDL_Log("Failed to load player sprite sheet!");
    }
}

Player::~Player()
{
}

void Player::ProcessInput(const Uint8* keyState)
{
    Vector2 velocity = Vector2::Zero;
    
    // WASD movement
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
    
    // Normalize diagonal movement
    if (velocity.LengthSq() > 0.0f)
    {
        velocity.Normalize();
        velocity *= mSpeed;
        mState = PlayerState::Walking;
    }
    else
    {
        mState = PlayerState::Idle;
    }
    
    mVelocity = velocity;
}

void Player::OnUpdate(float deltaTime)
{
    // Update position
    Vector2 pos = GetPosition();
    pos += mVelocity * deltaTime;
    
    // Clamp to screen bounds (consider tile size)
    const float margin = 32.0f;
    pos.x = std::max(margin, std::min(pos.x, static_cast<float>(Game::WINDOW_WIDTH - margin)));
    pos.y = std::max(margin, std::min(pos.y, static_cast<float>(Game::WINDOW_HEIGHT - margin)));
    
    SetPosition(pos);
    
    // Update animation
    mAnimTime += deltaTime;
    float frameTime = 1.0f / ANIM_SPEED;
    
    if (mAnimTime >= frameTime)
    {
        mAnimTime -= frameTime;
        
        int maxFrames = (mState == PlayerState::Walking) ? WALK_FRAMES : IDLE_FRAMES;
        mAnimFrame = (mAnimFrame + 1) % maxFrames;
    }
}

void Player::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    if (!spriteRenderer) return;
    
    if (!mSpriteSheet) return;
    
    // Player.png sprite sheet layout: 192x320 (6 cols × 10 rows of 32x32)
    // Rows 1-3: Idle animations (Row 1=Down, Row 2=Right, Row 3=Up)
    // Rows 4-6: Walking animations (Row 4=Down, Row 5=Right, Row 6=Up)
    // Rows 7-9: Attack animations (Row 7=Down, Row 8=Right, Row 9=Up)
    // Row 10: Death animation (facing right, falls to the ground)
    // Note: Left-facing sprites are created by flipping Right-facing sprites
    
    int row;
    int col = mAnimFrame;
    
    if (mState == PlayerState::Idle)
    {
        // Idle animations are in rows 0-2 (0-indexed, which is rows 1-3 in 1-indexed)
        switch (mDirection)
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
        switch (mDirection)
        {
            case 0: row = 3; break; // Down - Row 4 (0-indexed)
            case 1: row = 4; break; // Right - Row 5 (0-indexed)
            case 2: row = 5; break; // Up - Row 6 (0-indexed)
            case 3: row = 4; break; // Left - Use Right row and flip
            default: row = 3; break;
        }
    }
    
    // Calculate normalized texture coordinates
    // Sheet is 192x320 pixels, tiles are 32x32
    float sheetWidth = 192.0f;
    float sheetHeight = 320.0f;
    
    Vector2 srcPos((col * SPRITE_WIDTH) / sheetWidth, (row * SPRITE_HEIGHT) / sheetHeight);
    Vector2 srcSize(SPRITE_WIDTH / sheetWidth, SPRITE_HEIGHT / sheetHeight);
    
    // Draw position (centered on player position)
    // Scale 32px tiles to 80px for display (2.5× to match 40px base tile scale)
    Vector2 pos = GetPosition();
    const float renderSize = 80.0f;  // 32px × 2.5 = 80px
    Vector2 drawPos(pos.x - renderSize/2, pos.y - renderSize/2);
    Vector2 drawSize(renderSize, renderSize);
    
    // Flip horizontally for left-facing sprites
    bool flipHorizontal = (mDirection == 3); // Left direction
    
    // Draw using sprite sheet with source rectangle
    spriteRenderer->DrawSprite(mSpriteSheet.get(), drawPos, drawSize, srcPos, srcSize, 0.0f, 
                              Vector3(1.0f, 1.0f, 1.0f), flipHorizontal, false);
}
