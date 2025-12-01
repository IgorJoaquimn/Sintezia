#include "Statue.hpp"
#include "../../../../Game/Game.hpp"

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

StatueNPC::~StatueNPC() {}

