#include "Statue.hpp"
#include "../../../../Game/Game.hpp"
#include "../../../../Actor/ItemActor.hpp"
#include "../../../../Crafting/Crafting.hpp"
#include "../../../../Crafting/Item.hpp"

StatueNPC::StatueNPC(Game* game)
    : AggressiveNPC(game)
{
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Statue.tsx");
    SetPosition(Vector2(480.0f, 360.0f));
    SetAnchorPosition(GetPosition());

    // Heavy and slow
    SetMovementSpeed(35.0f);
    SetChaseSpeed(80.0f);
    SetAggroRange(120.0f);
    SetDeaggroRange(360.0f);
    SetMaxChaseDistance(180.0f);
}

StatueNPC::~StatueNPC()
{
}

void StatueNPC::OnDeath()
{
    // Drop Stone (ID 12)
    if (mGame && mGame->GetCrafting())
    {
        const Item* stone = mGame->GetCrafting()->FindItemById(12);
        if (stone)
        {
            auto itemActor = std::make_unique<ItemActor>(mGame, *stone);
            itemActor->SetPosition(GetPosition());
            mGame->AddActor(std::move(itemActor));
        }
    }
    
    NPC::OnDeath();
}

