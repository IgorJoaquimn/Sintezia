// ----------------------------------------------------------------
// Actor class following the asteroids game architecture
// ----------------------------------------------------------------

#pragma once
#include <vector>
#include <cstdint>
#include <memory>
#include "../Math.h"

// Forward declarations
class Game;
class Component;

bool ComponentUpdateOrderCompare(Component* a, Component* b);

enum class ActorState
{
    Active,
    Paused,
    Destroy
};

class Actor
{
public:
    Actor(class Game* game);
    virtual ~Actor();

    // Update function called from Game (not overridable)
    void Update(float deltaTime);
    // ProcessInput function called from Game (not overridable)
    void ProcessInput(const uint8_t* keyState);

    // Position getter/setter
    const Vector2& GetPosition() const { return mPosition; }
    void SetPosition(const Vector2& pos) { mPosition = pos; }

    // Scale getter/setter
    const Vector2& GetScale() const { return mScale; }
    void SetScale(const Vector2& scale) { mScale = scale; }

    // Rotation getter/setter
    float GetRotation() const { return mRotation; }
    void SetRotation(float rotation) { mRotation = rotation; }

    // State getter/setter
    ActorState GetState() const { return mState; }
    void SetState(ActorState state) { mState = state; }

    // Get Forward vector
    Vector2 GetForward() const
    {
        return Vector2(Math::Sin(mRotation), -Math::Cos(mRotation));
    }

    // Model matrix
    Matrix4 GetModelMatrix() const;
    // Game getter
    class Game* GetGame() { return mGame; }
    const class Game* GetGame() const { return mGame; }

    // Components getter
    const std::vector<std::unique_ptr<Component>>& GetComponents() const { return mComponents; }

    // Returns component of type T, or null if doesn't exist
    template <typename T>
    T* GetComponent() const
    {
        for (auto& comp : mComponents)
        {
            T* t = dynamic_cast<T*>(comp.get());
            if (t)
            {
                return t;
            }
        }
        
        return nullptr;
    }

    // Add a component and return a pointer to it
    template <typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* ptr = component.get();
        AddComponent(std::move(component));
        return ptr;
    }

    // Drawing method for rendering
    virtual void OnDraw(class TextRenderer* textRenderer);

protected:
    class Game* mGame;

    // Any actor-specific update code (overridable)
    virtual void OnUpdate(float deltaTime);
    // Any actor-specific update code (overridable)
    virtual void OnProcessInput(const uint8_t* keyState);

    // Actor's state
    ActorState mState;

    // Transform
    Vector2 mPosition;
    Vector2 mScale;
    float mRotation;

    // Components
    std::vector<std::unique_ptr<Component>> mComponents;

private:
    friend class Component;

    // Adds component to Actor (this is automatically called
    // in the component constructor)
    void AddComponent(std::unique_ptr<Component> c);
};