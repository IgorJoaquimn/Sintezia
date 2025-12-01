#pragma once
#include "../Game/Inventory.hpp"
#include "../MathUtils.h"
#include "../Core/Texture/Texture.hpp"
#include <functional>
#include <memory>

// Forward declarations
class Game;
class TextRenderer;
class RectRenderer;
class SpriteRenderer;
class TileMap;

class InventoryUI
{
public:
    InventoryUI(Game* game, Inventory* inventory);
    ~InventoryUI();

    // UI control
    void Show();
    void Hide();
    void Toggle();
    bool IsVisible() const { return mVisible; }

    // Update and render
    void Update(float deltaTime);
    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);

    // Input handling
    void HandleInput(const uint8_t* keyState);
    void HandleMouseClick(const Vector2& mousePos);
    void HandleRightClick(const Vector2& mousePos);
    void HandleMouseMove(const Vector2& mousePos);

    // UI configuration
    void SetPosition(const Vector2& position) { mPosition = position; }
    void SetSlotSize(float size) { mSlotSize = size; }
    void SetSlotsPerRow(int count) { mSlotsPerRow = count; }
    void SetPadding(float padding) { mPadding = padding; }
    
    // Layout helpers
    Vector2 GetDimensions() const;
    void CenterOnScreen(float screenWidth, float screenHeight);

    // Callbacks
    void SetOnItemSelected(std::function<void(int itemId)> callback) { mOnItemSelected = callback; }
    void SetOnItemUsed(std::function<void(const Item& item)> callback) { mOnItemUsed = callback; }

private:
    Game* mGame;
    Inventory* mInventory;
    bool mVisible;

    // UI layout
    Vector2 mPosition;
    float mSlotSize;
    int mSlotsPerRow;
    float mPadding;
    int mSelectedSlot;
    int mHoveredSlot;

    // UI colors
    Vector3 mBackgroundColor;
    Vector3 mSlotColor;
    Vector3 mSlotHoverColor;
    Vector3 mSlotSelectedColor;
    Vector3 mTextColor;

    // Input state
    bool mKeyPressed[10];
    Vector2 mCurrentMousePos;

    // Crafting state
    int mCraftInputSlot1; // Index in inventory
    int mCraftInputSlot2; // Index in inventory
    std::unique_ptr<Item> mCraftResult;

    // Crafting UI helpers
    void DrawCraftingPanel(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);
    void UpdateCraftingResult();
    void PerformCraft();
    void ClearCraftingSlots();
    Vector2 GetCraftingSlotPosition(int slotIndex) const; // 0=Input1, 1=Input2, 2=Result
    bool IsPointInRect(const Vector2& point, const Vector2& rectPos, const Vector2& rectSize) const;

    // Callbacks
    std::function<void(int itemId)> mOnItemSelected;
    std::function<void(const Item& item)> mOnItemUsed;

    // Tiled Map Background
    std::unique_ptr<TileMap> mBackgroundMap;
    float mMapScale;
    int mSelectionCursorGID;

    // Icons
    std::shared_ptr<Texture> mMouseRightIcon;
    
    // Helper methods
    void DrawInventorySlots(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);
    void DrawItemInSlot(int slotIndex, const Vector2& slotPos, TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);
    void DrawConsumeCursor(TextRenderer* textRenderer, RectRenderer* rectRenderer, SpriteRenderer* spriteRenderer);
    Vector2 GetSlotPosition(int slotIndex) const;
    int GetSlotAtPosition(const Vector2& mousePos) const;
    void UpdateKeyState(const uint8_t* keyState);
    void AttemptCombination(int slotIndex1, int slotIndex2);
};
