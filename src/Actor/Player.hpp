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
class HealthComponent;
class HungerComponent;
class ThirstComponent;
class AttackComponent;
class Texture;
class Inventory;
class InventoryUI;
class HealthBar; // adicionado forward-declaration para HealthBar

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
    
    // Inventory access
    Inventory* GetInventory() { return mInventory.get(); }
    const Inventory* GetInventory() const { return mInventory.get(); }
    InventoryUI* GetInventoryUI() { return mInventoryUI.get(); }
    
    // Item interaction
    bool PickupItem(const class Item& item, int quantity = 1);
    bool UseItem(int itemId);
    
    // Movement control
    void StopMovement();

private:
    void LoadTextures();

    // Components
    PlayerInputComponent* mInputComponent;
    MovementComponent* mMovementComponent;
    AnimationComponent* mAnimationComponent;
    SpriteComponent* mSpriteComponent;
    HealthComponent* mHealthComponent;
    HungerComponent* mHungerComponent;
    ThirstComponent* mThirstComponent;
    AttackComponent* mAttackComponent;

    PlayerState mState;
    float mAttackTimer;
    int mLastDirection;

    // Textures
    std::shared_ptr<Texture> mSpriteSheet;
    std::shared_ptr<Texture> mAttackTexture;

    // Inventory system
    std::unique_ptr<Inventory> mInventory;
    std::unique_ptr<InventoryUI> mInventoryUI;

    // Health bar UI
    std::unique_ptr<HealthBar> mHealthBar; // adicionado para desenhar a barra de vida

    // Warning flags
    bool mHasShownHungerWarning;
    bool mHasShownThirstWarning;
    bool mHasShownDamageWarning;

    // Animation constants
    static constexpr float ANIM_SPEED = 8.0f; // Frames per second
    static constexpr float ATTACK_DURATION = 0.3f; // Seconds
};
