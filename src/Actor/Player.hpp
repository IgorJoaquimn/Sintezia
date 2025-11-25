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
class AttackComponent;
class Texture;
class Inventory;
class InventoryUI;

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
    
private:
    void LoadTextures();
    std::shared_ptr<Texture> GetTextureForState(PlayerState state, int direction, int frame);

    // Components
    PlayerInputComponent* mInputComponent;
    MovementComponent* mMovementComponent;
    AnimationComponent* mAnimationComponent;
    SpriteComponent* mSpriteComponent;
    HealthComponent* mHealthComponent;
    AttackComponent* mAttackComponent;

    PlayerState mState;
    float mAttackTimer;
    int mLastDirection;

    // Inventory system
    std::unique_ptr<Inventory> mInventory;
    std::unique_ptr<InventoryUI> mInventoryUI;

    // Textures
    std::map<std::string, std::shared_ptr<Texture>> mTextures;

    // Animation constants
    static constexpr float ANIM_SPEED = 8.0f; // Frames per second
    static constexpr float ATTACK_DURATION = 0.3f; // Seconds
};
