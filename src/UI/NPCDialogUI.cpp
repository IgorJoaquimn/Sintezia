#include "NPCDialogUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include <algorithm>

// NPCDialogUI Implementation
NPCDialogUI::NPCDialogUI(Game* game)
    : mGame(game)
    , mState(DialogUIState::Hidden)
    , mSelectedIndex(0)
    , mOnDialogSelected(nullptr)
    , mOnTradeSelected(nullptr)
    , mOnTalkSelected(nullptr)
    , mOnTradeMenuSelected(nullptr)
    , mOnLeaveSelected(nullptr)
{
}

NPCDialogUI::~NPCDialogUI()
{
}

void NPCDialogUI::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!IsVisible()) return;

    switch (mState)
    {
        case DialogUIState::Greeting:
            DrawGreetingUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::MainMenu:
            DrawMainMenuUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::DialogMenu:
            DrawDialogMenuUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::TradeMenu:
            DrawTradeMenuUI(textRenderer, rectRenderer);
            break;
        case DialogUIState::Message:
            DrawMessageUI(textRenderer, rectRenderer);
            break;
        default:
            break;
    }
}

void NPCDialogUI::Update(float deltaTime)
{
    // Future: Add animations, transitions, etc.
}

void NPCDialogUI::ShowGreeting(const std::string& greeting)
{
    mState = DialogUIState::Greeting;
    mCurrentText = greeting;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowMainMenu()
{
    mState = DialogUIState::MainMenu;
    mSelectedIndex = 0;
    mCurrentOptions.clear();
    mCurrentOptions = {"Talk", "Trade", "Leave"};
}

void NPCDialogUI::ShowDialogMenu(const std::vector<std::string>& options)
{
    mState = DialogUIState::DialogMenu;
    mCurrentOptions = options;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowTradeMenu(const std::vector<std::string>& tradeDescriptions)
{
    mState = DialogUIState::TradeMenu;
    mCurrentOptions = tradeDescriptions;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowMessage(const std::string& message)
{
    mState = DialogUIState::Message;
    mCurrentText = message;
    mSelectedIndex = 0;
}

void NPCDialogUI::Hide()
{
    mState = DialogUIState::Hidden;
    mSelectedIndex = 0;
    mCurrentText.clear();
    mCurrentOptions.clear();
}

void NPCDialogUI::NavigateUp()
{
    if (mCurrentOptions.empty()) return;
    mSelectedIndex--;
    if (mSelectedIndex < 0)
        mSelectedIndex = mCurrentOptions.size() - 1;
}

void NPCDialogUI::NavigateDown()
{
    if (mCurrentOptions.empty()) return;
    mSelectedIndex++;
    if (mSelectedIndex >= static_cast<int>(mCurrentOptions.size()))
        mSelectedIndex = 0;
}

void NPCDialogUI::SelectCurrent()
{
    switch (mState)
    {
        case DialogUIState::Greeting:
            ShowMainMenu();
            break;

        case DialogUIState::MainMenu:
            if (mSelectedIndex == 0 && mOnTalkSelected)
                mOnTalkSelected();
            else if (mSelectedIndex == 1 && mOnTradeMenuSelected)
                mOnTradeMenuSelected();
            else if (mSelectedIndex == 2 && mOnLeaveSelected)
                mOnLeaveSelected();
            break;

        case DialogUIState::DialogMenu:
            if (mOnDialogSelected && mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mCurrentOptions.size()))
                mOnDialogSelected(mSelectedIndex);
            break;

        case DialogUIState::TradeMenu:
            if (mOnTradeSelected && mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mCurrentOptions.size()))
                mOnTradeSelected(mSelectedIndex);
            break;

        case DialogUIState::Message:
            ShowMainMenu();
            break;

        default:
            break;
    }
}

void NPCDialogUI::DrawBox(RectRenderer* rectRenderer, float x, float y, float width, float height, const Vector3& color, float alpha)
{
    if (rectRenderer)
        rectRenderer->RenderRect(x, y, width, height, color, alpha);
}

void NPCDialogUI::DrawGreetingUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw background box
    float boxWidth = 800.0f;
    float boxHeight = 200.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw greeting text
    float textX = boxX + 30.0f;
    float textY = boxY + 30.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText(mCurrentText, textX, textY, 1.0f);

    // Draw continue prompt
    std::string prompt = "Press SPACE to continue...";
    float promptY = boxY + boxHeight - 50.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText(prompt, textX, promptY, 0.8f);
}

void NPCDialogUI::DrawMainMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw background box
    float boxWidth = 800.0f;
    float boxHeight = 300.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw title
    float textX = boxX + 30.0f;
    float textY = boxY + 30.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText("What would you like to do?", textX, textY, 1.0f);

    // Draw options
    float optionY = textY + 60.0f;
    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        std::string prefix = (i == static_cast<size_t>(mSelectedIndex)) ? "> " : "  ";
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.9f, 0.9f, 0.9f);

        textRenderer->RenderText(prefix + mCurrentOptions[i], textX, optionY + i * 50.0f, 1.0f);
    }

    // Draw controls
    float controlsY = boxY + boxHeight - 50.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText("W/S: Navigate | SPACE: Select | ESC: Cancel",
                           textX, controlsY, 0.7f);
}

void NPCDialogUI::DrawDialogMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw background box
    float boxWidth = 800.0f;
    float boxHeight = std::min(400.0f, 150.0f + mCurrentOptions.size() * 50.0f);
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw title
    float textX = boxX + 30.0f;
    float textY = boxY + 30.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText("Choose a topic:", textX, textY, 1.0f);

    // Draw dialog options
    float optionY = textY + 60.0f;
    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        std::string prefix = (i == static_cast<size_t>(mSelectedIndex)) ? "> " : "  ";
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.9f, 0.9f, 0.9f);

        textRenderer->RenderText(prefix + mCurrentOptions[i], textX,
                                optionY + i * 50.0f, 0.9f);
    }

    // Draw controls
    float controlsY = boxY + boxHeight - 50.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText("W/S: Navigate | SPACE: Select | ESC: Back",
                           textX, controlsY, 0.7f);
}

void NPCDialogUI::DrawTradeMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw background box
    float boxWidth = 900.0f;
    float boxHeight = std::min(500.0f, 200.0f + mCurrentOptions.size() * 80.0f);
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw title
    float textX = boxX + 30.0f;
    float textY = boxY + 30.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText("Available Trades:", textX, textY, 1.0f);

    // Draw trade offers
    float optionY = textY + 60.0f;
    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        std::string prefix = isSelected ? "> " : "  ";

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.9f, 0.9f, 0.9f);

        textRenderer->RenderText(prefix + mCurrentOptions[i], textX,
                                optionY + i * 80.0f, 0.9f);
    }

    // Draw controls
    float controlsY = boxY + boxHeight - 50.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText("W/S: Navigate | SPACE: Trade | ESC: Back",
                           textX, controlsY, 0.7f);
}

void NPCDialogUI::DrawMessageUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Draw background box
    float boxWidth = 800.0f;
    float boxHeight = 250.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw message text
    float textX = boxX + 30.0f;
    float textY = boxY + 30.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText(mCurrentText, textX, textY, 0.9f);

    // Draw continue prompt
    std::string prompt = "Press SPACE to continue...";
    float promptY = boxY + boxHeight - 50.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText(prompt, textX, promptY, 0.8f);
}

// InteractionIndicator Implementation
InteractionIndicator::InteractionIndicator(Game* game)
    : mGame(game)
    , mIsVisible(false)
    , mWorldPosition(0.0f, 0.0f)
    , mScreenPosition(0.0f, 0.0f)
{
}

InteractionIndicator::~InteractionIndicator()
{
}

void InteractionIndicator::Show(const Vector2& worldPosition)
{
    mIsVisible = true;
    mWorldPosition = worldPosition;
    UpdateScreenPosition();
}

void InteractionIndicator::Hide()
{
    mIsVisible = false;
}

void InteractionIndicator::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!mIsVisible || !textRenderer || !rectRenderer) return;

    // Draw a small box with "[E] Interact" text above the NPC
    std::string text = "[E] Interact";
    float textScale = 0.8f;

    // Calculate text dimensions
    Vector2 textSize = textRenderer->MeasureText(text, textScale);

    // Position above the NPC's head
    float boxWidth = textSize.x + 20.0f;
    float boxHeight = textSize.y + 10.0f;
    float boxX = mScreenPosition.x - boxWidth / 2.0f;
    float boxY = mScreenPosition.y - 100.0f; // Above the NPC

    // Draw background
    rectRenderer->RenderRect(boxX, boxY, boxWidth, boxHeight, Vector3(0.1f, 0.1f, 0.2f), 0.8f);

    // Draw text
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText(text, boxX + 10.0f, boxY + 5.0f, textScale);
}

void InteractionIndicator::UpdateScreenPosition()
{
    // For now, assume world position = screen position
    // In a proper implementation, you'd convert world coordinates to screen coordinates
    // using the camera transform
    mScreenPosition = mWorldPosition;
}

