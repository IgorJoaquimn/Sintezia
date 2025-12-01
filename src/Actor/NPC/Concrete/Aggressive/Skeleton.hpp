#pragma once
#include "../../Base/PatrolNPC.hpp"

// Aggressive skeleton NPC that extends the PatrolNPC behavior
class SkeletonNPC : public PatrolNPC
{
public:
    SkeletonNPC(class Game* game);
    ~SkeletonNPC();

    void OnDeath() override;
};
