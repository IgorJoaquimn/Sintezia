#include "AggressiveNPC.hpp"
#include "../../../Game/Game.hpp"

AggressiveNPC::AggressiveNPC(Game* game)
    : PatrolNPC(game, true)
{
    // sensible defaults tuned for aggressive NPCs
    ConfigureDefaults(60.0f, 140.0f, 150.0f, 400.0f, 250.0f);
}

AggressiveNPC::AggressiveNPC(Game* game, float moveSpeed, float chaseSpeed, float aggroRange, float deaggroRange, float maxChaseDistance)
    : PatrolNPC(game, true)
{
    ConfigureDefaults(moveSpeed, chaseSpeed, aggroRange, deaggroRange, maxChaseDistance);
}

AggressiveNPC::~AggressiveNPC()
{
}

void AggressiveNPC::ConfigureDefaults(float moveSpeed, float chaseSpeed, float aggroRange, float deaggroRange, float maxChaseDistance)
{
    SetMovementSpeed(moveSpeed);
    SetChaseSpeed(chaseSpeed);
    SetAggroRange(aggroRange);
    SetDeaggroRange(deaggroRange);
    SetMaxChaseDistance(maxChaseDistance);
}
