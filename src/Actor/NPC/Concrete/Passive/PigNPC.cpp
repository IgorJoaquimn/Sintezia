#include "PigNPC.hpp"

PigNPC::PigNPC(Game* game)
    : PassiveAnimalNPC(game, "assets/tiled/tilesets/Pig.tsx")
{
    // Customize Pig stats
    mMoveSpeed = 45.0f;
    mWanderRadius = 90.0f;
    mAnimSpeedRun = 0.25f;

    SetPosition(Vector2(600.0f, 500.0f));
    mHomePosition = Vector2(600.0f, 500.0f);
}

PigNPC::~PigNPC()
{
}
