#pragma once
#include "Actor.hpp"
#include "../Math.h"
#include <SDL.h>

enum class PlayerState
{
    Idle,
    Walking
};

class Player : public Actor
{
public:
    Player(class Game* game);
    
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
    
    // Direction for sprite (0 = down, 1 = right, 2 = up, 3 = left)
    int mDirection;
};
