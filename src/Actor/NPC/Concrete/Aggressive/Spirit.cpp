#include "Spirit.hpp"
#include "../../../../Game/Game.hpp"

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

SpiritNPC::~SpiritNPC() {}
