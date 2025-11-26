#include "NPCDialogUI.hpp"
#include "../Game/Game.hpp"
#include "../Core/TextRenderer/TextRenderer.hpp"
#include "../Core/RectRenderer/RectRenderer.hpp"
#include "../Core/Texture/Texture.hpp"
#include <algorithm>

// ============================================================================
// UI Constants
// ============================================================================
namespace UIConstants
{
    // Dialog Box Settings
    constexpr float UI_SCALE = 2.5f;
    constexpr float DIALOG_BOX_Y_OFFSET = 50.0f;

    // Margins and Padding
    constexpr float MARGIN_LEFT = 20.0f;
    constexpr float MARGIN_TOP = 14.0f;
    constexpr float MARGIN_RIGHT = 20.0f;
    constexpr float MARGIN_BOTTOM = 35.0f;

    // Text Settings
    constexpr float TEXT_SCALE_NORMAL = 0.5f;
    constexpr float TEXT_SCALE_SMALL = 0.45f;
    constexpr float TEXT_SCALE_HINT = 0.35f;
    constexpr float LINE_SPACING = 20.0f;
    constexpr float LINE_HEIGHT = 28.0f;

    // Button Settings
    constexpr float BUTTON_SPACING = 15.0f;
    constexpr float BUTTON_PADDING_X = 20.0f;
    constexpr float BUTTON_PADDING_Y = 12.0f;

    // Faceset Settings
    constexpr float FACESET_MARGIN = 6.0f;
    constexpr float FACESET_SIZE = 38.0f;
    constexpr float FACESET_TEXT_OFFSET = 50.0f;
    constexpr float FACESET_VERTICAL_OFFSET = 20.0f;

    // Selection Indicator
    constexpr float SELECTION_ARROW_OFFSET = 15.0f;

    // Colors
    const Vector3 COLOR_BG_DEFAULT(0.2f, 0.2f, 0.3f);
    const Vector3 COLOR_BUTTON_DEFAULT(0.3f, 0.3f, 0.4f);
    const Vector3 COLOR_BUTTON_SELECTED(0.3f, 0.5f, 0.6f);
    const Vector3 COLOR_TINT_SELECTED(0.9f, 1.2f, 1.3f);
    const Vector3 COLOR_TINT_DEFAULT(0.85f, 0.85f, 0.85f);

    constexpr float ALPHA_DEFAULT = 0.9f;
}

// ============================================================================
// NPCDialogUI Implementation
// ============================================================================
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
    if (text.empty()) return lines;

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

// ============================================================================
// Helper Methods
// ============================================================================

void NPCDialogUI::DrawBox(RectRenderer* rectRenderer, float x, float y, float width, float height, const Vector3& color, float alpha)
{
    if (rectRenderer)
        rectRenderer->RenderRect(x, y, width, height, color, alpha);
}

DialogBoxLayout NPCDialogUI::CalculateDialogBoxLayout() const
{
    DialogBoxLayout layout;

    layout.boxWidth = mDialogBoxTexture ?
        mDialogBoxTexture->GetWidth() * UIConstants::UI_SCALE : 800.0f;
    layout.boxHeight = mDialogBoxTexture ?
        mDialogBoxTexture->GetHeight() * UIConstants::UI_SCALE : 200.0f;

    layout.boxX = (Game::WINDOW_WIDTH - layout.boxWidth) / 2.0f;
    layout.boxY = Game::WINDOW_HEIGHT - layout.boxHeight - UIConstants::DIALOG_BOX_Y_OFFSET;

    float marginLeft = UIConstants::MARGIN_LEFT * UIConstants::UI_SCALE;
    float marginTop = UIConstants::MARGIN_TOP * UIConstants::UI_SCALE;
    float marginRight = UIConstants::MARGIN_RIGHT * UIConstants::UI_SCALE;

    layout.textX = layout.boxX + marginLeft;
    layout.textY = layout.boxY + marginTop + 20.0f;
    layout.maxTextWidth = layout.boxWidth - marginLeft - marginRight;

    return layout;
}

void NPCDialogUI::DrawDialogBoxBackground(const DialogBoxLayout& layout, RectRenderer* rectRenderer)
{
    if (mDialogBoxTexture && mGame->GetSpriteRenderer()) {
        mGame->GetSpriteRenderer()->DrawSprite(
            mDialogBoxTexture.get(),
            Vector2(layout.boxX, layout.boxY),
            Vector2(layout.boxWidth, layout.boxHeight),
            0.0f,
            Vector3(1.0f, 1.0f, 1.0f)
        );
    } else {
        DrawBox(rectRenderer, layout.boxX, layout.boxY, layout.boxWidth, layout.boxHeight,
                UIConstants::COLOR_BG_DEFAULT, UIConstants::ALPHA_DEFAULT);
    }
}

void NPCDialogUI::DrawNavigationHint(const std::string& hint, const DialogBoxLayout& layout, TextRenderer* textRenderer)
{
    textRenderer->SetTextColor(0.5f, 0.5f, 0.5f);
    float hintY = layout.boxY + layout.boxHeight - UIConstants::MARGIN_BOTTOM;
    float marginLeft = UIConstants::MARGIN_LEFT * UIConstants::UI_SCALE;
    textRenderer->RenderText(hint, layout.boxX + marginLeft, hintY, UIConstants::TEXT_SCALE_HINT);
}

// ============================================================================
// Drawing Methods
// ============================================================================

void NPCDialogUI::DrawFaceset(const DialogBoxLayout& layout, float& outTextX, float& outTextWidth, float& outTextY)
{
    if (!mFacesetTexture || !mGame->GetSpriteRenderer()) return;

    int faceSize = std::min(mFacesetTexture->GetWidth(), mFacesetTexture->GetHeight());
    float faceDisplaySize = UIConstants::FACESET_SIZE * UIConstants::UI_SCALE;

    float faceX = layout.boxX + (UIConstants::FACESET_MARGIN * UIConstants::UI_SCALE);
    float faceY = layout.boxY + (UIConstants::MARGIN_TOP * UIConstants::UI_SCALE);

    // Calculate normalized source coordinates
    float normW = static_cast<float>(faceSize) / static_cast<float>(mFacesetTexture->GetWidth());
    float normH = static_cast<float>(faceSize) / static_cast<float>(mFacesetTexture->GetHeight());

    mGame->GetSpriteRenderer()->DrawSprite(
        mFacesetTexture.get(),
        Vector2(faceX, faceY),
        Vector2(faceDisplaySize, faceDisplaySize),
        Vector2(0.0f, 0.0f),
        Vector2(normW, normH)
    );

    // Adjust text position to accommodate faceset
    float marginRight = UIConstants::MARGIN_RIGHT * UIConstants::UI_SCALE;
    outTextX = layout.boxX + (UIConstants::FACESET_TEXT_OFFSET * UIConstants::UI_SCALE);
    outTextWidth = layout.boxWidth - (UIConstants::FACESET_TEXT_OFFSET * UIConstants::UI_SCALE) - marginRight;
    outTextY = faceY + UIConstants::FACESET_VERTICAL_OFFSET;
}

void NPCDialogUI::DrawGreetingUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    DialogBoxLayout layout = CalculateDialogBoxLayout();
    DrawDialogBoxBackground(layout, rectRenderer);

    float textX = layout.textX;
    float textY = layout.textY;
    float textWidth = layout.maxTextWidth;

    // Draw faceset if available and adjust text position
    if (mFacesetTexture) {
        DrawFaceset(layout, textX, textWidth, textY);
    }

    // Draw greeting text with wrapping
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f);
    RenderWrappedText(mCurrentText, textX, textY, textWidth,
                     UIConstants::TEXT_SCALE_NORMAL,
                     UIConstants::LINE_SPACING, textRenderer);
}

void NPCDialogUI::DrawButton(const std::string& text, float x, float y, bool isSelected,
                            TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    // Measure text
    Vector2 textSize = textRenderer->MeasureText(text, UIConstants::TEXT_SCALE_NORMAL);
    float buttonWidth = textSize.x + (UIConstants::BUTTON_PADDING_X * 2);
    float buttonHeight = textSize.y + (UIConstants::BUTTON_PADDING_Y * 2);

    // Draw button background
    if (mChoiceBoxTexture && mGame->GetSpriteRenderer()) {
        Vector3 tint = isSelected ? UIConstants::COLOR_TINT_SELECTED : UIConstants::COLOR_TINT_DEFAULT;
        mGame->GetSpriteRenderer()->DrawSprite(
            mChoiceBoxTexture.get(),
            Vector2(x, y),
            Vector2(buttonWidth, buttonHeight),
            0.0f, tint
        );
    } else {
        Vector3 color = isSelected ? UIConstants::COLOR_BUTTON_SELECTED : UIConstants::COLOR_BUTTON_DEFAULT;
        DrawBox(rectRenderer, x, y, buttonWidth, buttonHeight, color, UIConstants::ALPHA_DEFAULT);
    }

    // Draw button text
    textRenderer->SetTextColor(
        isSelected ? 0.0f : 0.08f,
        isSelected ? 0.9f : 0.11f,
        isSelected ? 1.0f : 0.11f
    );
    textRenderer->RenderText(
        text,
        x + UIConstants::BUTTON_PADDING_X,
        y + UIConstants::BUTTON_PADDING_Y + 15.0f,
        UIConstants::TEXT_SCALE_NORMAL
    );
}

void NPCDialogUI::DrawMainMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    DialogBoxLayout layout = CalculateDialogBoxLayout();
    DrawDialogBoxBackground(layout, rectRenderer);

    float marginLeft = UIConstants::MARGIN_LEFT * UIConstants::UI_SCALE;
    float marginTop = UIConstants::MARGIN_TOP * UIConstants::UI_SCALE;
    float buttonY = layout.boxY + marginTop + 20.0f;
    float currentX = layout.boxX + marginLeft;

    // Draw horizontal menu buttons
    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        DrawButton(mCurrentOptions[i], currentX, buttonY, isSelected, textRenderer, rectRenderer);

        Vector2 textSize = textRenderer->MeasureText(mCurrentOptions[i], UIConstants::TEXT_SCALE_NORMAL);
        currentX += textSize.x + (UIConstants::BUTTON_PADDING_X * 2) + UIConstants::BUTTON_SPACING;
    }

    DrawNavigationHint("[A/D] Navigate  [ENTER] Select  [ESC] Close", layout, textRenderer);
}

std::string NPCDialogUI::TruncateText(const std::string& text, float maxWidth, float scale, TextRenderer* textRenderer)
{
    if (!textRenderer || textRenderer->GetTextWidth(text, scale) <= maxWidth) {
        return text;
    }

    std::string truncated;
    for (char c : text) {
        if (textRenderer->GetTextWidth(truncated + c + "...", scale) > maxWidth) {
            break;
        }
        truncated += c;
    }
    return truncated + "...";
}

void NPCDialogUI::DrawListOption(const std::string& text, float x, float y, bool isSelected,
                                 float maxWidth, float textScale, TextRenderer* textRenderer)
{
    // Draw selection indicator
    if (isSelected) {
        textRenderer->SetTextColor(0.0f, 0.8f, 0.9f);
        textRenderer->RenderText(">", x - UIConstants::SELECTION_ARROW_OFFSET, y, textScale);
    }

    // Set text color based on selection
    textRenderer->SetTextColor(
        isSelected ? 0.0f : 0.08f,
        isSelected ? 0.9f : 0.11f,
        isSelected ? 1.0f : 0.11f
    );

    // Truncate if necessary and render
    std::string displayText = TruncateText(text, maxWidth - 30.0f, textScale, textRenderer);
    textRenderer->RenderText(displayText, x, y, textScale);
}

void NPCDialogUI::DrawDialogMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    DialogBoxLayout layout = CalculateDialogBoxLayout();
    DrawDialogBoxBackground(layout, rectRenderer);

    float optionY = layout.textY + 5.0f;

    // Draw all dialog options as a vertical list
    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        float currentY = optionY + static_cast<float>(i) * UIConstants::LINE_HEIGHT;

        DrawListOption(mCurrentOptions[i], layout.textX, currentY, isSelected,
                      layout.maxTextWidth, UIConstants::TEXT_SCALE_SMALL, textRenderer);
    }

    DrawNavigationHint("[W/S] Navigate  [ENTER] Select  [ESC] Back", layout, textRenderer);
}

void NPCDialogUI::DrawTradeMenuUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    DialogBoxLayout layout = CalculateDialogBoxLayout();
    DrawDialogBoxBackground(layout, rectRenderer);

    float optionY = layout.textY + 5.0f;

    // Draw all trade offers as a vertical list
    for (size_t i = 0; i < mCurrentOptions.size(); i++)
    {
        bool isSelected = (i == static_cast<size_t>(mSelectedIndex));
        float currentY = optionY + static_cast<float>(i) * UIConstants::LINE_HEIGHT;

        DrawListOption(mCurrentOptions[i], layout.textX, currentY, isSelected,
                      layout.maxTextWidth, UIConstants::TEXT_SCALE_SMALL, textRenderer);
    }

    DrawNavigationHint("[W/S] Navigate  [ENTER] Select  [ESC] Back", layout, textRenderer);
}

void NPCDialogUI::DrawMessageUI(TextRenderer* textRenderer, RectRenderer* rectRenderer)
{
    if (!textRenderer || !rectRenderer) return;

    DialogBoxLayout layout = CalculateDialogBoxLayout();
    DrawDialogBoxBackground(layout, rectRenderer);

    // Draw message text with wrapping
    textRenderer->SetTextColor(0.08f, 0.11f, 0.11f);
    RenderWrappedText(mCurrentText, layout.textX, layout.textY, layout.maxTextWidth,
                     UIConstants::TEXT_SCALE_NORMAL,
                     UIConstants::LINE_SPACING, textRenderer);
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

