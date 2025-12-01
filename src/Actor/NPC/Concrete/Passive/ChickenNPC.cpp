#include "ChickenNPC.hpp"

ChickenNPC::ChickenNPC(Game* game)
    : PassiveAnimalNPC(game, "assets/tiled/tilesets/Chicken.tsx")
{
    // Customize Chicken stats
    mMoveSpeed = 60.0f;
    mWanderRadius = 80.0f;
    mAnimSpeedRun = 0.15f;

    SetPosition(Vector2(500.0f, 400.0f));
    mHomePosition = Vector2(500.0f, 400.0f);
}

ChickenNPC::~ChickenNPC()
{
}
