#include "InventoryUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Map/TileMap.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Actor/ItemActor.hpp"
#include "../Actor/Player.hpp"
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
    , mCraftInputSlot1(nullptr)
    , mCraftInputSlot2(nullptr)
    , mCraftResult(nullptr)
    , mBackgroundMap(nullptr)
    , mMapScale(4.0f)
    , mSelectionCursorGID(0)
    , mMouseRightIcon(nullptr)
    , mCurrentMousePos(Vector2::Zero)
    , mIsDragging(false)
    , mDraggedSlotIndex(-1)
    , mDragStartPos(Vector2::Zero)
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

    // Load mouse icon
    mMouseRightIcon = std::make_shared<Texture>();
    if (!mMouseRightIcon->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Input/Mouse/MouseButtonRight.png"))
    {
        SDL_Log("Failed to load mouse right icon");
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
    SDL_ShowCursor(SDL_ENABLE);
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

    // Ensure text color is set to the inventory's text color (white)
    if (textRenderer)
    {
        textRenderer->SetTextColor(mTextColor.x, mTextColor.y, mTextColor.z);
    }

    // Draw background map
    if (mBackgroundMap && spriteRenderer)
    {
        mBackgroundMap->Draw(spriteRenderer, mPosition, mMapScale);
    }

    DrawInventorySlots(textRenderer, rectRenderer, spriteRenderer);
    DrawCraftingPanel(textRenderer, rectRenderer, spriteRenderer);
    DrawConsumeCursor(textRenderer, rectRenderer, spriteRenderer);

    // Draw dragged item
    if (mIsDragging && mDraggedSlotIndex != -1 && mInventory)
    {
        const InventorySlot* slot = mInventory->GetSlot(mDraggedSlotIndex);
        if (slot)
        {
            // Draw item at mouse position
            float emojiScale = 0.8f;
            Vector2 emojiSize = textRenderer->MeasureText(slot->item.emoji, emojiScale);
            float emojiX = mCurrentMousePos.x - emojiSize.x / 2.0f;
            float emojiY = mCurrentMousePos.y - emojiSize.y / 2.0f;
            textRenderer->RenderText(slot->item.emoji, emojiX, emojiY, emojiScale);
        }
    }
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
    float resultSize = mSlotSize * 1.5f;

    // Check Result Slot (Immediate action)
    if (IsPointInRect(mousePos, resultPos, Vector2(resultSize, resultSize)))
    {
        PerformCraft();
        return;
    }

    // Check Input Slots (Start Drag)
    // We use special indices for crafting slots: -100 for Input1, -101 for Input2
    Vector2 input1Pos = GetCraftingSlotPosition(0);
    if (IsPointInRect(mousePos, input1Pos, Vector2(mSlotSize, mSlotSize)))
    {
        if (mCraftInputSlot1)
        {
            mIsDragging = true;
            mDraggedSlotIndex = -100; // Special index for Input 1
            mDragStartPos = mousePos;
        }
        return;
    }

    Vector2 input2Pos = GetCraftingSlotPosition(1);
    if (IsPointInRect(mousePos, input2Pos, Vector2(mSlotSize, mSlotSize)))
    {
        if (mCraftInputSlot2)
        {
            mIsDragging = true;
            mDraggedSlotIndex = -101; // Special index for Input 2
            mDragStartPos = mousePos;
        }
        return;
    }

    // Check Inventory Slots (Start Drag)
    int clickedSlot = GetSlotAtPosition(mousePos);
    
    if (clickedSlot != -1 && clickedSlot < mInventory->GetUsedSlots())
    {
        mIsDragging = true;
        mDraggedSlotIndex = clickedSlot;
        mDragStartPos = mousePos;
    }
}

void InventoryUI::HandleMouseUp(const Vector2& mousePos)
{
    if (!mIsDragging) return;

    mIsDragging = false;
    float dragDist = (mousePos - mDragStartPos).Length();
    bool isClick = dragDist < 5.0f;

    // Identify source item
    InventorySlot* sourceSlot = nullptr;
    if (mDraggedSlotIndex >= 0)
    {
        sourceSlot = mInventory->GetSlot(mDraggedSlotIndex);
    }
    else if (mDraggedSlotIndex == -100 && mCraftInputSlot1)
    {
        sourceSlot = mCraftInputSlot1.get();
    }
    else if (mDraggedSlotIndex == -101 && mCraftInputSlot2)
    {
        sourceSlot = mCraftInputSlot2.get();
    }

    if (!sourceSlot) return;

        // Helper lambda to move from inventory to crafting
    auto moveFromInventoryToCrafting = [&](int invSlotIdx, std::unique_ptr<InventorySlot>& craftSlot) {
        InventorySlot* s = mInventory->GetSlot(invSlotIdx);
        if (!s) return;
        
        // If craft slot has item, return it to inventory first
        if (craftSlot)
        {
            mInventory->AddItem(craftSlot->item, craftSlot->quantity);
            craftSlot.reset();
        }
        
        // Take 1 from inventory
        Item item = s->item;
        mInventory->RemoveItemAt(invSlotIdx, 1);
        
        // Put in crafting
        craftSlot = std::make_unique<InventorySlot>(item, 1);
        UpdateCraftingResult();
    };

    // Helper lambda to return from crafting to inventory
    auto returnToInventory = [&](std::unique_ptr<InventorySlot>& craftSlot) {
        if (!craftSlot) return;
        mInventory->AddItem(craftSlot->item, craftSlot->quantity);
        craftSlot.reset();
        UpdateCraftingResult();
    };

    // If Click (not drag)
    if (isClick)
    {
        if (mDraggedSlotIndex >= 0)
        {
            // Clicked Inventory Slot -> Move to Crafting
            if (!mCraftInputSlot1) moveFromInventoryToCrafting(mDraggedSlotIndex, mCraftInputSlot1);
            else if (!mCraftInputSlot2) moveFromInventoryToCrafting(mDraggedSlotIndex, mCraftInputSlot2);
            else {
                // Both full, replace 1
                moveFromInventoryToCrafting(mDraggedSlotIndex, mCraftInputSlot1);
            }
        }
        else if (mDraggedSlotIndex == -100)
        {
            // Clicked Input 1 -> Return to inventory
            returnToInventory(mCraftInputSlot1);
        }
        else if (mDraggedSlotIndex == -101)
        {
            // Clicked Input 2 -> Return to inventory
            returnToInventory(mCraftInputSlot2);
        }
        return;
    }

    // Check where it was dropped
    
    // Check Input 1
    Vector2 input1Pos = GetCraftingSlotPosition(0);
    if (IsPointInRect(mousePos, input1Pos, Vector2(mSlotSize, mSlotSize)))
    {
        if (mDraggedSlotIndex >= 0) // From Inventory
        {
            moveFromInventoryToCrafting(mDraggedSlotIndex, mCraftInputSlot1);
        }
        else if (mDraggedSlotIndex == -101) // From Input 2
        {
            if (mCraftInputSlot2)
            {
                if (mCraftInputSlot1) returnToInventory(mCraftInputSlot1); // Clear target
                mCraftInputSlot1 = std::move(mCraftInputSlot2); // Move
                UpdateCraftingResult();
            }
        }
        return;
    }

    // Check Input 2
    Vector2 input2Pos = GetCraftingSlotPosition(1);
    if (IsPointInRect(mousePos, input2Pos, Vector2(mSlotSize, mSlotSize)))
    {
        if (mDraggedSlotIndex >= 0) // From Inventory
        {
            moveFromInventoryToCrafting(mDraggedSlotIndex, mCraftInputSlot2);
        }
        else if (mDraggedSlotIndex == -100) // From Input 1
        {
             if (mCraftInputSlot1)
            {
                if (mCraftInputSlot2) returnToInventory(mCraftInputSlot2); // Clear target
                mCraftInputSlot2 = std::move(mCraftInputSlot1); // Move
                UpdateCraftingResult();
            }
        }
        return;
    }

    // Check Inventory Slots (Dropped back to inventory)
    bool droppedInInventory = false;
    if (mBackgroundMap)
    {
        Vector2 mapDims = GetDimensions();
        if (IsPointInRect(mousePos, mPosition, mapDims))
        {
            droppedInInventory = true;
        }
    }
    
    if (droppedInInventory)
    {
        if (mDraggedSlotIndex == -100) returnToInventory(mCraftInputSlot1);
        else if (mDraggedSlotIndex == -101) returnToInventory(mCraftInputSlot2);
        return;
    }

    // Dropped outside UI -> Drop to world
    if (!droppedInInventory)
    {
        // Get item data copy before removing
        Item itemToDrop = sourceSlot->item;
        
        // Remove from source
        if (mDraggedSlotIndex >= 0)
        {
            mInventory->RemoveItemAt(mDraggedSlotIndex, 1);
        }
        else if (mDraggedSlotIndex == -100)
        {
            mCraftInputSlot1.reset();
            UpdateCraftingResult();
        }
        else if (mDraggedSlotIndex == -101)
        {
            mCraftInputSlot2.reset();
            UpdateCraftingResult();
        }

        // Spawn Actor
        if (mGame)
        {
            auto actor = std::make_unique<ItemActor>(mGame, itemToDrop);
            
            // Drop near player
            if (mGame->GetPlayer())
            {
                float angle = (static_cast<float>(rand()) / RAND_MAX) * 6.283185f; // 2*PI
                float distance = 60.0f + (static_cast<float>(rand()) / RAND_MAX) * 40.0f; // 60-100 units
                
                float offsetX = std::cos(angle) * distance;
                float offsetY = std::sin(angle) * distance;
                actor->SetPosition(mGame->GetPlayer()->GetPosition() + Vector2(offsetX, offsetY));
            }
            
            mGame->AddActor(std::move(actor));
            SDL_Log("Dropped item: %s", itemToDrop.name.c_str());
        }
        return;
    }
}



void InventoryUI::HandleRightClick(const Vector2& mousePos)
{
    if (!mVisible) return;

    int clickedSlot = GetSlotAtPosition(mousePos);
    
    if (clickedSlot != -1 && clickedSlot < mInventory->GetUsedSlots())
    {
        const InventorySlot* slot = mInventory->GetSlot(clickedSlot);
        if (slot && mOnItemUsed)
        {
            mOnItemUsed(slot->item);
        }
    }
}

void InventoryUI::HandleMouseMove(const Vector2& mousePos)
{
    if (!mVisible) return;

    mCurrentMousePos = mousePos;
    mHoveredSlot = GetSlotAtPosition(mousePos);

    // Handle cursor visibility
    bool showCustomCursor = false;
    if (mHoveredSlot != -1 && mInventory)
    {
        const InventorySlot* slot = mInventory->GetSlot(mHoveredSlot);
        if (slot && (slot->item.hungerRestoration > 0.0f || slot->item.thirstRestoration > 0.0f))
        {
            showCustomCursor = true;
        }
    }

    if (showCustomCursor)
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
    else
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
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

void InventoryUI::DrawInventorySlots(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw slots
    int maxSlots = mInventory->GetMaxSlots();
    for (int i = 0; i < maxSlots; ++i)
    {
        Vector2 slotPos = GetSlotPosition(i);
        
        // Only draw selection/hover highlight if using map
        // The map already has slot backgrounds
        bool isSelected = (i == mSelectedSlot);
        
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
            DrawItemInSlot(i, slotPos, textRenderer, rectRenderer, spriteRenderer);
        }
    }
}

void InventoryUI::DrawItemInSlot(int slotIndex, const Vector2& slotPos, TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer)
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
    if (slot->quantity >= 1)
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

void InventoryUI::DrawConsumeCursor(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer)
{
    if (!mVisible || mHoveredSlot == -1 || !mInventory || !textRenderer || !rectRenderer || !spriteRenderer || !mMouseRightIcon) return;

    const InventorySlot* slot = mInventory->GetSlot(mHoveredSlot);
    if (!slot) return;

    if (slot->item.hungerRestoration > 0.0f || slot->item.thirstRestoration > 0.0f)
    {
        std::string hintText = "consumir";
        if (slot->item.hungerRestoration > 0.0f) hintText = "comer";
        else if (slot->item.thirstRestoration > 0.0f) hintText = "beber";

        float hintScale = 0.25f;
        Vector2 textSize = textRenderer->MeasureText(hintText, hintScale);
        
        float iconSize = 20.0f;
        float spacing = 8.0f;
        float totalWidth = iconSize + spacing + textSize.x;
        
        // Position relative to mouse cursor
        // Center the hint block on the mouse cursor, but slightly offset to not block the exact point
        float startX = mCurrentMousePos.x - totalWidth / 2.0f;
        float startY = mCurrentMousePos.y - 25.0f; 
        
        // Padding
        float padX = 8.0f;
        float padY = 6.0f;

        // Calculate height based on the tallest element
        float contentHeight = std::max(iconSize, textSize.y);

        // Draw background
        rectRenderer->RenderRect(
            startX - padX,
            startY - padY,
            totalWidth + padX * 2.0f,
            contentHeight + padY * 2.0f,
            Vector3(0.0f, 0.0f, 0.0f),
            0.8f
        );

        // Draw Icon (centered vertically relative to content height)
        float iconY = startY + (contentHeight - iconSize) / 2.0f;
        spriteRenderer->DrawSprite(mMouseRightIcon.get(), Vector2(startX, iconY), Vector2(iconSize, iconSize));
        
        // Draw Text (centered vertically relative to content height)
        float textY = startY + (contentHeight - textSize.y) / 2.0f;
        textRenderer->RenderText(hintText, startX + iconSize + spacing, textY, hintScale);
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

void InventoryUI::DrawCraftingPanel(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Calculate positions
    Vector2 input1Pos = GetCraftingSlotPosition(0);
    Vector2 input2Pos = GetCraftingSlotPosition(1);
    Vector2 resultPos = GetCraftingSlotPosition(2);
    float resultSize = mSlotSize; 

    // Helper to draw item
    auto drawItem = [&](const InventorySlot* slot, Vector2 pos) {
        if (!slot) return;
        float emojiScale = 0.8f;
        Vector2 emojiSize = textRenderer->MeasureText(slot->item.emoji, emojiScale);
        float emojiX = pos.x + (mSlotSize - emojiSize.x) / 2.0f;
        float emojiY = pos.y + (mSlotSize / 2.0f) + (emojiSize.y / 2.0f) - 5.0f;
        textRenderer->RenderText(slot->item.emoji, emojiX, emojiY, emojiScale);

        // Draw name above slot
        float nameScale = 0.4f;
        Vector2 nameSize = textRenderer->MeasureText(slot->item.name, nameScale);
        float nameX = pos.x + (mSlotSize - nameSize.x) / 2.0f;
        float nameY = pos.y - 2.0f;
        textRenderer->RenderText(slot->item.name, nameX, nameY, nameScale);
    };

    // Draw Input 1 Item
    if (mCraftInputSlot1)
    {
        drawItem(mCraftInputSlot1.get(), input1Pos);
    }

    // Draw Input 2 Item
    if (mCraftInputSlot2)
    {
        drawItem(mCraftInputSlot2.get(), input2Pos);
    }

    // Draw Result Item if available
    if (mCraftResult)
    {
        // Create temporary slot to use same drawing logic
        InventorySlot tempSlot(*mCraftResult, 1);
        drawItem(&tempSlot, resultPos);
    }
}

void InventoryUI::UpdateCraftingResult()
{
    mCraftResult.reset();

    if (mCraftInputSlot1 && mCraftInputSlot2)
    {
        if (mGame && mGame->GetCrafting())
        {
            mCraftResult = mGame->GetCrafting()->combine_items(mCraftInputSlot1->item, mCraftInputSlot2->item);
        }
    }
}

void InventoryUI::PerformCraft()
{
    if (mCraftResult && mCraftInputSlot1 && mCraftInputSlot2)
    {
        // Items are already removed from inventory when placed in crafting slots
        // Just consume them (clear slots) and add result
        mInventory->AddItem(*mCraftResult, 1);
        ClearCraftingSlots();
    }
}

void InventoryUI::ClearCraftingSlots()
{
    mCraftInputSlot1.reset();
    mCraftInputSlot2.reset();
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
        // (10, 3) - Based on visual inspection of map data (row 3 has tile at col 10)
        return Vector2(mPosition.x + 10 * tileSize, mPosition.y + 3 * tileSize);
    }
}

bool InventoryUI::IsPointInRect(const Vector2& point, const Vector2& rectPos, const Vector2& rectSize) const
{
    return point.x >= rectPos.x && point.x <= rectPos.x + rectSize.x &&
           point.y >= rectPos.y && point.y <= rectPos.y + rectSize.y;
}
