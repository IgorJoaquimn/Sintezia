#include "Skeleton.hpp"
#include "../../../../Game/Game.hpp"
#include "../../../../Actor/ItemActor.hpp"
#include "../../../../Crafting/Crafting.hpp"
#include "../../../../Crafting/Item.hpp"

SkeletonNPC::SkeletonNPC(Game* game)
    : PatrolNPC(game, true) // true = aggressive
{
    // Configure sprite to use Skeleton sprite sheet from tsx
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Skeleton.tsx");

    // Skeleton sprite layout defaults assumed (column-based directions)

    // Set initial position and anchor
    SetPosition(Vector2(700.0f, 300.0f));
    SetAnchorPosition(Vector2(700.0f, 300.0f));

    // Configure movement
    SetMovementSpeed(60.0f);   // Slower patrol speed
    SetChaseSpeed(120.0f);     // Faster chase speed

    // Configure aggression ranges
    SetAggroRange(150.0f);     // Start chasing when player is within 150 units
    SetDeaggroRange(400.0f);   // Stop chasing when NPC is 400 units from anchor
    SetMaxChaseDistance(250.0f); // Stop chasing if player gets more than 250 units away from NPC

    // Intentionally do not add waypoints here; patrol will remain idle at anchor until waypoints added
}

SkeletonNPC::~SkeletonNPC()
{
}

void SkeletonNPC::OnDeath()
{
    // Drop Bone (ID 200)
    if (mGame && mGame->GetCrafting())
    {
        const Item* bone = mGame->GetCrafting()->FindItemById(200);
        if (bone)
        {
            auto itemActor = std::make_unique<ItemActor>(mGame, *bone);
            itemActor->SetPosition(GetPosition());
            mGame->AddActor(std::move(itemActor));
        }
    }
    
    NPC::OnDeath();
}
