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
    
protected:
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
    
    std::string GetDisplayText() const;
};
