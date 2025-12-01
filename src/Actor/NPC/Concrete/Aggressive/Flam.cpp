#include "Flam.hpp"
#include "../../../../Game/Game.hpp"
#include "../../../../Actor/ItemActor.hpp"
#include "../../../../Crafting/Crafting.hpp"
#include "../../../../Crafting/Item.hpp"

FlamNPC::FlamNPC(Game* game)
    : AggressiveNPC(game)
{
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Flam.tsx");
    SetPosition(Vector2(600.0f, 300.0f));
    SetAnchorPosition(GetPosition());

    // Slightly faster than base
    SetMovementSpeed(70.0f);
    SetChaseSpeed(150.0f);
    SetAggroRange(160.0f);
    SetDeaggroRange(420.0f);
    SetMaxChaseDistance(260.0f);
}

FlamNPC::~FlamNPC()
{
}

void FlamNPC::OnDeath()
{
    // Drop Fire (ID 2)
    if (mGame && mGame->GetCrafting())
    {
        const Item* fire = mGame->GetCrafting()->FindItemById(2);
        if (fire)
        {
            auto itemActor = std::make_unique<ItemActor>(mGame, *fire);
            itemActor->SetPosition(GetPosition());
            mGame->AddActor(std::move(itemActor));
        }
    }
    
    NPC::OnDeath();
}
