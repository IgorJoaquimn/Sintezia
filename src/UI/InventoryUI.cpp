#include "InventoryUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Map/TileMap.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include <SDL.h>
#include <algorithm>

InventoryUI::InventoryUI(Game* game, Inventory* inventory)
    : mGame(game)
    , mInventory(inventory)
    , mVisible(false)
    , mPosition(100.0f, 100.0f)
    , mSlotSize(60.0f)
    , mSlotsPerRow(4)
    , mPadding(10.0f)
    , mSelectedSlot(-1)
    , mHoveredSlot(-1)
    , mBackgroundColor(0.2f, 0.2f, 0.25f)
    , mSlotColor(0.3f, 0.3f, 0.35f)
    , mSlotHoverColor(0.4f, 0.4f, 0.45f)
    , mSlotSelectedColor(0.5f, 0.6f, 0.7f)
    , mTextColor(1.0f, 1.0f, 1.0f)
    , mCraftInputSlot1(-1)
    , mCraftInputSlot2(-1)
    , mCraftResult(nullptr)
    , mBackgroundMap(nullptr)
    , mMapScale(4.0f)
    , mSelectionCursorGID(0)
{
    // Initialize key states
    for (int i = 0; i < 10; i++)
    {
        mKeyPressed[i] = false;
    }

    // Load background map
    mBackgroundMap = std::make_unique<TileMap>(12, 6, 16);
    if (!mBackgroundMap->LoadFromJSON("assets/maps/inventario.json"))
    {
        SDL_Log("Failed to load inventory background map");
    }
    else
    {
        mSelectionCursorGID = mBackgroundMap->GetGIDFromLayer("selected_icon");
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
    ClearCraftingSlots();
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

void InventoryUI::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer)
{
    if (!mVisible || !mInventory) return;

    // Draw background map
    if (mBackgroundMap && spriteRenderer)
    {
        mBackgroundMap->Draw(spriteRenderer, mPosition, mMapScale);
    }

    DrawInventorySlots(textRenderer, rectRenderer);
    DrawCraftingPanel(textRenderer, rectRenderer);
}

void InventoryUI::HandleInput(const uint8_t* keyState)
{
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

    if ((keyState[SDL_SCANCODE_I] || keyState[SDL_SCANCODE_TAB]) && !mKeyPressed[1])
    {
        Toggle();
        mKeyPressed[1] = true;
    }
    else if (!keyState[SDL_SCANCODE_I] && !keyState[SDL_SCANCODE_TAB])
    {
        mKeyPressed[1] = false;
    }
}

void InventoryUI::HandleMouseClick(const Vector2& mousePos)
{
    if (!mVisible) return;

    // Check crafting slots first
    Vector2 resultPos = GetCraftingSlotPosition(2);
    Vector2 input1Pos = GetCraftingSlotPosition(0);
    Vector2 input2Pos = GetCraftingSlotPosition(1);
    float resultSize = mSlotSize * 1.5f;

    // Check Result Slot
    if (IsPointInRect(mousePos, resultPos, Vector2(resultSize, resultSize)))
    {
        PerformCraft();
        return;
    }

    // Check Input 1
    if (IsPointInRect(mousePos, input1Pos, Vector2(mSlotSize, mSlotSize)))
    {
        mCraftInputSlot1 = -1;
        UpdateCraftingResult();
        return;
    }

    // Check Input 2
    if (IsPointInRect(mousePos, input2Pos, Vector2(mSlotSize, mSlotSize)))
    {
        mCraftInputSlot2 = -1;
        UpdateCraftingResult();
        return;
    }

    // Check Inventory Slots
    int clickedSlot = GetSlotAtPosition(mousePos);
    
    if (clickedSlot != -1 && clickedSlot < mInventory->GetUsedSlots())
    {
        // Add to crafting slots
        if (mCraftInputSlot1 == -1)
        {
            mCraftInputSlot1 = clickedSlot;
        }
        else if (mCraftInputSlot2 == -1)
        {
            mCraftInputSlot2 = clickedSlot;
        }
        else
        {
            // Both full, replace slot 1 (cycle)
            mCraftInputSlot1 = mCraftInputSlot2;
            mCraftInputSlot2 = clickedSlot;
        }
        UpdateCraftingResult();
    }
}

void InventoryUI::HandleMouseMove(const Vector2& mousePos)
{
    if (!mVisible) return;

    mHoveredSlot = GetSlotAtPosition(mousePos);
}

Vector2 InventoryUI::GetDimensions() const
{
    if (mBackgroundMap)
    {
        return Vector2(
            mBackgroundMap->GetWidth() * mBackgroundMap->GetTileSize() * mMapScale,
            mBackgroundMap->GetHeight() * mBackgroundMap->GetTileSize() * mMapScale
        );
    }

    if (!mInventory) return Vector2::Zero;

    int rows = (mInventory->GetMaxSlots() + mSlotsPerRow - 1) / mSlotsPerRow;
    float width = mSlotsPerRow * mSlotSize + (mSlotsPerRow + 1) * mPadding;
    float height = rows * mSlotSize + (rows + 1) * mPadding + 70.0f; // Extra space for title and instructions
    
    return Vector2(width, height);
}

void InventoryUI::CenterOnScreen(float screenWidth, float screenHeight)
{
    Vector2 dims = GetDimensions();
    mPosition.x = (screenWidth - dims.x) / 2.0f;
    mPosition.y = (screenHeight - dims.y) / 2.0f;
}

void InventoryUI::DrawInventorySlots(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw slots
    int maxSlots = mInventory->GetMaxSlots();
    for (int i = 0; i < maxSlots; ++i)
    {
        Vector2 slotPos = GetSlotPosition(i);
        
        // Only draw selection/hover highlight if using map
        // The map already has slot backgrounds
        bool isSelected = (i == mSelectedSlot) || (i == mCraftInputSlot1) || (i == mCraftInputSlot2);
        
        if (isSelected)
        {
            if (mBackgroundMap && mSelectionCursorGID != 0 && mGame && mGame->GetSpriteRenderer())
            {
                // Draw selection cursor from map
                mBackgroundMap->DrawGID(mGame->GetSpriteRenderer(), mSelectionCursorGID, slotPos, mMapScale);
            }
        }
        else if (i == mHoveredSlot)
        {
             rectRenderer->RenderRect(
                slotPos.x,
                slotPos.y,
                16.0f * mMapScale,
                16.0f * mMapScale,
                mSlotHoverColor,
                0.3f // Semi-transparent
            );
        }

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
    float emojiScale = 0.8f;
    Vector2 emojiSize = textRenderer->MeasureText(slot->item.emoji, emojiScale);
    float emojiX = slotPos.x + (mSlotSize - emojiSize.x) / 2.0f;
    float emojiY = slotPos.y + (mSlotSize / 2.0f) + (emojiSize.y / 2.0f) - 5.0f; // Slight offset adjustment
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
        
        textRenderer->RenderText(slot->item.name, nameX, nameY, nameScale);
    }
}

Vector2 InventoryUI::GetSlotPosition(int slotIndex) const
{
    // Map layout:
    // Inventory slots start at (1, 1) in tiles (0-indexed)
    // 3 columns, 4 rows
    // Tile size 16, Scale 4.0 -> 64 pixels per tile
    
    int row = slotIndex / 3; // 3 slots per row in map
    int col = slotIndex % 3;

    // Offset (1, 1) tiles
    float startX = mPosition.x + (1 * 16.0f * mMapScale);
    float startY = mPosition.y + (1 * 16.0f * mMapScale);

    float x = startX + col * (16.0f * mMapScale);
    float y = startY + row * (16.0f * mMapScale);

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

void InventoryUI::AttemptCombination(int slotIndex1, int slotIndex2)
{
    if (!mGame || !mInventory) return;
    
    InventorySlot* slot1 = mInventory->GetSlot(slotIndex1);
    InventorySlot* slot2 = mInventory->GetSlot(slotIndex2);

    if (!slot1 || !slot2) return;

    Crafting* crafting = mGame->GetCrafting();
    if (!crafting) return;

    // Check if combination is valid
    std::unique_ptr<Item> result = crafting->combine_items(slot1->item, slot2->item);

    if (result)
    {
        bool success = false;
        
        if (slotIndex1 == slotIndex2)
        {
            // Same slot (stack) - need at least 2 items
            if (slot1->quantity >= 2)
            {
                mInventory->RemoveItemAt(slotIndex1, 2);
                success = true;
            }
        }
        else
        {
            // Different slots
            // Remove from higher index first to avoid shifting issues for the lower index
            int first = std::max(slotIndex1, slotIndex2);
            int second = std::min(slotIndex1, slotIndex2);
            
            // We need to check if removal is possible before removing
            // But RemoveItemAt checks quantity.
            
            if (mInventory->RemoveItemAt(first, 1))
            {
                if (mInventory->RemoveItemAt(second, 1))
                {
                    success = true;
                }
                else
                {
                    // This shouldn't happen if logic is correct and single threaded
                    // But if it does, we are in inconsistent state.
                }
            }
        }
        
        if (success)
        {
            mInventory->AddItem(*result, 1);
            SDL_Log("Combined items to create: %s", result->name.c_str());
        }
    }
    else
    {
        SDL_Log("Invalid combination");
    }
}

void InventoryUI::DrawCraftingPanel(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Calculate positions
    Vector2 input1Pos = GetCraftingSlotPosition(0);
    Vector2 input2Pos = GetCraftingSlotPosition(1);
    Vector2 resultPos = GetCraftingSlotPosition(2);
    float resultSize = mSlotSize; // Use standard slot size for result too, or adjust if map has larger slot

    // Draw Input 1 Item
    if (mCraftInputSlot1 != -1)
    {
        DrawItemInSlot(mCraftInputSlot1, input1Pos, textRenderer, rectRenderer);
    }

    // Draw Input 2 Item
    if (mCraftInputSlot2 != -1)
    {
        DrawItemInSlot(mCraftInputSlot2, input2Pos, textRenderer, rectRenderer);
    }

    // Draw Result Item if available
    if (mCraftResult)
    {
        // Manually draw item since it's not in inventory
        float emojiScale = 1.2f;
        Vector2 emojiSize = textRenderer->MeasureText(mCraftResult->emoji, emojiScale);
        float emojiX = resultPos.x + (resultSize - emojiSize.x) / 2.0f;
        float emojiY = resultPos.y + (resultSize / 2.0f) + (emojiSize.y / 2.0f) - 5.0f;
        textRenderer->RenderText(mCraftResult->emoji, emojiX, emojiY, emojiScale);
        
        // Draw name below result
        float nameScale = 0.6f;
        Vector2 nameSize = textRenderer->MeasureText(mCraftResult->name, nameScale);
        float nameX = resultPos.x + (resultSize - nameSize.x) / 2.0f;
        float nameY = resultPos.y - 20.0f;
        
        // Draw name background
        rectRenderer->RenderRect(
            nameX - 5.0f,
            nameY - nameSize.y - 2.0f,
            nameSize.x + 10.0f,
            nameSize.y + 4.0f,
            Vector3(0.1f, 0.1f, 0.15f),
            0.95f
        );

        textRenderer->RenderText(mCraftResult->name, nameX, nameY, nameScale);
    }
}

void InventoryUI::UpdateCraftingResult()
{
    mCraftResult.reset();

    if (mCraftInputSlot1 != -1 && mCraftInputSlot2 != -1)
    {
        InventorySlot* slot1 = mInventory->GetSlot(mCraftInputSlot1);
        InventorySlot* slot2 = mInventory->GetSlot(mCraftInputSlot2);

        if (slot1 && slot2 && mGame && mGame->GetCrafting())
        {
            mCraftResult = mGame->GetCrafting()->combine_items(slot1->item, slot2->item);
        }
    }
}

void InventoryUI::PerformCraft()
{
    if (mCraftResult && mCraftInputSlot1 != -1 && mCraftInputSlot2 != -1)
    {
        // Remove items
        bool removed = false;
        if (mCraftInputSlot1 == mCraftInputSlot2)
        {
            // Same slot
            removed = mInventory->RemoveItemAt(mCraftInputSlot1, 2);
        }
        else
        {
            // Different slots - remove higher index first
            int first = std::max(mCraftInputSlot1, mCraftInputSlot2);
            int second = std::min(mCraftInputSlot1, mCraftInputSlot2);
            if (mInventory->RemoveItemAt(first, 1))
            {
                removed = mInventory->RemoveItemAt(second, 1);
            }
        }

        if (removed)
        {
            mInventory->AddItem(*mCraftResult, 1);
            ClearCraftingSlots();
        }
    }
}

void InventoryUI::ClearCraftingSlots()
{
    mCraftInputSlot1 = -1;
    mCraftInputSlot2 = -1;
    mCraftResult.reset();
}

Vector2 InventoryUI::GetCraftingSlotPosition(int slotIndex) const
{
    // Map layout:
    // Input 1: (8, 1) tiles
    // Input 2: (10, 1) tiles
    // Result: (10, 4) tiles? Or maybe (9, 4)?
    // Let's assume inputs are top row, result is bottom row
    
    // Tile size 16, Scale 4.0
    float tileSize = 16.0f * mMapScale;

    if (slotIndex == 0) // Input 1
    {
        // (8, 1)
        return Vector2(mPosition.x + 8 * tileSize, mPosition.y + 1 * tileSize);
    }
    else if (slotIndex == 1) // Input 2
    {
        // (10, 1)
        return Vector2(mPosition.x + 10 * tileSize, mPosition.y + 1 * tileSize);
    }
    else // Result
    {
        // (10, 4) - Based on visual inspection of map data (row 4 has tile at col 10)
        return Vector2(mPosition.x + 10 * tileSize, mPosition.y + 4 * tileSize);
    }
}

bool InventoryUI::IsPointInRect(const Vector2& point, const Vector2& rectPos, const Vector2& rectSize) const
{
    return point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x &&
           point.y >= rectPos.y && point.y <= rectPos.y + rectSize.y;
}
