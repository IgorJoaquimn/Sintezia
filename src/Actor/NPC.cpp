#include "NPC.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/SpriteRenderer.hpp"
#include "../Component/AnimationComponent.hpp"
#include "../Component/SpriteComponent.hpp"
#include "../Crafting/Crafting.hpp"
#include "../Crafting/Item.hpp"
#include <cmath>
#include <algorithm>
#include <SDL.h>
#include <GL/glew.h>

NPC::NPC(Game* game)
    : Actor(game)
    , mGreeting("Hello!")
    , mDialogUI(std::make_unique<NPCDialogUI>(game))
    , mInteractionIndicator(std::make_unique<InteractionIndicator>(game))
    , mAnimationComponent(nullptr)
    , mSpriteComponent(nullptr)
    , mSpriteWidth(32)
    , mSpriteHeight(32)
    , mIdleFrames(6)
    , mWalkFrames(6)
    , mAnimSpeed(8.0f)
{
    // Initialize key states
    for (int i = 0; i < 10; i++)
    {
        mKeyPressed[i] = false;
    }

    // Create and add components
    mAnimationComponent = AddComponent<AnimationComponent>();
    mSpriteComponent = AddComponent<SpriteComponent>(200); // Higher update order for rendering

    // Configure animation component with default values
    mAnimationComponent->SetFrameCount(mIdleFrames);
    mAnimationComponent->SetAnimSpeed(mAnimSpeed);

    // Setup UI callbacks
    mDialogUI->SetOnTalkSelected([this]() { OnTalkSelected(); });
    mDialogUI->SetOnTradeMenuSelected([this]() { OnTradeMenuSelected(); });
    mDialogUI->SetOnLeaveSelected([this]() { OnLeaveSelected(); });
    mDialogUI->SetOnDialogSelected([this](int index) { OnDialogOptionSelected(index); });
    mDialogUI->SetOnTradeSelected([this](int index) { OnTradeOptionSelected(index); });
}

NPC::~NPC()
{
}

void NPC::OnUpdate(float deltaTime)
{
    // NPCs are currently stationary, but this can be extended for moving NPCs
}

void NPC::OnDraw(TextRenderer* textRenderer)
{
    auto* spriteRenderer = mGame->GetSpriteRenderer();
    auto* rectRenderer = mGame->GetRectRenderer();
    if (!spriteRenderer || !mSpriteComponent || !mAnimationComponent) return;

    // For now, NPCs are idle and facing down (row 0)
    int row = 0;  // Idle down animation
    int col = mAnimationComponent->GetCurrentFrame();

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

bool NPC::CanInteract(const Vector2& playerPos, float interactionRange) const
{
    float distance = (GetPosition() - playerPos).Length();
    return distance <= interactionRange;
}

void NPC::StartInteraction()
{
    if (!IsInteracting())
    {
        mDialogUI->ShowGreeting(mGreeting);
        HideInteractionIndicator();
    }
}

void NPC::EndInteraction()
{
    if (mDialogUI)
    {
        mDialogUI->Hide();
    }
}

bool NPC::IsInteracting() const
{
    return mDialogUI && mDialogUI->IsVisible();
}

void NPC::ShowInteractionIndicator(const Vector2& playerPos)
{
    if (mInteractionIndicator && !IsInteracting())
    {
        mInteractionIndicator->Show(GetPosition());
    }
}

void NPC::HideInteractionIndicator()
{
    if (mInteractionIndicator)
    {
        mInteractionIndicator->Hide();
    }
}

void NPC::HandleInteractionInput(const uint8_t* keyState)
{
    if (!IsInteracting() || !mDialogUI) return;

    UpdateKeyState(keyState);

    // Handle navigation
    if (!mKeyPressed[1] && keyState[SDL_SCANCODE_W])
    {
        mKeyPressed[1] = true;
        mDialogUI->NavigateUp();
    }
    else if (!mKeyPressed[2] && keyState[SDL_SCANCODE_S])
    {
        mKeyPressed[2] = true;
        mDialogUI->NavigateDown();
    }

    // Handle selection
    if (!mKeyPressed[3] && (keyState[SDL_SCANCODE_SPACE] || keyState[SDL_SCANCODE_RETURN]))
    {
        mKeyPressed[3] = true;
        mDialogUI->SelectCurrent();
    }

    // Handle back/cancel
    if (!mKeyPressed[4] && keyState[SDL_SCANCODE_ESCAPE])
    {
        mKeyPressed[4] = true;

        // If in main menu or greeting, close dialog
        if (mDialogUI->GetState() == DialogUIState::MainMenu ||
            mDialogUI->GetState() == DialogUIState::Greeting)
        {
            EndInteraction();
        }
        else
        {
            // Otherwise go back to main menu
            mDialogUI->ShowMainMenu();
        }
    }
}

void NPC::UpdateKeyState(const uint8_t* keyState)
{
    // Reset key states when keys are released
    if (!keyState[SDL_SCANCODE_SPACE] && !keyState[SDL_SCANCODE_RETURN])
        mKeyPressed[0] = false;
    if (!keyState[SDL_SCANCODE_W])
        mKeyPressed[1] = false;
    if (!keyState[SDL_SCANCODE_S])
        mKeyPressed[2] = false;
    if (!keyState[SDL_SCANCODE_RETURN] && !keyState[SDL_SCANCODE_SPACE])
        mKeyPressed[3] = false;
    if (!keyState[SDL_SCANCODE_ESCAPE])
        mKeyPressed[4] = false;
}

void NPC::AddDialogOption(const std::string& text, const std::string& response)
{
    mDialogOptions.emplace_back(text, response);
}

void NPC::AddTradeOffer(const TradeOffer& offer)
{
    mTradeOffers.push_back(offer);
}

void NPC::LoadSpriteSheet(const std::string& filepath)
{
    if (mSpriteComponent)
    {
        mSpriteComponent->LoadSpriteSheet(filepath);
    }
}

void NPC::SetSpriteConfiguration(int width, int height, int idleFrames, int walkFrames, float animSpeed)
{
    mSpriteWidth = width;
    mSpriteHeight = height;
    mIdleFrames = idleFrames;
    mWalkFrames = walkFrames;
    mAnimSpeed = animSpeed;

    if (mSpriteComponent)
    {
        mSpriteComponent->SetSpriteSize(width, height);
        mSpriteComponent->SetRenderSize(80.0f);
    }

    if (mAnimationComponent)
    {
        mAnimationComponent->SetFrameCount(idleFrames);
        mAnimationComponent->SetAnimSpeed(animSpeed);
    }
}

// UI Callback Handlers
void NPC::OnTalkSelected()
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

void NPC::OnTradeMenuSelected()
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

            tradeDescs.push_back(desc);
        }
        mDialogUI->ShowTradeMenu(tradeDescs);
    }
    else
    {
        mDialogUI->ShowMessage("I don't have anything to trade right now.");
    }
}

void NPC::OnLeaveSelected()
{
    EndInteraction();
}

void NPC::OnDialogOptionSelected(int index)
{
    if (index >= 0 && index < static_cast<int>(mDialogOptions.size()))
    {
        mDialogUI->ShowMessage(mDialogOptions[index].npcResponse);
    }
}

void NPC::OnTradeOptionSelected(int index)
{
    if (index >= 0 && index < static_cast<int>(mTradeOffers.size()))
    {
        // For now, just show a confirmation message
        // In a full implementation, this would check player inventory
        // and execute the trade
        mDialogUI->ShowMessage("Trade executed! (Inventory system not yet implemented)");
    }
}











