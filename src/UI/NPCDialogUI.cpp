#include "NPCDialogUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/Texture.hpp"
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
    , mFacesetTexture(nullptr)
{
    // Load UI textures
    mDialogBoxTexture = std::make_shared<Texture>();
    if (!mDialogBoxTexture->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/DialogBox.png")) {
        SDL_Log("Failed to load DialogBox.png");
    }

    mChoiceBoxTexture = std::make_shared<Texture>();
    if (!mChoiceBoxTexture->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/ChoiceBox.png")) {
        SDL_Log("Failed to load ChoiceBox.png");
    }
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
            // Return to the previous menu state
            if (mPreviousState == DialogUIState::DialogMenu || mPreviousState == DialogUIState::TradeMenu)
            {
                mState = mPreviousState;
                mCurrentOptions = mPreviousOptions;
                mSelectedIndex = 0;
            }
            else
            {
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

    // UI Settings
    float textScale = 0.5f;
    float lineSpacing = 20.0f;
    float uiScale = 2.5f;

    // Calculate Dialog Box Dimensions
    float boxWidth = mDialogBoxTexture ? mDialogBoxTexture->GetWidth() * uiScale : 800.0f;
    float boxHeight = mDialogBoxTexture ? mDialogBoxTexture->GetHeight() * uiScale : 200.0f;

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    // Draw Dialog Box Background
    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(),
            Vector2(boxX, boxY), Vector2(boxWidth, boxHeight),
            0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    // Text layout
    float marginLeft = 20.0f * uiScale;
    float marginTop = 14.0f * uiScale;
    float marginRight = 20.0f * uiScale;

    float textX = boxX + marginLeft;
    float textY = boxY + marginTop + 20.0f;
    float textWidth = boxWidth - marginLeft - marginRight;

    // Draw Faceset if available
    if (mFacesetTexture && mGame->GetSpriteRenderer())
    {
        int faceSize = std::min(mFacesetTexture->GetWidth(), mFacesetTexture->GetHeight());
        float faceDisplaySize = 38.0f * uiScale;

        float faceX = boxX + (6.0f * uiScale);
        float faceY = boxY + marginTop;

        // Calculate normalized source coordinates
        float normW = static_cast<float>(faceSize) / static_cast<float>(mFacesetTexture->GetWidth());
        float normH = static_cast<float>(faceSize) / static_cast<float>(mFacesetTexture->GetHeight());

        mGame->GetSpriteRenderer()->DrawSprite(mFacesetTexture.get(),
            Vector2(faceX, faceY), Vector2(faceDisplaySize, faceDisplaySize),
            Vector2(0.0f, 0.0f), Vector2(normW, normH));

        // Adjust text position to accommodate face
        textX = boxX + (50.0f * uiScale);
        textWidth = boxWidth - (50.0f * uiScale) - marginRight;
        textY = faceY + 20.0f;
    }

    // Draw greeting text with wrapping
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f);
    RenderWrappedText(mCurrentText, textX, textY, textWidth, textScale, lineSpacing, textRenderer);
}

void NPCDialogUI::DrawMainMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // UI Settings
    float textScale = 0.5f;
    float uiScale = 2.5f;

    // Calculate Dialog Box Dimensions
    float boxWidth = mDialogBoxTexture ? mDialogBoxTexture->GetWidth() * uiScale : 800.0f;
    float boxHeight = mDialogBoxTexture ? mDialogBoxTexture->GetHeight() * uiScale : 200.0f;

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    // Draw Dialog Box Background
    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(),
            Vector2(boxX, boxY), Vector2(boxWidth, boxHeight),
            0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    float marginLeft = 20.0f * uiScale;
    float marginTop = 14.0f * uiScale;

    // Draw options as horizontal buttons (centered vertically in the dialog box)
    float buttonSpacing = 15.0f;
    float buttonPaddingX = 20.0f;
    float buttonPaddingY = 12.0f;
    float buttonY = boxY + marginTop + 20.0f;

    float currentX = boxX + marginLeft;

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));

        // Measure text
        Vector2 textSize = textRenderer->MeasureText(mCurrentOptions[i], textScale);
        float buttonWidth = textSize.x + (buttonPaddingX * 2);
        float buttonHeight = textSize.y + (buttonPaddingY * 2);

        // Draw button background with ChoiceBox texture
        if (mChoiceBoxTexture && mGame->GetSpriteRenderer()) {
            Vector3 tint = isSelected ? Vector3(0.9f, 1.2f, 1.3f) : Vector3(0.85f, 0.85f, 0.85f);
            mGame->GetSpriteRenderer()->DrawSprite(mChoiceBoxTexture.get(),
                Vector2(currentX, buttonY), Vector2(buttonWidth, buttonHeight),
                0.0f, tint);
        } else {
            Vector3 color = isSelected ? Vector3(0.3f, 0.5f, 0.6f) : Vector3(0.3f, 0.3f, 0.4f);
            DrawBox(rectRenderer, currentX, buttonY, buttonWidth, buttonHeight, color, 0.9f);
        }

        // Draw button text
        textRenderer->SetTextColor(
            isSelected ? 0.0f : 0.08f,
            isSelected ? 0.9f : 0.11f,
            isSelected ? 1.0f : 0.11f
        );
        textRenderer->RenderText(mCurrentOptions[i],
            currentX + buttonPaddingX,
            buttonY + buttonPaddingY + 15.0f,
            textScale);

        currentX += buttonWidth + buttonSpacing;
    }

    // Draw navigation hint
    textRenderer->SetTextColor(0.5f, 0.5f, 0.5f);
    float hintY = boxY + boxHeight - 35.0f;
    textRenderer->RenderText("[A/D] Navigate  [ENTER] Select  [ESC] Close", boxX + marginLeft, hintY, 0.35f);
}

void NPCDialogUI::DrawDialogMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // UI Settings
    float textScale = 0.45f;
    float lineHeight = 28.0f;
    float uiScale = 2.5f;

    // Calculate Dialog Box Dimensions
    float boxWidth = mDialogBoxTexture ? mDialogBoxTexture->GetWidth() * uiScale : 800.0f;
    float boxHeight = mDialogBoxTexture ? mDialogBoxTexture->GetHeight() * uiScale : 200.0f;

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    // Draw Dialog Box Background
    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(),
            Vector2(boxX, boxY), Vector2(boxWidth, boxHeight),
            0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    float marginLeft = 20.0f * uiScale;
    float marginTop = 14.0f * uiScale;
    float marginRight = 20.0f * uiScale;

    float textX = boxX + marginLeft;
    float textY = boxY + marginTop + 20.0f;
    float maxTextWidth = boxWidth - marginLeft - marginRight;

    // Draw dialog options
    float optionY = textY + 5.0f;

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));

        // Draw selection indicator
        if (isSelected) {
            textRenderer->SetTextColor(0.0f, 0.8f, 0.9f);
            textRenderer->RenderText(">", textX - 15.0f, optionY + static_cast<float>(i) * lineHeight, textScale);
        }

        // Draw option text
        textRenderer->SetTextColor(
            isSelected ? 0.0f : 0.08f,
            isSelected ? 0.9f : 0.11f,
            isSelected ? 1.0f : 0.11f
        );

        // Truncate if too long
        std::string optionText = mCurrentOptions[i];
        if (textRenderer->GetTextWidth(optionText, textScale) > maxTextWidth - 30.0f) {
            std::string truncated;
            for (char c : optionText) {
                if (textRenderer->GetTextWidth(truncated + c + "...", textScale) > maxTextWidth - 30.0f) break;
                truncated += c;
            }
            optionText = truncated + "...";
        }

        textRenderer->RenderText(optionText, textX, optionY + static_cast<float>(i) * lineHeight, textScale);
    }

    // Draw navigation hint
    textRenderer->SetTextColor(0.5f, 0.5f, 0.5f);
    float hintY = boxY + boxHeight - 35.0f;
    textRenderer->RenderText("[W/S] Navigate  [ENTER] Select  [ESC] Back", boxX + marginLeft, hintY, 0.35f);
}

void NPCDialogUI::DrawTradeMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // UI Settings
    float textScale = 0.45f;
    float itemSpacing = 28.0f;
    float uiScale = 2.5f;

    // Calculate Dialog Box Dimensions
    float boxWidth = mDialogBoxTexture ? mDialogBoxTexture->GetWidth() * uiScale : 800.0f;
    float boxHeight = mDialogBoxTexture ? mDialogBoxTexture->GetHeight() * uiScale : 200.0f;

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    // Draw Dialog Box Background
    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(),
            Vector2(boxX, boxY), Vector2(boxWidth, boxHeight),
            0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    float marginLeft = 20.0f * uiScale;
    float marginTop = 14.0f * uiScale;
    float marginRight = 20.0f * uiScale;

    float textX = boxX + marginLeft;
    float textY = boxY + marginTop + 20.0f;
    float maxTextWidth = boxWidth - marginLeft - marginRight;

    // Draw trade offers - compact format
    float optionY = textY + 5.0f;

    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        float currentY = optionY + static_cast<float>(i) * itemSpacing;

        // Draw selection indicator
        if (isSelected) {
            textRenderer->SetTextColor(0.0f, 0.8f, 0.9f);
            textRenderer->RenderText(">", textX - 15.0f, currentY, textScale);
        }

        // Set text color
        textRenderer->SetTextColor(
            isSelected ? 0.0f : 0.08f,
            isSelected ? 0.9f : 0.11f,
            isSelected ? 1.0f : 0.11f
        );

        // Draw trade description (already formatted concisely by caller)
        std::string tradeText = mCurrentOptions[i];
        if (textRenderer->GetTextWidth(tradeText, textScale) > maxTextWidth - 30.0f) {
            std::string truncated;
            for (char c : tradeText) {
                if (textRenderer->GetTextWidth(truncated + c + "...", textScale) > maxTextWidth - 30.0f) break;
                truncated += c;
            }
            tradeText = truncated + "...";
        }

        textRenderer->RenderText(tradeText, textX, currentY, textScale);
    }

    // Draw navigation hint
    textRenderer->SetTextColor(0.5f, 0.5f, 0.5f);
    float hintY = boxY + boxHeight - 35.0f;
    textRenderer->RenderText("[W/S] Navigate  [ENTER] Select  [ESC] Back", boxX + marginLeft, hintY, 0.35f);
}

void NPCDialogUI::DrawMessageUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    // UI Settings
    float textScale = 0.5f;
    float lineSpacing = 20.0f;
    float uiScale = 2.5f;

    // Calculate Dialog Box Dimensions
    float boxWidth = mDialogBoxTexture ? mDialogBoxTexture->GetWidth() * uiScale : 800.0f;
    float boxHeight = mDialogBoxTexture ? mDialogBoxTexture->GetHeight() * uiScale : 200.0f;

    float boxX = (Game::WINDOW_WIDTH - boxWidth) / 2.0f;
    float boxY = Game::WINDOW_HEIGHT - boxHeight - 50.0f;

    // Draw Dialog Box Background
    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        mGame->GetSpriteRenderer()->DrawSprite(mDialogBoxTexture.get(),
            Vector2(boxX, boxY), Vector2(boxWidth, boxHeight),
            0.0f, Vector3(1.0f, 1.0f, 1.0f));
    } else {
        DrawBox(rectRenderer, boxX, boxY, boxWidth, boxHeight, Vector3(0.2f, 0.2f, 0.3f), 0.9f);
    }

    float marginLeft = 20.0f * uiScale;
    float marginTop = 14.0f * uiScale;
    float marginRight = 20.0f * uiScale;

    float textX = boxX + marginLeft;
    float textY = boxY + marginTop + 20.0f;
    float maxTextWidth = boxWidth - marginLeft - marginRight;

    // Draw message text with wrapping
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f);
    RenderWrappedText(mCurrentText, textX, textY, maxTextWidth, textScale, lineSpacing, textRenderer);
}

void NPCDialogUI::SetFacesetTexture(const std::string& path)
{
    mFacesetTexture = std::make_shared<Texture>();
    if (!mFacesetTexture->Load(path))
    {
        SDL_Log("Failed to load faceset: %s", path.c_str());
        mFacesetTexture.reset();
    }
}

// InteractionIndicator Implementation
InteractionIndicator::InteractionIndicator(Game* game)
    : mGame(game)
    , mIsVisible(false)
    , mWorldPosition(0.0f, 0.0f)
    , mScreenPosition(0.0f, 0.0f)
    , mAnimTime(0.0f)
    , mAnimFrame(0)
    , mMaxFrames(4)  // DialogInfo.png has 4 animation frames (empty, ., .., ...)
    , mAnimSpeed(4.0f)  // 4 fps for smooth bubble animation
{
    // Load DialogInfo texture
    mDialogInfoTexture = std::make_shared<Texture>();
    if (!mDialogInfoTexture->Load("assets/third_party/Ninja Adventure - Asset Pack/Ui/Dialog/DialogInfo.png")) {
        SDL_Log("Failed to load DialogInfo.png");
        mDialogInfoTexture.reset();
    }
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

void InteractionIndicator::Update(float deltaTime)
{
    if (!mIsVisible) return;

    // Update animation
    mAnimTime += deltaTime;
    float frameTime = 1.0f / mAnimSpeed;

    if (mAnimTime >= frameTime)
    {
        mAnimTime -= frameTime;
        mAnimFrame = (mAnimFrame + 1) % mMaxFrames;
    }

    // Update screen position (in case world/camera changes)
    UpdateScreenPosition();
}

void InteractionIndicator::Draw(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!mIsVisible || !mDialogInfoTexture || !mGame->GetSpriteRenderer()) return;

    // DialogInfo.png is typically a horizontal sprite sheet with multiple frames
    // Each frame is usually 16x16 pixels
    int frameWidth = 20;
    int frameHeight = 16;

    // Calculate source rectangle for current frame
    // Frames are arranged horizontally in the sprite sheet
    float texWidth = static_cast<float>(mDialogInfoTexture->GetWidth());
    float texHeight = static_cast<float>(mDialogInfoTexture->GetHeight());

    // Normalize coordinates for source rect (0.0 to 1.0)
    float srcX = (static_cast<float>(mAnimFrame * frameWidth)) / texWidth;
    float srcY = 0.0f;
    float srcW = static_cast<float>(frameWidth) / texWidth;
    float srcH = static_cast<float>(frameHeight) / texHeight;

    Vector2 srcPos(srcX, srcY);
    Vector2 srcSize(srcW, srcH);

    // Position above the NPC's head
    float spriteRenderSize = 80.0f;
    float topOfSprite = mScreenPosition.y - spriteRenderSize * 0.5f;

    // Scale up the bubble sprite (2x scale looks good)
    float bubbleScale = 2.5f;
    float bubbleWidth = frameWidth * bubbleScale;
    float bubbleHeight = frameHeight * bubbleScale;

    // Center the bubble above the NPC
    float bubbleX = mScreenPosition.x - bubbleWidth * 0.5f;
    float bubbleY = topOfSprite - 20.0f - bubbleHeight; // 20px gap above sprite

    Vector2 position(bubbleX, bubbleY);
    Vector2 size(bubbleWidth, bubbleHeight);

    // Draw the animated sprite
    mGame->GetSpriteRenderer()->DrawSprite(
        mDialogInfoTexture.get(),
        position,
        size,
        srcPos,
        srcSize,
        0.0f,  // rotation
        Vector3(1.0f, 1.0f, 1.0f)  // white color (no tint)
    );
}

void InteractionIndicator::UpdateScreenPosition()
{
    // For now, assume world position = screen position
    // In a proper implementation, you'd convert world coordinates to screen coordinates
    // using the camera transform
    mScreenPosition = mWorldPosition;
}

