#include "GoldenStatue.hpp"
#include "../../../../Game/Game.hpp"

GoldenStatueNPC::GoldenStatueNPC(Game* game)
    : AggressiveNPC(game)
{
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/GoldenStatue.tsx");
    SetPosition(Vector2(500.0f, 320.0f));
    SetAnchorPosition(GetPosition());

    // Slower, more defensive
    SetMovementSpeed(40.0f);
    SetChaseSpeed(90.0f);
    SetAggroRange(130.0f);
    SetDeaggroRange(380.0f);
    SetMaxChaseDistance(200.0f);
}

GoldenStatueNPC::~GoldenStatueNPC() {}

