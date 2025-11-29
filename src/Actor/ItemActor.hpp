#pragma once
#include "Actor.hpp"
#include "../Crafting/Item.hpp"
#include <string>
#include <memory>

class ItemActor : public Actor
{
public:
    ItemActor(class Game* game, const Item& item);
    ItemActor(class Game* game, int itemId, const std::string& name, const std::string& emoji);
    
    const Item& GetItem() const { return mItem; }
    void SetItem(const Item& item);
    
    // Display options
    void SetShowName(bool show) { mShowName = show; }
    void SetShowEmoji(bool show) { mShowEmoji = show; }
    bool GetShowName() const { return mShowName; }
    bool GetShowEmoji() const { return mShowEmoji; }
    
    // Background options
    void SetShowBackground(bool show) { mShowBackground = show; }
    void SetBackgroundColor(const Vector3& color) { mBackgroundColor = color; }
    void SetBackgroundAlpha(float alpha) { mBackgroundAlpha = alpha; }
    void SetPadding(float padding) { mPadding = padding; }
    void SetBorderRadius(float radius) { mBorderRadius = radius; }
    
    // Get dimensions
    Vector2 GetTextDimensions(float scale = 1.0f) const;
    float GetTextWidth(float scale = 1.0f) const;
    float GetTextHeight(float scale = 1.0f) const;
    
    // Drag functionality
    void SetDraggable(bool draggable) { mDraggable = draggable; }
    bool IsDraggable() const { return mDraggable; }
    bool IsDragging() const { return mIsDragging; }
    bool ContainsPoint(const Vector2& point) const;
    void OnMouseDown(const Vector2& mousePos);
    void OnMouseUp(const Vector2& mousePos);
    void OnMouseMove(const Vector2& mousePos);
    
    // Collision detection
    bool Intersects(const ItemActor* other) const;
    Vector2 GetBounds() const; // Returns width and height
    
    // Pickup functionality
    void StartPickup(Actor* target);
    bool IsBeingPickedUp() const { return mIsBeingPickedUp; }

protected:
    void OnUpdate(float deltaTime) override;
    void OnDraw(class TextRenderer* textRenderer) override;
    
private:
    Item mItem;
    bool mShowName;
    bool mShowEmoji;
    bool mShowBackground;
    Vector3 mBackgroundColor;
    float mBackgroundAlpha;
    float mPadding;
    float mBorderRadius;
    float mBaseScale;
    
    // Animation state
    float mSpawnScale;
    float mSpawnTimer;
    const float mSpawnDuration = 0.5f;
    Vector2 mBasePosition; // The target position where the item should land
    Vector2 mStartOffset;  // Random offset for the jump start
    float mJumpHeight;     // Height of the parabolic arc

    // Drag state
    bool mDraggable;
    bool mIsDragging;
    Vector2 mDragOffset; // Offset from item position to mouse when drag started
    
    // Pickup state
    bool mIsBeingPickedUp;
    Actor* mPickupTarget;
    float mPickupSpeed;

    std::string GetDisplayText() const;
};
