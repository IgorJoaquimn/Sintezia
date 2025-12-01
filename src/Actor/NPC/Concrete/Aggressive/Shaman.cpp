#include "Shaman.hpp"
#include "../../../../Game/Game.hpp"
#include "../../../../Actor/ItemActor.hpp"
#include "../../../../Crafting/Crafting.hpp"
#include "../../../../Crafting/Item.hpp"

ShamanNPC::ShamanNPC(Game* game)
    : AggressiveNPC(game)
{
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Shaman.tsx");
    SetPosition(Vector2(720.0f, 280.0f));
    SetAnchorPosition(GetPosition());

    // Shaman has moderate speed and larger aggro
    SetMovementSpeed(65.0f);
    SetChaseSpeed(150.0f);
    SetAggroRange(200.0f);
    SetDeaggroRange(450.0f);
    SetMaxChaseDistance(300.0f);
}

ShamanNPC::~ShamanNPC()
{
}

void ShamanNPC::OnDeath()
{
    // Drop Ancient Scroll (ID 108)
    if (mGame && mGame->GetCrafting())
    {
        const Item* scroll = mGame->GetCrafting()->FindItemById(108);
        if (scroll)
        {
            auto itemActor = std::make_unique<ItemActor>(mGame, *scroll);
            itemActor->SetPosition(GetPosition());
            mGame->AddActor(std::move(itemActor));
        }
    }
    
    NPC::OnDeath();
}

