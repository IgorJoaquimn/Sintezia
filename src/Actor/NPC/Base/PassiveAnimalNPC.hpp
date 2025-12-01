#pragma once
#include "NPC.hpp"

class PassiveAnimalNPC : public NPC
{
public:
    PassiveAnimalNPC(class Game* game, const std::string& spriteSheetPath);
    virtual ~PassiveAnimalNPC();

    void OnUpdate(float deltaTime) override;
    void OnDeath() override;

protected:
    // Configuration parameters
    float mMoveSpeed;
    float mWanderRadius;
    float mAnimSpeedRun;
    Vector2 mHomePosition;

private:
    // Wander state
    float mWanderTimer;
    Vector2 mWanderTarget;
    bool mIsMoving;
    int mCurrentFrame;
};
