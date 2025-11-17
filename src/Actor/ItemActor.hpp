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
    
protected:
    void OnDraw(class TextRenderer* textRenderer) override;
    
private:
    Item mItem;
    bool mShowName;
    bool mShowEmoji;
    
    std::string GetDisplayText() const;
};
