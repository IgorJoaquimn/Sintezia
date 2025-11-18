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
    , mDraggable(true)
    , mIsDragging(false)
    , mDragOffset(Vector2::Zero)
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
    , mDraggable(true)
    , mIsDragging(false)
    , mDragOffset(Vector2::Zero)
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

bool ItemActor::ContainsPoint(const Vector2& point) const
{
    Vector2 pos = GetPosition();
    std::string displayText = GetDisplayText();
    
    // Get the text renderer from game to measure
    auto* game = GetGame();
    if (!game || !game->GetTextRenderer())
        return false;
    
    Vector2 textSize = game->GetTextRenderer()->MeasureText(displayText, 1.0f);
    
    float bgWidth = textSize.x + (mPadding * 2.0f);
    float bgHeight = textSize.y + (mPadding * 2.0f);
    float bgTopY = pos.y - (bgHeight / 2.0f);
    
    // Check if point is within the bounds
    return point.x >= pos.x && point.x <= pos.x + bgWidth &&
           point.y >= bgTopY && point.y <= bgTopY + bgHeight;
}

void ItemActor::OnMouseDown(const Vector2& mousePos)
{
    if (!mDraggable || mIsDragging)
        return;
        
    if (ContainsPoint(mousePos))
    {
        mIsDragging = true;
        mDragOffset = GetPosition() - mousePos;
        
        // Visual feedback: make background more opaque when dragging
        mBackgroundAlpha = 0.6f;
    }
}

void ItemActor::OnMouseUp(const Vector2& mousePos)
{
    if (mIsDragging)
    {
        mIsDragging = false;
        
        // Reset background alpha
        mBackgroundAlpha = 0.3f;
    }
}

void ItemActor::OnMouseMove(const Vector2& mousePos)
{
    if (mIsDragging)
    {
        SetPosition(mousePos + mDragOffset);
    }
}

bool ItemActor::Intersects(const ItemActor* other) const
{
    if (!other)
        return false;
    
    Vector2 pos1 = GetPosition();
    Vector2 pos2 = other->GetPosition();
    Vector2 bounds1 = GetBounds();
    Vector2 bounds2 = other->GetBounds();
    
    // Calculate the bounds (top-left based)
    float bgTopY1 = pos1.y - (bounds1.y / 2.0f);
    float bgTopY2 = pos2.y - (bounds2.y / 2.0f);
    
    // AABB collision detection
    return !(pos1.x + bounds1.x < pos2.x ||
             pos2.x + bounds2.x < pos1.x ||
             bgTopY1 + bounds1.y < bgTopY2 ||
             bgTopY2 + bounds2.y < bgTopY1);
}

Vector2 ItemActor::GetBounds() const
{
    std::string displayText = GetDisplayText();
    
    auto* game = GetGame();
    if (!game || !game->GetTextRenderer())
        return Vector2(100.0f, 50.0f); // Fallback
    
    Vector2 textSize = game->GetTextRenderer()->MeasureText(displayText, 1.0f);
    
    float bgWidth = textSize.x + (mPadding * 2.0f);
    float bgHeight = textSize.y + (mPadding * 2.0f);
    
    return Vector2(bgWidth, bgHeight);
}
