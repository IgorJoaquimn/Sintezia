#pragma once
#include "Actor.hpp"
#include "../Math.h"
#include "../Core/Texture/Texture.hpp"
#include <SDL.h>
#include <memory>

enum class PlayerState
{
    Idle,
    Walking
};

class Player : public Actor
{
public:
    Player(class Game* game);
    ~Player();
    
    void ProcessInput(const Uint8* keyState);
    void OnUpdate(float deltaTime);
    void OnDraw(class TextRenderer* textRenderer) override;
    
    // Movement
    void SetVelocity(const Vector2& velocity) { mVelocity = velocity; }
    const Vector2& GetVelocity() const { return mVelocity; }
    
    // State
    PlayerState GetState() const { return mState; }
    
private:
    Vector2 mVelocity;
    PlayerState mState;
    float mSpeed;
    
    // Sprite sheet
    std::unique_ptr<Texture> mSpriteSheet;
    
    // Animation
    float mAnimTime;
    int mAnimFrame;
    
    // Direction for sprite (0 = down, 1 = right, 2 = up, 3 = left)
    int mDirection;
    
    // Sprite configuration for Player.tsx (32x32 tiles from Player.png)
    // Player.png is 192x320, which is 6 columns × 10 rows of 32x32 tiles
    static constexpr int SPRITE_WIDTH = 32;   // Each tile is 32 pixels wide
    static constexpr int SPRITE_HEIGHT = 32;  // Each tile is 32 pixels tall
    static constexpr int IDLE_FRAMES = 6;     // 6 frames for idle animation
    static constexpr int WALK_FRAMES = 6;     // 6 frames for walk animation
    static constexpr float ANIM_SPEED = 8.0f; // Frames per second
};
