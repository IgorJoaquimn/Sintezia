#include "Player.hpp"
#include "../Game/Game.hpp"
#include <algorithm>

Player::Player(Game* game)
    : Actor(game)
    , mVelocity(Vector2::Zero)
    , mState(PlayerState::Idle)
    , mSpeed(200.0f)
    , mDirection(0)
{
    SetPosition(Vector2(640.0f, 360.0f)); // Center of screen
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
    
    // Clamp to screen bounds
    pos.x = std::max(20.0f, std::min(pos.x, static_cast<float>(Game::WINDOW_WIDTH - 20)));
    pos.y = std::max(20.0f, std::min(pos.y, static_cast<float>(Game::WINDOW_HEIGHT - 20)));
    
    SetPosition(pos);
}

void Player::OnDraw(TextRenderer* textRenderer)
{
    // Draw player as emoji
    std::string playerEmoji = "🧑"; // Person emoji
    
    if (textRenderer)
    {
        Vector2 pos = GetPosition();
        textRenderer->RenderText(playerEmoji, pos.x - 24.0f, pos.y - 24.0f, 1.0f);
    }
}
