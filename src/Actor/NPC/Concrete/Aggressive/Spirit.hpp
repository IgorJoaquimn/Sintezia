#pragma once
#include "../../Base/AggressiveNPC.hpp"

class SpiritNPC : public AggressiveNPC
{
public:
    SpiritNPC(class Game* game);
    ~SpiritNPC();

    void OnDeath() override;
};

