#pragma once
#include "PatrolNPC.hpp"

// Base class for aggressive NPCs that provides sensible default
// movement/aggression configuration which can be overridden by derived classes.
class AggressiveNPC : public PatrolNPC
{
public:
    AggressiveNPC(class Game* game);
    AggressiveNPC(class Game* game,
                  float moveSpeed,
                  float chaseSpeed,
                  float aggroRange,
                  float deaggroRange,
                  float maxChaseDistance);
    virtual ~AggressiveNPC();

protected:
    void ConfigureDefaults(float moveSpeed, float chaseSpeed, float aggroRange, float deaggroRange, float maxChaseDistance);
};

