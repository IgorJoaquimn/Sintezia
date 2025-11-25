#pragma once
#include "../Base/PatrolNPC.hpp"

// Test passive NPC that patrols in a loop without attacking
class TestPassivePatrolNPC : public PatrolNPC
{
public:
    TestPassivePatrolNPC(class Game* game);
    ~TestPassivePatrolNPC();
};

