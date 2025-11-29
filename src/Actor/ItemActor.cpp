#include "ItemActor.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Game/Game.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>

ItemActor::ItemActor(class Game* game, const Item& item)
    : Actor(game)
    , mItem(item)
    , mShowName(true)
    , mShowEmoji(true)
    , mShowBackground(true)
    , mBackgroundColor(Vector3(0.95f, 0.95f, 0.95f))
    , mBackgroundAlpha(0.3f)
    , mPadding(4.0f)
    , mBorderRadius(4.0f)
    , mBaseScale(0.5f)
    , mDraggable(true)
    , mIsDragging(false)
    , mDragOffset(Vector2::Zero)
    , mSpawnScale(0.0f)
    , mSpawnTimer(0.0f)
    , mBasePosition(Vector2::Zero)
    , mStartOffset(Vector2::Zero)
    , mJumpHeight(20.0f)
{
    // Generate random start offset for the jump
    // Random X between -16 and 16
    float randomX = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 32.0f) - 16.0f;
    mStartOffset = Vector2(randomX, 0.0f);
}

ItemActor::ItemActor(class Game* game, int itemId, const std::string& name, const std::string& emoji)
    : Actor(game)
    , mItem(itemId, name, emoji)
    , mShowName(true)
    , mShowEmoji(true)
    , mShowBackground(true)
    , mBackgroundColor(Vector3(0.95f, 0.95f, 0.95f))
    , mBackgroundAlpha(0.3f)
    , mPadding(4.0f)
    , mBorderRadius(4.0f)
    , mBaseScale(0.5f)
    , mDraggable(true)
    , mIsDragging(false)
    , mDragOffset(Vector2::Zero)
    , mSpawnScale(0.0f)
    , mSpawnTimer(0.0f)
    , mBasePosition(Vector2::Zero)
    , mStartOffset(Vector2::Zero)
    , mJumpHeight(20.0f)
{
    // Generate random start offset for the jump
    float randomX = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 32.0f) - 16.0f;
    mStartOffset = Vector2(randomX, 0.0f);
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

void ItemActor::OnUpdate(float deltaTime)
{
    if (mSpawnTimer < mSpawnDuration)
    {
        // Capture the target position on the first frame of update
        if (mSpawnTimer == 0.0f)
        {
            mBasePosition = GetPosition();
        }

        mSpawnTimer += deltaTime;
        float t = std::min(mSpawnTimer / mSpawnDuration, 1.0f);
        
        // "Back Out" easing function for a pop effect (Scale)
        // s = 1.70158
        float c1 = 2.0f;
        float c3 = c1 + 1;
        
        mSpawnScale = 1 + c3 * std::pow(t - 1, 3) + c1 * std::pow(t - 1, 2);
        
        // Parabolic motion for Position
        // Horizontal: Linear interpolation from (Target + Offset) to Target
        // Vertical: Parabolic arc (up and down)
        
        // Calculate current horizontal offset (fades to 0)
        Vector2 currentOffset = mStartOffset * (1.0f - t);
        
        // Calculate vertical jump offset (parabola)
        // h(t) = 4 * H * t * (1 - t)
        float jumpY = -mJumpHeight * 4.0f * t * (1.0f - t);
        
        // Update position
        SetPosition(mBasePosition + currentOffset + Vector2(0.0f, jumpY));

        if (mSpawnTimer >= mSpawnDuration)
        {
            mSpawnScale = 1.0f;
            SetPosition(mBasePosition); // Ensure we land exactly on target
        }
    }
}void ItemActor::OnDraw(class TextRenderer* textRenderer)
{
    if (!textRenderer)
        return;

    Vector2 pos = GetPosition();
    
    // Adjust for camera
    if (GetGame())
    {
        Vector2 cameraPos = GetGame()->GetCameraPosition();
        pos.x -= cameraPos.x;
        pos.y -= cameraPos.y;
    }

    std::string displayText = GetDisplayText();
    
    // Measure text to calculate background size (unscaled)
    Vector2 textSize = textRenderer->MeasureText(displayText, mBaseScale);
    
    float bgWidth = textSize.x + (mPadding * 2.0f);
    float bgHeight = textSize.y + (mPadding * 2.0f);
    
    // Calculate center of the item
    float centerX = pos.x + bgWidth / 2.0f;
    float centerY = pos.y; // pos.y is vertical center
    
    // Apply scale
    float scaledWidth = bgWidth * mSpawnScale;
    float scaledHeight = bgHeight * mSpawnScale;
    
    float scaledLeftX = centerX - (scaledWidth / 2.0f);
    float scaledTopY = centerY - (scaledHeight / 2.0f);
    
    // Draw background if enabled
    if (mShowBackground)
    {
        auto* game = GetGame();
        if (game && game->GetRectRenderer())
        {
            // Draw background rectangle centered vertically at pos.y
            game->GetRectRenderer()->RenderRect(
                scaledLeftX, 
                scaledTopY, 
                scaledWidth, 
                scaledHeight, 
                mBackgroundColor, 
                mBackgroundAlpha
            );
            
            // Draw white border
            game->GetRectRenderer()->RenderRectOutline(
                scaledLeftX, 
                scaledTopY, 
                scaledWidth, 
                scaledHeight, 
                Vector3(1.0f, 1.0f, 1.0f), // White
                0.8f, // Slightly transparent
                1.0f  // 1px thickness
            );
        }
    }
    
    // Draw text centered inside the background
    // Calculate centered position for text
    float textLeftX = centerX - (textSize.x * mSpawnScale / 2.0f);
    
    // Baseline calculation:
    // CenterY is the middle of the box.
    // Baseline is roughly half the text height below the center.
    float textBaselineY = centerY + (textSize.y * mSpawnScale / 2.0f);
    
    // Ensure text color is black for items (since background is light)
    textRenderer->SetTextColor(0.0f, 0.0f, 0.0f);
    textRenderer->RenderText(displayText, textLeftX, textBaselineY, mBaseScale * mSpawnScale);
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
    
    // Adjust for camera
    if (GetGame())
    {
        Vector2 cameraPos = GetGame()->GetCameraPosition();
        pos.x -= cameraPos.x;
        pos.y -= cameraPos.y;
    }

    std::string displayText = GetDisplayText();
    
    // Get the text renderer from game to measure
    auto* game = GetGame();
    if (!game || !game->GetTextRenderer())
        return false;
    
    Vector2 textSize = game->GetTextRenderer()->MeasureText(displayText, mBaseScale);
    
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
        
        // Store offset relative to world position
        // mousePos is screen coordinates
        // GetPosition() is world coordinates
        // We need to convert mousePos to world coordinates to calculate offset correctly
        Vector2 worldMousePos = mousePos;
        if (GetGame())
        {
            worldMousePos += GetGame()->GetCameraPosition();
        }
        
        mDragOffset = GetPosition() - worldMousePos;
        
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
        // Convert screen mouse pos to world pos
        Vector2 worldMousePos = mousePos;
        if (GetGame())
        {
            worldMousePos += GetGame()->GetCameraPosition();
        }
        
        SetPosition(worldMousePos + mDragOffset);
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
    
    Vector2 textSize = game->GetTextRenderer()->MeasureText(displayText, mBaseScale);
    
    float bgWidth = textSize.x + (mPadding * 2.0f);
    float bgHeight = textSize.y + (mPadding * 2.0f);
    
    return Vector2(bgWidth, bgHeight);
}
