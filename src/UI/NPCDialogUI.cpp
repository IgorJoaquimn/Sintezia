#include "NPCDialogUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include <algorithm>

// NPCDialogUI Implementation
NPCDialogUI::NPCDialogUI(Game* game)
    : mGame(game)
    , mState(DialogUIState::Hidden)
    , mPreviousState(DialogUIState::Hidden)
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
    mPreviousState = mState;  // Save current state
    mState = DialogUIState::DialogMenu;
    mCurrentOptions = options;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowTradeMenu(const std::vector<std::string>& tradeDescriptions)
{
    mPreviousState = mState;  // Save current state
    mState = DialogUIState::TradeMenu;
    mCurrentOptions = tradeDescriptions;
    mSelectedIndex = 0;
}

void NPCDialogUI::ShowMessage(const std::string& message)
{
    mPreviousState = mState;  // Save current state before showing message
    mPreviousOptions = mCurrentOptions;  // Save current options to restore later
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
            // Return to the previous menu (DialogMenu or TradeMenu)
            if (mPreviousState == DialogUIState::DialogMenu)
            {
                // Restore dialog menu with previous options
                mState = DialogUIState::DialogMenu;
                mCurrentOptions = mPreviousOptions;
                mSelectedIndex = 0;
            }
            else if (mPreviousState == DialogUIState::TradeMenu)
            {
                // Restore trade menu with previous options
                mState = DialogUIState::TradeMenu;
                mCurrentOptions = mPreviousOptions;
                mSelectedIndex = 0;
            }
            else
            {
                // Fallback to main menu if we don't know where we came from
                ShowMainMenu();
            }
            break;

        default:
            break;
    }
}

std::vector<std::string> NPCDialogUI::WrapText(const std::string& text, float maxWidth, float scale, TextRenderer* textRenderer)
{
    std::vector<std::string> lines;
    if (!textRenderer || text.empty()) return lines;

    std::string currentLine;
    std::string word;

    for (size_t i = 0; i <= text.length(); i++)
    {
        char c = (i < text.length()) ? text[i] : ' ';

        if (c == ' ' || c == '\n' || i == text.length())
        {
            if (!word.empty())
            {
                std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                float lineWidth = textRenderer->GetTextWidth(testLine, scale);

                if (lineWidth > maxWidth && !currentLine.empty())
                {
                    // Current word doesn't fit, push current line and start new one
                    lines.push_back(currentLine);
                    currentLine = word;
                }
                else
                {
                    currentLine = testLine;
                }
                word.clear();
            }

            if (c == '\n' && i < text.length())
            {
                lines.push_back(currentLine);
                currentLine.clear();
            }
        }
        else
        {
            word += c;
        }
    }

    if (!currentLine.empty())
    {
        lines.push_back(currentLine);
    }

    return lines;
}

void NPCDialogUI::RenderWrappedText(const std::string& text, float x, float y, float maxWidth, float scale, float lineSpacing, TextRenderer* textRenderer)
{
    if (!textRenderer) return;

    std::vector<std::string> lines = WrapText(text, maxWidth, scale, textRenderer);

    for (size_t i = 0; i < lines.size(); i++)
    {
        textRenderer->RenderText(lines[i], x, y + static_cast<float>(i) * lineSpacing, scale);
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

    // Settings
    float textScale = 0.5f;
    float lineSpacing = 25.0f;
    float padding = 25.0f;

    // Draw background box
    float boxWidth = 800.0f;
    float boxHeight = 200.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw greeting text with wrapping
    float textX = boxX + padding;
    float textY = boxY + padding + 10.0f;
    float maxTextWidth = boxWidth - (padding * 2);

    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    RenderWrappedText(mCurrentText, textX, textY, maxTextWidth, textScale, lineSpacing, textRenderer);

    // Draw continue prompt
    std::string prompt = "Press SPACE to continue...";
    float promptY = boxY + boxHeight - 35.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText(prompt, textX, promptY, 0.4f);
}

void NPCDialogUI::DrawMainMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.5f;
    float padding = 25.0f;
    float lineHeight = 32.0f;

    // Draw background box
    float boxWidth = 600.0f;
    float boxHeight = 220.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw title
    float textX = boxX + padding;
    float textY = boxY + padding + 10.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText("What would you like to do?", textX, textY, textScale);

    // Draw options
    float optionY = textY + 45.0f;
    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        std::string prefix = (i == static_cast<size_t>(mSelectedIndex)) ? "> " : "  ";
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.9f, 0.9f, 0.9f);

        textRenderer->RenderText(prefix + mCurrentOptions[i], textX, optionY + static_cast<float>(i) * lineHeight, textScale);
    }

    // Draw controls
    float controlsY = boxY + boxHeight - 28.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText("W/S: Navigate | SPACE: Select | ESC: Exit",
                           textX, controlsY, 0.4f);
}

void NPCDialogUI::DrawDialogMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.5f;
    float padding = 25.0f;
    float lineHeight = 35.0f;

    // Calculate box height
    float minHeight = 180.0f;
    float contentHeight = static_cast<float>(mCurrentOptions.size()) * lineHeight;
    float boxHeight = std::min(400.0f, minHeight + contentHeight);

    // Draw background box
    float boxWidth = 700.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw title
    float textX = boxX + padding;
    float textY = boxY + padding + 10.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText("Choose a topic:", textX, textY, textScale);

    // Draw dialog options with wrapping
    float optionY = textY + 40.0f;
    float maxTextWidth = boxWidth - (padding * 2) - 30.0f;

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        std::string prefix = (i == static_cast<size_t>(mSelectedIndex)) ? "> " : "  ";
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.9f, 0.9f, 0.9f);

        std::string fullText = prefix + mCurrentOptions[i];

        // Check if text fits, if not wrap it
        if (textRenderer->GetTextWidth(fullText, textScale) > maxTextWidth)
        {
            // Truncate with ellipsis for now (proper wrapping would require multi-line per option)
            std::string truncated = prefix;
            for (char c : mCurrentOptions[i])
            {
                std::string test = truncated + c + "...";
                if (textRenderer->GetTextWidth(test, textScale) > maxTextWidth) break;
                truncated += c;
            }
            fullText = truncated + "...";
        }

        textRenderer->RenderText(fullText, textX, optionY + static_cast<float>(i) * lineHeight, textScale);
    }

    // Draw controls
    float controlsY = boxY + boxHeight - 28.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText("W/S: Navigate | SPACE: Select | A: Back | ESC: Exit",
                           textX, controlsY, 0.4f);
}

void NPCDialogUI::DrawTradeMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.45f;
    float padding = 25.0f;
    float itemSpacing = 65.0f;
    float lineSpacing = 20.0f;

    // Calculate box height
    float minHeight = 180.0f;
    float contentHeight = static_cast<float>(mCurrentOptions.size()) * itemSpacing;
    float boxHeight = std::min(500.0f, minHeight + contentHeight);

    // Draw background box
    float boxWidth = 850.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw title
    float textX = boxX + padding;
    float textY = boxY + padding + 10.0f;
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText("Available Trades:", textX, textY, textScale);

    // Draw trade offers with wrapping
    float optionY = textY + 40.0f;
    float maxTextWidth = boxWidth - (padding * 2) - 30.0f;

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        std::string prefix = isSelected ? "> " : "  ";

        if (isSelected)
            textRenderer->SetTextColor(1.0f, 1.0f, 0.0f);
        else
            textRenderer->SetTextColor(0.9f, 0.9f, 0.9f);

        // Wrap text for trade description
        std::vector<std::string> lines = WrapText(mCurrentOptions[i], maxTextWidth, textScale, textRenderer);

        float currentY = optionY + static_cast<float>(i) * itemSpacing;
        for (size_t lineIdx = 0; lineIdx < lines.size(); lineIdx++)
        {
            std::string lineText = (lineIdx == 0) ? prefix + lines[lineIdx] : "  " + lines[lineIdx];
            textRenderer->RenderText(lineText, textX, currentY + static_cast<float>(lineIdx) * lineSpacing, textScale);
        }
    }

    // Draw controls
    float controlsY = boxY + boxHeight - 28.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText("W/S: Navigate | SPACE: Trade | A: Back | ESC: Exit",
                           textX, controlsY, 0.4f);
}

void NPCDialogUI::DrawMessageUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // Settings
    float textScale = 0.5f;
    float lineSpacing = 25.0f;
    float padding = 25.0f;

    // Draw background box
    float boxWidth = 800.0f;
    float boxHeight = 220.0f;
    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);

    // Draw message text with wrapping
    float textX = boxX + padding;
    float textY = boxY + padding + 10.0f;
    float maxTextWidth = boxWidth - (padding * 2);

    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    RenderWrappedText(mCurrentText, textX, textY, maxTextWidth, textScale, lineSpacing, textRenderer);

    // Draw continue prompt
    std::string prompt = "Press SPACE to continue...";
    float promptY = boxY + boxHeight - 35.0f;
    textRenderer->SetTextColor(0.7f, 0.7f, 0.7f);
    textRenderer->RenderText(prompt, textX, promptY, 0.4f);
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
    float textScale = 0.4f;
    float padding = 10.0f;

    // Calculate text dimensions
    Vector2 textSize = textRenderer->MeasureText(text, textScale);

    // Position above the NPC's head
    // Assume sprite render size is ~80px and position is center of sprite
    float spriteRenderSize = 80.0f;
    float topOfSprite = mScreenPosition.y - spriteRenderSize * 0.5f;

    float boxWidth = textSize.x + (padding * 2);
    float boxHeight = textSize.y + (padding * 1.2f);
    float boxX = mScreenPosition.x - boxWidth * 0.5f;
    float boxY = topOfSprite - 12.0f - boxHeight; // 12px gap above sprite

    // Draw background
    rectRenderer->RenderRect(boxX, boxY, boxWidth, boxHeight, Vector3(0.1f, 0.1f, 0.2f), 0.85f);

    // Draw text centered in box
    textRenderer->SetTextColor(1.0f, 1.0f, 1.0f);
    textRenderer->RenderText(text, boxX + padding, boxY + (padding * 2), textScale);
}

void InteractionIndicator::UpdateScreenPosition()
{
    // For now, assume world position = screen position
    // In a proper implementation, you'd convert world coordinates to screen coordinates
    // using the camera transform
    mScreenPosition = mWorldPosition;
}

