#pragma once
#include "../Game/Inventory.hpp"
#include "../MathUtils.h"
#include <functional>
#include <memory>

// Forward declarations
class Game;
class TextRenderer;
class RectRenderer;

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
    void Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer);

    // Input handling
    void HandleInput(const uint8_t* keyState);
    void HandleMouseClick(const Vector2& mousePos);
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
    void SetOnItemUsed(std::function<void(int itemId)> callback) { mOnItemUsed = callback; }

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

    // Helper methods
    void DrawInventoryBackground(RectRenderer* rectRenderer);
    void DrawInventorySlots(TextRenderer* textRenderer, RectRenderer* rectRenderer);
    void DrawItemInSlot(int slotIndex, const Vector2& slotPos, TextRenderer* textRenderer, RectRenderer* rectRenderer);
    Vector2 GetSlotPosition(int slotIndex) const;
    int GetSlotAtPosition(const Vector2& mousePos) const;
    void UpdateKeyState(const uint8_t* keyState);

    // Callbacks
    std::function<void(int itemId)> mOnItemSelected;
    std::function<void(int itemId)> mOnItemUsed;
};
