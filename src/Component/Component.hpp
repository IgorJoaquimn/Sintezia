// ----------------------------------------------------------------
// Component class following the asteroids game architecture
// ----------------------------------------------------------------

#pragma once
#include <cstdint>

class Component
{
public:
    // Constructor
    // (the lower the update order, the earlier the component updates)
    Component(class Actor* owner, int updateOrder = 100);
    // Destructor
    virtual ~Component();
    // Update this component by delta time
    virtual void Update(float deltaTime);
    // Process input for this component (if needed)
    virtual void ProcessInput(const uint8_t* keyState);

    int GetUpdateOrder() const { return mUpdateOrder; }
    class Actor* GetOwner() const { return mOwner; }
    class Game* GetGame() const;

protected:
    // Owning actor
    class Actor* mOwner;
    // Update order
    int mUpdateOrder;
};