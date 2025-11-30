#include "AttackComponent.hpp"
#include "HealthComponent.hpp"
#include "MovementComponent.hpp"
#include "../Actor/Actor.hpp"
#include "../Game/Game.hpp"
#include <SDL.h>
#include <cmath>

#include "../Map/TileMap.hpp"      // Necessário para ler o mapa
#include "../Actor/ItemActor.hpp"  // Necessário para spawnar o item
#include "../AudioSystem/AudioSystem.h"  // Necessário para tocar som

AttackComponent::AttackComponent(Actor* owner, int updateOrder)
    : Component(owner, updateOrder)
    , mConfig()
    , mIsAttacking(false)
    , mAttackTimer(0.0f)
    , mCooldownTimer(0.0f)
    , mAttackDirection(0)
    , mAttackStartCallback(nullptr)
    , mAttackEndCallback(nullptr)
{
}

void AttackComponent::Update(float deltaTime)
{
    // Update cooldown timer
    if (mCooldownTimer > 0.0f)
    {
        mCooldownTimer -= deltaTime;
    }

    // Update attack timer
    if (mIsAttacking)
    {
        mAttackTimer -= deltaTime;

        if (mAttackTimer <= 0.0f)
        {
            mIsAttacking = false;

            // Call attack end callback
            if (mAttackEndCallback)
            {
                mAttackEndCallback();
            }
        }
    }
}

void AttackComponent::StartAttack(int direction)
{
    if (!CanAttack()) return;

    mIsAttacking = true;
    mAttackTimer = mConfig.attackDuration;
    mCooldownTimer = mConfig.cooldown;
    mAttackDirection = direction;

    // Call attack start callback (for animation)
    if (mAttackStartCallback)
    {
        mAttackStartCallback(direction);
    }

    // Perform the attack immediately
    PerformAttack();
}

void AttackComponent::PerformAttack()
{
    std::vector<Actor*> targets;
    FindTargetsInRange(targets);

    Vector2 ownerPos = mOwner->GetPosition();
    Vector2 attackDir;

    // Determine attack direction vector (matches PlayerInputComponent: 0=Down, 1=Right, 2=Up, 3=Left)
    switch (mAttackDirection)
    {
        case 0: attackDir = Vector2(0.0f, 1.0f); break;   // Down
        case 1: attackDir = Vector2(1.0f, 0.0f); break;   // Right
        case 2: attackDir = Vector2(0.0f, -1.0f); break;  // Up
        case 3: attackDir = Vector2(-1.0f, 0.0f); break;  // Left
        default: attackDir = Vector2(0.0f, 1.0f); break;
    }

    TryHarvestResource(attackDir);


    // Apply damage and knockback to all targets
    for (Actor* target : targets)
    {
        Vector2 targetPos = target->GetPosition();
        Vector2 toTarget = targetPos - ownerPos;
        float distance = toTarget.Length();
        toTarget.Normalize();

        // Check if target is roughly in the attack direction
        float dot = Vector2::Dot(attackDir, toTarget);

        // SDL_Log("  Target at distance %.1f, dot product: %.2f (attackDir=(%.2f,%.2f), toTarget=(%.2f,%.2f))",
        //         distance, dot, attackDir.x, attackDir.y, toTarget.x, toTarget.y);

        if (dot > 0.0f) // Target is in front (90 degree cone)
        {
            // SDL_Log("  -> HIT! Applying damage");
            ApplyDamageAndKnockback(target, toTarget);
        }
        else
        {
            // SDL_Log("  -> MISS! Target not in attack cone");
        }
    }
}

void AttackComponent::FindTargetsInRange(std::vector<Actor*>& targets)
{
    Game* game = mOwner->GetGame();
    if (!game) return;

    Vector2 ownerPos = mOwner->GetPosition();

    // Get all actors from the game
    const auto& actors = game->GetActors();

    for (const auto& actorPtr : actors)
    {
        Actor* actor = actorPtr.get();

        // Skip self
        if (actor == mOwner) continue;

        // Check if actor has health component (is damageable)
        HealthComponent* healthComp = actor->GetComponent<HealthComponent>();
        if (!healthComp) continue;

        // Check if in range
        Vector2 targetPos = actor->GetPosition();
        float distance = (targetPos - ownerPos).Length();

        if (distance <= mConfig.range)
        {
            targets.push_back(actor);
        }
    }
}

void AttackComponent::TryHarvestResource(const Vector2& attackDir)
{
    Game* game = mOwner->GetGame();
    TileMap* tileMap = game->GetTileMap();
    if (!game || !tileMap) return;

    // 1. Calcular a posição do tile alvo
    int tileSize = tileMap->GetTileSize();
    Vector2 ownerPos = mOwner->GetPosition();

    // O ponto alvo é a posição do jogador + (Direção * tamanho do tile)
    // Isso garante que estamos olhando para o tile imediatamente à frente
    Vector2 targetPos = ownerPos + (attackDir * static_cast<float>(tileSize));

    // Converter posição do mundo para coordenadas da grade (Grid)
    int col = static_cast<int>(targetPos.x / tileSize);
    int row = static_cast<int>(targetPos.y / tileSize);

    // Pegar dados do mapa
    auto mapData = tileMap->GetMapData();
    if (!mapData) return;

    // Verificar limites do mapa
    if (col < 0 || row < 0 || col >= tileMap->GetWidth() || row >= tileMap->GetHeight()) return;

    // 2. Definir o mapeamento (Igual ao ItemGenerator)
    // Layer Name -> Item Name
    std::map<std::string, std::string> resourceLayers = {
        {"gerador_agua", "Água"},
        {"gerador_fogo", "Fogo"},
        {"gerador_madeira", "Madeira"}
    };

    int index = row * tileMap->GetWidth() + col;
    
    // Get current game time in seconds
    float currentTime = game->GetGameTime();
    const float HARVEST_COOLDOWN = 5.0f; // 5 seconds cooldown

    // 3. Iterar pelas camadas para ver se existe um bloco gerador nessa coordenada
    for (const auto& layer : mapData->layers)
    {
        // Verifica se é uma camada de recurso conhecida
        auto it = resourceLayers.find(layer.name);
        if (it != resourceLayers.end())
        {
            // Verifica se existe um tile nessa posição (diferente de 0)
            if (index < layer.data.size() && layer.data[index] != 0)
            {
                // Check if this block's cooldown has expired
                if (!tileMap->CanHarvestBlock(col, row, currentTime, HARVEST_COOLDOWN))
                {
                    // Block is still on cooldown
                    return;
                }
                
                std::string itemName = it->second;

                // 4. Buscar definição do item (Lógica copiada do ItemGenerator)
                const Item* itemDef = nullptr;
                if (game->GetCrafting()) {
                    for (const auto& item : game->GetCrafting()->GetAllItems()) {
                        if (item.name == itemName) {
                            itemDef = &item;
                            break;
                        }
                    }
                }

                if (itemDef)
                {
                    // 5. Spawnar o Item (Dropado no mundo)
                    auto itemActor = std::make_unique<ItemActor>(game, *itemDef);

                    // Centralizar o item no tile alvo
                    Vector2 itemPos(col * tileSize + tileSize / 2.0f, row * tileSize + tileSize / 2.0f);
                    itemActor->SetPosition(itemPos);

                    // 6. Tocar som baseado no tipo de item
                    AudioSystem* audioSystem = game->GetAudioSystem();
                    if (audioSystem)
                    {
                        if (itemName == "Fogo")
                        {
                            audioSystem->PlaySound("fire.wav", false, 128);
                        }
                        else if (itemName == "Água")
                        {
                            audioSystem->PlaySound("water.wav", false, 128);
                        }
                        else if (itemName == "Madeira")
                        {
                            audioSystem->PlaySound("wood.wav", false, 128);
                        }
                    }

                    game->AddActor(std::move(itemActor));
                    
                    // Update the harvest time for this block
                    tileMap->SetBlockHarvestTime(col, row, currentTime);

                    return;
                }
            }
        }
    }
}

void AttackComponent::ApplyDamageAndKnockback(Actor* target, const Vector2& direction)
{
    // Apply damage
    HealthComponent* healthComp = target->GetComponent<HealthComponent>();
    if (healthComp)
    {
        float healthBefore = healthComp->GetCurrentHealth();
        healthComp->TakeDamage(mConfig.damage);
        float healthAfter = healthComp->GetCurrentHealth();

        // SDL_Log("Attack hit! Damage: %.1f | Target health: %.1f -> %.1f",
        //         mConfig.damage, healthBefore, healthAfter);
    }

    // Apply knockback
    if (mConfig.knockback > 0.0f)
    {
        MovementComponent* movementComp = target->GetComponent<MovementComponent>();
        if (movementComp)
        {
            Vector2 knockbackImpulse = direction * mConfig.knockback;
            movementComp->ApplyImpulse(knockbackImpulse);
            // SDL_Log("Knockback applied: direction=(%.2f, %.2f), force=%.1f",
            //         direction.x, direction.y, mConfig.knockback);
        }
    }
}

void AttackComponent::SetAttackAnimationRows(int down, int right, int up)
{
    mConfig.attackDownRow = down;
    mConfig.attackRightRow = right;
    mConfig.attackUpRow = up;
}

int AttackComponent::GetAttackAnimationRow() const
{
    if (!mIsAttacking) return -1;

    switch (mAttackDirection)
    {
        case 0: return mConfig.attackDownRow;   // Down
        case 1: return mConfig.attackRightRow;  // Left (will be flipped)
        case 2: return mConfig.attackRightRow;  // Right
        case 3: return mConfig.attackUpRow;     // Up
        default: return mConfig.attackDownRow;
    }
}

float AttackComponent::GetAttackProgress() const
{
    if (!mIsAttacking || mConfig.attackDuration <= 0.0f) return 0.0f;

    float progress = 1.0f - (mAttackTimer / mConfig.attackDuration);
    return std::max(0.0f, std::min(1.0f, progress));
}

