#include "ItemActor.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Game/Game.hpp"

ItemActor::ItemActor(class Game* game, const Item& item)
    : Actor(game)
    , mItem(item)
    , mShowName(true)
    , mShowEmoji(true)
{
}

ItemActor::ItemActor(class Game* game, int itemId, const std::string& name, const std::string& emoji)
    : Actor(game)
    , mItem(itemId, name, emoji)
    , mShowName(true)
    , mShowEmoji(true)
{
}

void ItemActor::SetItem(const Item& item)
{
    mItem = item;
}

std::string ItemActor::GetDisplayText() const
{
    std::string text;
    
    if (mShowEmoji && mShowName)
    {
        text = mItem.emoji + " " + mItem.name;
    }
    else if (mShowEmoji)
    {
        text = mItem.emoji;
    }
    else if (mShowName)
    {
        text = mItem.name;
    }
    
    return text;
}

void ItemActor::OnDraw(class TextRenderer* textRenderer)
{
    if (textRenderer)
    {
        Vector2 pos = GetPosition();
        std::string displayText = GetDisplayText();
        textRenderer->RenderText(displayText, pos.x, pos.y, 1.0f);
    }
}
