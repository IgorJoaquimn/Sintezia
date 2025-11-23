#pragma once
#include "Actor.hpp"
#include "../MathUtils.h"
#include <SDL.h>

// Forward declarations
class PlayerInputComponent;
class MovementComponent;
class AnimationComponent;
class SpriteComponent;

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
    
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime);
    void OnDraw(class TextRenderer* textRenderer) override;
    
    // State
    PlayerState GetState() const { return mState; }
    
private:
    // Components
    PlayerInputComponent* mInputComponent;
    MovementComponent* mMovementComponent;
    AnimationComponent* mAnimationComponent;
    SpriteComponent* mSpriteComponent;
    
    PlayerState mState;
    
    // Sprite configuration for Player.tsx (32x32 tiles from Player.png)
    // Player.png is 192x320, which is 6 columns × 10 rows of 32x32 tiles
    static constexpr int SPRITE_WIDTH = 32;   // Each tile is 32 pixels wide
    static constexpr int SPRITE_HEIGHT = 32;  // Each tile is 32 pixels tall
    static constexpr int IDLE_FRAMES = 6;     // 6 frames for idle animation
    static constexpr int WALK_FRAMES = 6;     // 6 frames for walk animation
    static constexpr float ANIM_SPEED = 8.0f; // Frames per second
};
