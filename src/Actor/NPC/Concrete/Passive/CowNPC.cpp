#include "CowNPC.hpp"

CowNPC::CowNPC(Game* game)
    : PassiveAnimalNPC(game, "assets/tiled/tilesets/Cow.tsx")
{
    // Customize Cow stats
    mMoveSpeed = 40.0f;
    mWanderRadius = 100.0f;
    mAnimSpeedRun = 0.2f;

    SetPosition(Vector2(400.0f, 400.0f));
    mHomePosition = Vector2(400.0f, 400.0f);
}

CowNPC::~CowNPC()
{
}
