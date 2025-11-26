#include "DialogNPC.hpp"
#include "../../../Game/Game.hpp"
#include "../../../Map/TiledParser.hpp"
#include "../../../Core/TextRenderer/TextRenderer.hpp"
#include "../../../Core/RectRenderer/RectRenderer.hpp"
#include "../../../Core/Texture/SpriteRenderer.hpp"
#include "../../../Component/AnimationComponent.hpp"
#include "../../../Component/SpriteComponent.hpp"
#include "../../../Crafting/Crafting.hpp"
#include "../../../Crafting/Item.hpp"
#include "../../../Actor/Player.hpp"
#include "../../../Game/Inventory.hpp"
#include <cmath>
#include <algorithm>
#include <SDL.h>
#include <GL/glew.h>

DialogNPC::DialogNPC(Game* game)
    : NPC(game)
    , mGreeting("Hello!")
    , mDialogUI(std::make_unique<NPCDialogUI>(game))
    , mInteractionIndicator(std::make_unique<InteractionIndicator>(game))
{
    // Initialize key states
    for (int i = 0; i < 10; i++)
    {
        mKeyPressed[i] = false;
    }

    // Setup UI callbacks
    mDialogUI->SetOnTalkSelected([this]() { OnTalkSelected(); });
    mDialogUI->SetOnTradeMenuSelected([this]() { OnTradeMenuSelected(); });
    mDialogUI->SetOnLeaveSelected([this]() { OnLeaveSelected(); });
    mDialogUI->SetOnDialogSelected([this](int index) { OnDialogOptionSelected(index); });
    mDialogUI->SetOnTradeSelected([this](int index) { OnTradeOptionSelected(index); });
}

DialogNPC::~DialogNPC()
{
}

void DialogNPC::OnUpdate(float deltaTime)
{
    // NPCs are currently stationary, but this can be extended for moving NPCs
}

void DialogNPC::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    auto* rectRenderer = mGame->GetRectRenderer();
    if (!spriteRenderer || !mSpriteComponent || !mAnimationComponent) return;

    // For stationary NPCs, just show idle frame
    // Row 0, Column 0 = idle facing down
    int row = 0;
    int col = 0;  // Fixed at frame 0 for stationary idle

    // Configure sprite component for rendering
    mSpriteComponent->SetCurrentFrame(row, col);
    mSpriteComponent->SetFlipHorizontal(false);

    // Draw the sprite
    mSpriteComponent->Draw(spriteRenderer);

    // Draw interaction indicator if visible
    if (mInteractionIndicator)
    {
        mInteractionIndicator->Draw(textRenderer, rectRenderer);

        // Reset OpenGL state after UI rendering to prevent affecting subsequent sprites
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Draw dialog UI if active
    if (mDialogUI && mDialogUI->IsVisible())
    {
        mDialogUI->Draw(textRenderer, rectRenderer);

        // Reset OpenGL state after UI rendering
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

bool DialogNPC::CanInteract(const Vector2& playerPos, float interactionRange) const
{
    float distance = (GetPosition() - playerPos).Length();
    return distance <= interactionRange;
}

void DialogNPC::StartInteraction()
{
    if (!IsInteracting())
    {
        mDialogUI->ShowGreeting(mGreeting);
        HideInteractionIndicator();
    }
}

void DialogNPC::EndInteraction()
{
    if (mDialogUI)
    {
        mDialogUI->Hide();
    }
}

bool DialogNPC::IsInteracting() const
{
    return mDialogUI && mDialogUI->IsVisible();
}

void DialogNPC::ShowInteractionIndicator(const Vector2& playerPos)
{
    if (mInteractionIndicator && !IsInteracting())
    {
        mInteractionIndicator->Show(GetPosition());
    }
}

void DialogNPC::HideInteractionIndicator()
{
    if (mInteractionIndicator)
    {
        mInteractionIndicator->Hide();
    }
}

void DialogNPC::HandleInteractionInput(const uint8_t* keyState)
{
    if (!IsInteracting() || !mDialogUI) return;

    UpdateKeyState(keyState);

    // Handle navigation - W/A for up, S/D for down
    if (!mKeyPressed[1] && (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_A]))
    {
        mKeyPressed[1] = true;
        mDialogUI->NavigateUp();
    }
    else if (!mKeyPressed[2] && (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_D]))
    {
        mKeyPressed[2] = true;
        mDialogUI->NavigateDown();
    }

    // Handle selection - SPACE or ENTER
    if (!mKeyPressed[3] && (keyState[SDL_SCANCODE_SPACE] || keyState[SDL_SCANCODE_RETURN]))
    {
        mKeyPressed[3] = true;
        mDialogUI->SelectCurrent();
    }

    // Handle back with Q key - only works in submenus (not main menu)
    if (!mKeyPressed[4] && keyState[SDL_SCANCODE_Q])
    {
        mKeyPressed[4] = true;

        // Q only works in dialog/trade menus, goes back to main menu
        if (mDialogUI->GetState() == DialogUIState::DialogMenu ||
            mDialogUI->GetState() == DialogUIState::TradeMenu ||
            mDialogUI->GetState() == DialogUIState::Message)
        {
            mDialogUI->ShowMainMenu();
        }
        // In main menu or greeting, Q does nothing
    }

    // Handle ESC - always closes the entire dialog
    if (!mKeyPressed[5] && keyState[SDL_SCANCODE_ESCAPE])
    {
        mKeyPressed[5] = true;
        EndInteraction();
    }
}

void DialogNPC::UpdateKeyState(const uint8_t* keyState)
{
    // Reset key states when keys are released
    if (!keyState[SDL_SCANCODE_SPACE] && !keyState[SDL_SCANCODE_RETURN])
        mKeyPressed[0] = false;
    if (!keyState[SDL_SCANCODE_W] && !keyState[SDL_SCANCODE_A])
        mKeyPressed[1] = false;
    if (!keyState[SDL_SCANCODE_S] && !keyState[SDL_SCANCODE_D])
        mKeyPressed[2] = false;
    if (!keyState[SDL_SCANCODE_RETURN] && !keyState[SDL_SCANCODE_SPACE])
        mKeyPressed[3] = false;
    if (!keyState[SDL_SCANCODE_Q])
        mKeyPressed[4] = false;
    if (!keyState[SDL_SCANCODE_ESCAPE])
        mKeyPressed[5] = false;
}

void DialogNPC::AddDialogOption(const std::string& text, const std::string& response)
{
    mDialogOptions.emplace_back(text, response);
}

void DialogNPC::AddTradeOffer(const TradeOffer& offer)
{
    mTradeOffers.push_back(offer);
}


// UI Callback Handlers
void DialogNPC::OnTalkSelected()
{
    if (!mDialogOptions.empty())
    {
        std::vector<std::string> options;
        for (const auto& opt : mDialogOptions)
        {
            options.push_back(opt.text);
        }
        mDialogUI->ShowDialogMenu(options);
    }
    else
    {
        mDialogUI->ShowMessage("I don't have much to say right now.");
    }
}

void DialogNPC::OnTradeMenuSelected()
{
    if (!mTradeOffers.empty())
    {
        std::vector<std::string> tradeDescs;
        auto* crafting = mGame->GetCrafting();

        for (const auto& trade : mTradeOffers)
        {
            std::string desc = trade.description + "\n";

            // Add reward info
            const Item* rewardItem = crafting ? crafting->FindItemById(trade.reward.itemId) : nullptr;
            desc += "  Get: ";
            if (rewardItem)
            {
                desc += rewardItem->emoji + " " + rewardItem->name + " x" +
                       std::to_string(trade.reward.quantity);
            }
            else
            {
                desc += "Item #" + std::to_string(trade.reward.itemId) + " x" +
                       std::to_string(trade.reward.quantity);
            }

            // Add requirements info
            if (!trade.requirements.empty())
            {
                desc += "\n  For: ";
                for (size_t i = 0; i < trade.requirements.size(); i++)
                {
                    const Item* reqItem = crafting ? crafting->FindItemById(trade.requirements[i].itemId) : nullptr;

                    if (reqItem)
                    {
                        desc += reqItem->emoji + " " + reqItem->name + " x" +
                               std::to_string(trade.requirements[i].quantity);
                    }
                    else
                    {
                        desc += "Item #" + std::to_string(trade.requirements[i].itemId) + " x" +
                               std::to_string(trade.requirements[i].quantity);
                    }

                    // Add comma if not the last item
                    if (i < trade.requirements.size() - 1)
                    {
                        desc += ", ";
                    }
                }
            }

            tradeDescs.push_back(desc);
        }
        mDialogUI->ShowTradeMenu(tradeDescs);
    }
    else
    {
        mDialogUI->ShowMessage("I don't have anything to trade right now.");
    }
}

void DialogNPC::OnLeaveSelected()
{
    EndInteraction();
}

void DialogNPC::OnDialogOptionSelected(int index)
{
    if (index >= 0 && index < static_cast<int>(mDialogOptions.size()))
    {
        mDialogUI->ShowMessage(mDialogOptions[index].npcResponse);
    }
}

void DialogNPC::OnTradeOptionSelected(int index)
{
    if (index >= 0 && index < static_cast<int>(mTradeOffers.size()))
    {
        const TradeOffer& trade = mTradeOffers[index];
        
        // Get player and inventory
        Player* player = mGame->GetPlayer();
        if (!player)
        {
            mDialogUI->ShowMessage("Error: Player not found!");
            return;
        }
        
        Inventory* inventory = player->GetInventory();
        if (!inventory)
        {
            mDialogUI->ShowMessage("Error: Inventory not found!");
            return;
        }
        
        // Check if player has all required items
        bool hasAllItems = true;
        std::string missingItems;
        auto* crafting = mGame->GetCrafting();
        
        for (const auto& req : trade.requirements)
        {
            if (!inventory->HasItem(req.itemId, req.quantity))
            {
                hasAllItems = false;
                const Item* item = crafting ? crafting->FindItemById(req.itemId) : nullptr;
                
                if (!missingItems.empty())
                    missingItems += ", ";
                    
                if (item)
                {
                    missingItems += item->emoji + " " + item->name + " x" + std::to_string(req.quantity);
                }
                else
                {
                    missingItems += "Item #" + std::to_string(req.itemId) + " x" + std::to_string(req.quantity);
                }
            }
        }
        
        // If player doesn't have all items, show error message
        if (!hasAllItems)
        {
            mDialogUI->ShowMessage("You don't have the required items!\nMissing: " + missingItems);
            return;
        }
        
        // Remove required items from inventory
        for (const auto& req : trade.requirements)
        {
            inventory->RemoveItem(req.itemId, req.quantity);
        }
        
        // Add reward item to inventory
        const Item* rewardItem = crafting ? crafting->FindItemById(trade.reward.itemId) : nullptr;
        if (rewardItem)
        {
            bool success = inventory->AddItem(*rewardItem, trade.reward.quantity);
            
            if (success)
            {
                std::string successMsg = "Trade successful!\nYou received: " +
                                        rewardItem->emoji + " " + rewardItem->name +
                                        " x" + std::to_string(trade.reward.quantity);
                mDialogUI->ShowMessage(successMsg);
            }
            else
            {
                // Inventory full - refund the items
                for (const auto& req : trade.requirements)
                {
                    const Item* reqItem = crafting->FindItemById(req.itemId);
                    if (reqItem)
                    {
                        inventory->AddItem(*reqItem, req.quantity);
                    }
                }
                mDialogUI->ShowMessage("Your inventory is full! Trade cancelled.");
            }
        }
        else
        {
            // Invalid reward item - refund
            for (const auto& req : trade.requirements)
            {
                const Item* reqItem = crafting->FindItemById(req.itemId);
                if (reqItem)
                {
                    inventory->AddItem(*reqItem, req.quantity);
                }
            }
            mDialogUI->ShowMessage("Error: Invalid reward item! Trade cancelled.");
        }
    }
}











