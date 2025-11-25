#pragma once
#include "../Crafting/Item.hpp"
#include <vector>
#include <map>
#include <memory>

// Represents a single slot in the inventory with an item and quantity
struct InventorySlot
{
    Item item;
    int quantity;

    InventorySlot(const Item& item, int quantity = 1)
        : item(item), quantity(quantity) {}
};

class Inventory
{
public:
    Inventory(int maxSlots = 20);
    ~Inventory();

    // Add/Remove items
    bool AddItem(const Item& item, int quantity = 1);
    bool RemoveItem(int itemId, int quantity = 1);
    bool RemoveItemAt(int slotIndex, int quantity = 1);
    
    // Query items
    bool HasItem(int itemId, int minQuantity = 1) const;
    int GetItemQuantity(int itemId) const;
    const InventorySlot* GetSlot(int index) const;
    InventorySlot* GetSlot(int index);
    int FindItemSlot(int itemId) const;
    
    // Inventory management
    void Clear();
    bool IsFull() const;
    int GetUsedSlots() const { return mSlots.size(); }
    int GetMaxSlots() const { return mMaxSlots; }
    
    // Get all slots (for UI rendering)
    const std::vector<InventorySlot>& GetAllSlots() const { return mSlots; }

private:
    std::vector<InventorySlot> mSlots;  // Dynamic list of filled slots
    int mMaxSlots;                       // Maximum number of slots
    
    // Helper to find an existing slot for an item
    int FindSlotIndex(int itemId) const;
};
