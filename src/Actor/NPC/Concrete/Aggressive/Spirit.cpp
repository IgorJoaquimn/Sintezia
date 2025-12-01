#include "Spirit.hpp"
#include "../../../../Game/Game.hpp"
#include "../../../../Actor/ItemActor.hpp"
#include "../../../../Crafting/Crafting.hpp"
#include "../../../../Crafting/Item.hpp"

SpiritNPC::SpiritNPC(Game* game)
    : AggressiveNPC(game)
{
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Spirit.tsx");
    SetPosition(Vector2(660.0f, 340.0f));
    SetAnchorPosition(GetPosition());

    // Fast and agile
    SetMovementSpeed(90.0f);
    SetChaseSpeed(170.0f);
    SetAggroRange(170.0f);
    SetDeaggroRange(420.0f);
    SetMaxChaseDistance(280.0f);
}

SpiritNPC::~SpiritNPC()
{
}

void SpiritNPC::OnDeath()
{
    // Drop Water (ID 1)
    if (mGame && mGame->GetCrafting())
    {
        const Item* water = mGame->GetCrafting()->FindItemById(1);
        if (water)
        {
            auto itemActor = std::make_unique<ItemActor>(mGame, *water);
            itemActor->SetPosition(GetPosition());
            mGame->AddActor(std::move(itemActor));
        }
    }
    
    NPC::OnDeath();
}
