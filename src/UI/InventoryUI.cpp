#include "InventoryUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include <SDL.h>
#include <algorithm>

InventoryUI::InventoryUI(Game* game, Inventory* inventory)
    : mGame(game)
    , mInventory(inventory)
    , mVisible(false)
    , mPosition(100.0f, 100.0f)
    , mSlotSize(60.0f)
    , mSlotsPerRow(5)
    , mPadding(10.0f)
    , mSelectedSlot(-1)
    , mHoveredSlot(-1)
    , mBackgroundColor(0.2f, 0.2f, 0.25f)
    , mSlotColor(0.3f, 0.3f, 0.35f)
    , mSlotHoverColor(0.4f, 0.4f, 0.45f)
    , mSlotSelectedColor(0.5f, 0.6f, 0.7f)
    , mTextColor(1.0f, 1.0f, 1.0f)
{
    // Initialize key states
    for (int i = 0; i < 10; i++)
    {
        mKeyPressed[i] = false;
    }
}

InventoryUI::~InventoryUI()
{
}

void InventoryUI::Show()
{
    mVisible = true;
}

void InventoryUI::Hide()
{
    mVisible = false;
    mSelectedSlot = -1;
    mHoveredSlot = -1;
}

void InventoryUI::Toggle()
{
    if (mVisible)
        Hide();
    else
        Show();
}

void InventoryUI::Update(float deltaTime)
{
    if (!mVisible) return;

    // Update logic here if needed
}

void InventoryUI::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!mVisible || !mInventory) return;

    DrawInventoryBackground(rectRenderer);
    DrawInventorySlots(textRenderer, rectRenderer);
}

void InventoryUI::HandleInput(const uint8_t* keyState)
{
    if (!mVisible) return;

    UpdateKeyState(keyState);

    // Close inventory with ESC or I
    if (keyState[SDL_SCANCODE_ESCAPE] && !mKeyPressed[0])
    {
        Hide();
        mKeyPressed[0] = true;
    }
    else if (!keyState[SDL_SCANCODE_ESCAPE])
    {
        mKeyPressed[0] = false;
    }

    if (keyState[SDL_SCANCODE_I] && !mKeyPressed[1])
    {
        Toggle();
        mKeyPressed[1] = true;
    }
    else if (!keyState[SDL_SCANCODE_I])
    {
        mKeyPressed[1] = false;
    }
}

void InventoryUI::HandleMouseClick(const Vector2& mousePos)
{
    if (!mVisible) return;

    int clickedSlot = GetSlotAtPosition(mousePos);
    
    if (clickedSlot != -1 && clickedSlot < mInventory->GetUsedSlots())
    {
        mSelectedSlot = clickedSlot;
        
        // Trigger callback
        if (mOnItemSelected)
        {
            const InventorySlot* slot = mInventory->GetSlot(clickedSlot);
            if (slot)
            {
                mOnItemSelected(slot->item.id);
            }
        }
    }
    else
    {
        mSelectedSlot = -1;
    }
}

void InventoryUI::HandleMouseMove(const Vector2& mousePos)
{
    if (!mVisible) return;

    mHoveredSlot = GetSlotAtPosition(mousePos);
}

void InventoryUI::DrawInventoryBackground(RectRenderer* rectRenderer)
{
    if (!rectRenderer) return;

    int rows = (mInventory->GetMaxSlots() + mSlotsPerRow - 1) / mSlotsPerRow;
    float width = mSlotsPerRow * mSlotSize + (mSlotsPerRow + 1) * mPadding;
    float height = rows * mSlotSize + (rows + 1) * mPadding + 40.0f; // Extra space for title

    rectRenderer->RenderRect(
        mPosition.x,
        mPosition.y,
        width,
        height,
        mBackgroundColor,
        0.95f
    );
}

void InventoryUI::DrawInventorySlots(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw title
    textRenderer->RenderText("Inventory", mPosition.x + mPadding, mPosition.y + mPadding + 20.0f, 0.8f);

    // Draw slots
    int maxSlots = mInventory->GetMaxSlots();
    for (int i = 0; i < maxSlots; ++i)
    {
        Vector2 slotPos = GetSlotPosition(i);
        
        // Determine slot color
        Vector3 slotColor = mSlotColor;
        if (i == mSelectedSlot)
            slotColor = mSlotSelectedColor;
        else if (i == mHoveredSlot)
            slotColor = mSlotHoverColor;

        // Draw slot background
        rectRenderer->RenderRect(
            slotPos.x,
            slotPos.y,
            mSlotSize,
            mSlotSize,
            slotColor,
            0.9f
        );

        // Draw item if slot is filled
        if (i < mInventory->GetUsedSlots())
        {
            DrawItemInSlot(i, slotPos, textRenderer, rectRenderer);
        }
    }
}

void InventoryUI::DrawItemInSlot(int slotIndex, const Vector2& slotPos, TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    const InventorySlot* slot = mInventory->GetSlot(slotIndex);
    if (!slot) return;

    // Draw item emoji
    float emojiScale = 1.2f;
    Vector2 emojiSize = textRenderer->MeasureText(slot->item.emoji, emojiScale);
    float emojiX = slotPos.x + (mSlotSize - emojiSize.x) / 2.0f;
    float emojiY = slotPos.y + (mSlotSize / 2.0f);
    textRenderer->RenderText(slot->item.emoji, emojiX, emojiY, emojiScale);

    // Draw quantity in bottom-right corner
    if (slot->quantity > 1)
    {
        std::string quantityText = std::to_string(slot->quantity);
        float quantityScale = 0.5f;
        Vector2 quantitySize = textRenderer->MeasureText(quantityText, quantityScale);
        float quantityX = slotPos.x + mSlotSize - quantitySize.x - 5.0f;
        float quantityY = slotPos.y + mSlotSize - 5.0f;
        textRenderer->RenderText(quantityText, quantityX, quantityY, quantityScale);
    }

    // Draw item name on hover
    if (slotIndex == mHoveredSlot)
    {
        float nameScale = 0.6f;
        Vector2 nameSize = textRenderer->MeasureText(slot->item.name, nameScale);
        float nameX = slotPos.x + (mSlotSize - nameSize.x) / 2.0f;
        float nameY = slotPos.y - 15.0f;
        
        // Draw name background
        rectRenderer->RenderRect(
            nameX - 5.0f,
            nameY - nameSize.y - 2.0f,
            nameSize.x + 10.0f,
            nameSize.y + 4.0f,
            Vector3(0.1f, 0.1f, 0.15f),
            0.95f
        );
        
        textRenderer->RenderText(slot->item.name, nameX, nameY, nameScale);
    }
}

Vector2 InventoryUI::GetSlotPosition(int slotIndex) const
{
    int row = slotIndex / mSlotsPerRow;
    int col = slotIndex % mSlotsPerRow;

    float x = mPosition.x + mPadding + col * (mSlotSize + mPadding);
    float y = mPosition.y + 40.0f + mPadding + row * (mSlotSize + mPadding);

    return Vector2(x, y);
}

int InventoryUI::GetSlotAtPosition(const Vector2& mousePos) const
{
    int maxSlots = mInventory->GetMaxSlots();
    
    for (int i = 0; i < maxSlots; ++i)
    {
        Vector2 slotPos = GetSlotPosition(i);
        
        if (mousePos.x >= slotPos.x && mousePos.x <= slotPos.x + mSlotSize &&
            mousePos.y >= slotPos.y && mousePos.y <= slotPos.y + mSlotSize)
        {
            return i;
        }
    }
    
    return -1;
}

void InventoryUI::UpdateKeyState(const uint8_t* keyState)
{
    // Track key states for debouncing
}
