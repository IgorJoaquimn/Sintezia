#pragma once
#include "NPC.hpp"
#include "../../../MathUtils.h"
#include "../../../UI/NPCDialogUI.hpp"
#include <SDL.h>
#include <string>
#include <vector>
#include <memory>

// Forward declarations
class Item;
class InteractionIndicator;

// Represents a dialog option that the player can choose
struct DialogOption
{
    std::string text;           // Text displayed to the player (e.g., "Ask about the village")
    std::string npcResponse;    // NPC's response when this option is selected

    DialogOption(const std::string& text, const std::string& response)
        : text(text), npcResponse(response) {}
};

// Represents an item requirement for a trade (item + quantity)
struct ItemRequirement
{
    int itemId;
    int quantity;

    ItemRequirement(int id, int qty) : itemId(id), quantity(qty) {}
};

// Represents a complete trade offer
struct TradeOffer
{
    std::string description;                    // Description of the trade
    ItemRequirement reward;                     // What the NPC gives
    std::vector<ItemRequirement> requirements;  // What the player must provide

    TradeOffer(const std::string& desc, int rewardId, int rewardQty)
        : description(desc), reward(rewardId, rewardQty) {}

    void AddRequirement(int itemId, int quantity)
    {
        requirements.emplace_back(itemId, quantity);
    }
};


class DialogNPC : public NPC
{
public:
    DialogNPC(class Game* game);
    virtual ~DialogNPC();

    void OnUpdate(float deltaTime) override;
    void OnDraw(class TextRenderer* textRenderer) override;

    // Interaction methods
    bool CanInteract(const Vector2& playerPos, float interactionRange = 100.0f) const;
    void StartInteraction();
    void EndInteraction();
    bool IsInteracting() const;

    // Handle input during interaction (should be called from Game)
    void HandleInteractionInput(const uint8_t* keyState);

    // Show/hide interaction indicator
    void ShowInteractionIndicator(const Vector2& playerPos);
    void HideInteractionIndicator();

    // Configuration methods
    void SetGreeting(const std::string& greeting) { mGreeting = greeting; }
    void AddDialogOption(const std::string& text, const std::string& response);
    void AddTradeOffer(const TradeOffer& offer);

protected:
    // NPC data
    std::string mGreeting;
    std::vector<DialogOption> mDialogOptions;
    std::vector<TradeOffer> mTradeOffers;

    // UI Components
    std::unique_ptr<NPCDialogUI> mDialogUI;
    std::unique_ptr<InteractionIndicator> mInteractionIndicator;

    // Input handling helpers
    bool mKeyPressed[10];  // Track key states to prevent repeated input
    void UpdateKeyState(const uint8_t* keyState);

    // UI callback handlers
    void OnTalkSelected();
    void OnTradeMenuSelected();
    void OnLeaveSelected();
    void OnDialogOptionSelected(int index);
    void OnTradeOptionSelected(int index);
};

