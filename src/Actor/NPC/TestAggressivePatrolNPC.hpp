#pragma once
#include "PatrolNPC.hpp"

// Test aggressive NPC that patrols and chases the player when in range
class TestAggressivePatrolNPC : public PatrolNPC
{
public:
    TestAggressivePatrolNPC(class Game* game);
    ~TestAggressivePatrolNPC();
};

