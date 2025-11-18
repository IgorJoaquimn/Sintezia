#include "ItemActor.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Game/Game.hpp"

ItemActor::ItemActor(class Game* game, const Item& item)
    : Actor(game)
    , mItem(item)
    , mShowName(true)
    , mShowEmoji(true)
    , mShowBackground(true)
    , mBackgroundColor(Vector3(0.95f, 0.95f, 0.95f))
    , mBackgroundAlpha(0.3f)
    , mPadding(25.0f)
    , mBorderRadius(10.0f)
{
}

ItemActor::ItemActor(class Game* game, int itemId, const std::string& name, const std::string& emoji)
    : Actor(game)
    , mItem(itemId, name, emoji)
    , mShowName(true)
    , mShowEmoji(true)
    , mShowBackground(true)
    , mBackgroundColor(Vector3(0.95f, 0.95f, 0.95f))
    , mBackgroundAlpha(0.3f)
    , mPadding(25.0f)
    , mBorderRadius(10.0f)
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
    if (!textRenderer)
        return;

    Vector2 pos = GetPosition();
    std::string displayText = GetDisplayText();
    
    // Measure text to calculate background size
    Vector2 textSize = textRenderer->MeasureText(displayText, 1.0f);
    
    float bgWidth = textSize.x + (mPadding * 2.0f);
    float bgHeight = textSize.y + (mPadding * 2.0f);
    
    // Calculate top-left position so the background is centered vertically at pos.y
    float bgTopY = pos.y - (bgHeight / 2.0f);
    
    // Draw background if enabled
    if (mShowBackground)
    {
        auto* game = GetGame();
        if (game && game->GetRectRenderer())
        {
            // Draw background rectangle centered vertically at pos.y
            game->GetRectRenderer()->RenderRect(
                pos.x, 
                bgTopY, 
                bgWidth, 
                bgHeight, 
                mBackgroundColor, 
                mBackgroundAlpha
            );
        }
    }
    
    // Draw text centered inside the background
    // Text Y coordinate acts as baseline, so we need to offset by text height
    // to position the top of the text at bgTopY + padding
    float textX = pos.x + mPadding;
    float textY = bgTopY + mPadding + textSize.y; // Add height to position baseline correctly
    textRenderer->RenderText(displayText, textX, textY, 1.0f);
}

Vector2 ItemActor::GetTextDimensions(float scale) const
{
    std::string displayText = GetDisplayText();
    
    // Get the text renderer from game
    auto* game = GetGame();
    if (game && game->GetTextRenderer())
    {
        return game->GetTextRenderer()->MeasureText(displayText, scale);
    }
    
    // Fallback: estimate
    float width = displayText.length() * 12.0f * scale;
    float height = 20.0f * scale;
    return Vector2(width, height);
}

float ItemActor::GetTextWidth(float scale) const
{
    return GetTextDimensions(scale).x;
}

float ItemActor::GetTextHeight(float scale) const
{
    return GetTextDimensions(scale).y;
}
