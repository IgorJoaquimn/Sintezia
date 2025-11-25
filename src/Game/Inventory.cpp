#include "Inventory.hpp"
#include <algorithm>

Inventory::Inventory(int maxSlots)
    : mMaxSlots(maxSlots)
{
    mSlots.reserve(maxSlots);
}

Inventory::~Inventory()
{
}

bool Inventory::AddItem(const Item& item, int quantity)
{
    if (quantity <= 0) return false;

    // Check if item already exists in inventory
    int existingSlot = FindSlotIndex(item.id);
    
    if (existingSlot != -1)
    {
        // Add to existing stack
        mSlots[existingSlot].quantity += quantity;
        return true;
    }
    else
    {
        // Check if we have space for a new slot
        if (IsFull())
        {
            return false;  // Inventory full
        }
        
        // Create new slot
        mSlots.emplace_back(item, quantity);
        return true;
    }
}

bool Inventory::RemoveItem(int itemId, int quantity)
{
    if (quantity <= 0) return false;

    int slotIndex = FindSlotIndex(itemId);
    if (slotIndex == -1)
    {
        return false;  // Item not found
    }

    return RemoveItemAt(slotIndex, quantity);
}

bool Inventory::RemoveItemAt(int slotIndex, int quantity)
{
    if (slotIndex < 0 || slotIndex >= static_cast<int>(mSlots.size()))
    {
        return false;  // Invalid slot
    }

    if (quantity <= 0) return false;

    InventorySlot& slot = mSlots[slotIndex];
    
    if (slot.quantity < quantity)
    {
        return false;  // Not enough items
    }

    slot.quantity -= quantity;

    // Remove slot if quantity reaches zero
    if (slot.quantity <= 0)
    {
        mSlots.erase(mSlots.begin() + slotIndex);
    }

    return true;
}

bool Inventory::HasItem(int itemId, int minQuantity) const
{
    return GetItemQuantity(itemId) >= minQuantity;
}

int Inventory::GetItemQuantity(int itemId) const
{
    int slotIndex = FindSlotIndex(itemId);
    if (slotIndex == -1)
    {
        return 0;
    }
    return mSlots[slotIndex].quantity;
}

const InventorySlot* Inventory::GetSlot(int index) const
{
    if (index < 0 || index >= static_cast<int>(mSlots.size()))
    {
        return nullptr;
    }
    return &mSlots[index];
}

InventorySlot* Inventory::GetSlot(int index)
{
    if (index < 0 || index >= static_cast<int>(mSlots.size()))
    {
        return nullptr;
    }
    return &mSlots[index];
}

int Inventory::FindItemSlot(int itemId) const
{
    return FindSlotIndex(itemId);
}

void Inventory::Clear()
{
    mSlots.clear();
}

bool Inventory::IsFull() const
{
    return static_cast<int>(mSlots.size()) >= mMaxSlots;
}

int Inventory::FindSlotIndex(int itemId) const
{
    for (size_t i = 0; i < mSlots.size(); ++i)
    {
        if (mSlots[i].item.id == itemId)
        {
            return static_cast<int>(i);
        }
    }
    return -1;  // Not found
}
