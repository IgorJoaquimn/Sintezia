#pragma once
#include "../Base/DialogNPC.hpp"

// Cat NPC - a friendly cat that runs away when player gets close
class CatNPC : public DialogNPC
{
public:
    CatNPC(class Game* game);
    ~CatNPC();
    
    void OnUpdate(float deltaTime) override;
    
private:
    int mCurrentFrame;
    
    // Flee behavior
    Vector2 mHomePosition;  // Where the cat returns to when calm
    bool mIsFleeing;
    float mFleeTimer;  // Time to keep fleeing
    
    static constexpr float ANIM_SPEED = 0.5f; // Slow animation (2 seconds per cycle)
    static constexpr float FLEE_RANGE = 150.0f; // Distance to trigger flee
    static constexpr float FLEE_SPEED = 200.0f; // Speed when running away
    static constexpr float RETURN_SPEED = 80.0f; // Speed when returning home
    static constexpr float FLEE_DURATION = 2.0f; // How long to flee before calming
};
