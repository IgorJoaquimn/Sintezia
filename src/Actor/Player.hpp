#pragma once
#include "Actor.hpp"
#include "../MathUtils.h"
#include <SDL.h>

#include <map>
#include <string>
#include <memory>
#include <vector>

// Forward declarations
class PlayerInputComponent;
class MovementComponent;
class AnimationComponent;
class SpriteComponent;
class Texture;

enum class PlayerState
{
    Idle,
    Walking,
    Jumping,
    Crouching,
    Attacking
};

class Player : public Actor
{
public:
    Player(class Game* game);
    ~Player();
    
    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    void OnDraw(class TextRenderer* textRenderer) override;
    
    // State
    PlayerState GetState() const { return mState; }
    
private:
    void LoadTextures();
    std::shared_ptr<Texture> GetTextureForState(PlayerState state, int direction, int frame);

    // Components
    PlayerInputComponent* mInputComponent;
    MovementComponent* mMovementComponent;
    AnimationComponent* mAnimationComponent;
    SpriteComponent* mSpriteComponent;
    
    PlayerState mState;
    float mAttackTimer;
    
    // Textures
    std::map<std::string, std::shared_ptr<Texture>> mTextures;

    // Animation constants
    static constexpr float ANIM_SPEED = 8.0f; // Frames per second
    static constexpr float ATTACK_DURATION = 0.3f; // Seconds
};
